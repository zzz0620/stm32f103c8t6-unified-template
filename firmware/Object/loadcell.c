#include "loadcell.h"

#include <string.h>

/*
Modbus RTU registers:
0x0000: channel 1 realtime weight (32-bit signed, low reg first)
0x0002: channel 2 realtime weight (32-bit signed, low reg first)
0x0004: reserved/other value depending on model
0x0008: decimal point (uint16)
0x0009: unit (uint16)
0x000B: command (uint16)
*/

#define LOADCELL_FUNC_READ_HOLDING 0x03u
#define LOADCELL_FUNC_WRITE_SINGLE 0x06u
#define LOADCELL_FUNC_WRITE_MULTI 0x10u

#define LOADCELL_REG_CH1 0x0000u
#define LOADCELL_REG_CH2 0x0002u
#define LOADCELL_REG_PEAK 0x0004u
#define LOADCELL_REG_DECIMAL 0x0008u
#define LOADCELL_REG_UNIT 0x0009u
#define LOADCELL_REG_CMD 0x000Bu

#define LOADCELL_UART_TIMEOUT_MS 8u
#define LOADCELL_META_REFRESH_INTERVAL 32u

static UART_HandleTypeDef *g_loadcell_uart = NULL;
static uint8_t g_loadcell_addr = 0u;
static uint8_t g_loadcell_meta_valid[256];
static uint8_t g_loadcell_cached_decimal[256];
static uint16_t g_loadcell_cached_unit[256];
static uint16_t g_loadcell_meta_refresh_countdown[256];

__weak void loadcell_rs485_set_tx(uint8_t enable)
{
    (void)enable;
}

static uint16_t loadcell_crc16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFFu;
    uint16_t i = 0u;

    for (i = 0u; i < len; i++)
    {
        uint8_t j = 0u;

        crc ^= buf[i];
        for (j = 0u; j < 8u; j++)
        {
            if ((crc & 0x0001u) != 0u)
            {
                crc >>= 1;
                crc ^= 0xA001u;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

static loadcell_status_t loadcell_uart_txrx(const uint8_t *tx,
                                            uint16_t tx_len,
                                            uint8_t *rx,
                                            uint16_t rx_len)
{
    uint32_t flush_guard = 0u;

    if ((g_loadcell_uart == NULL) || (tx == NULL) || (rx == NULL))
    {
        return LOADCELL_ERR_PARAM;
    }

    HAL_UART_AbortReceive(g_loadcell_uart);
    __HAL_UART_CLEAR_OREFLAG(g_loadcell_uart);
    while ((__HAL_UART_GET_FLAG(g_loadcell_uart, UART_FLAG_RXNE) != RESET) && (flush_guard < 64u))
    {
        __HAL_UART_FLUSH_DRREGISTER(g_loadcell_uart);
        flush_guard++;
    }

    loadcell_rs485_set_tx(1u);
    if (HAL_UART_Transmit(g_loadcell_uart, (uint8_t *)tx, tx_len, LOADCELL_UART_TIMEOUT_MS) != HAL_OK)
    {
        loadcell_rs485_set_tx(0u);
        return LOADCELL_ERR_UART;
    }

    if (HAL_UART_GetState(g_loadcell_uart) != HAL_UART_STATE_READY)
    {
        loadcell_rs485_set_tx(0u);
        return LOADCELL_ERR_UART;
    }

    loadcell_rs485_set_tx(0u);

    if (HAL_UART_Receive(g_loadcell_uart, rx, rx_len, LOADCELL_UART_TIMEOUT_MS) != HAL_OK)
    {
        HAL_UART_AbortReceive(g_loadcell_uart);
        __HAL_UART_CLEAR_OREFLAG(g_loadcell_uart);
        return LOADCELL_ERR_TIMEOUT;
    }

    return LOADCELL_OK;
}

static loadcell_status_t loadcell_read_regs(uint16_t start_reg,
                                            uint16_t count,
                                            uint8_t *out,
                                            uint16_t out_len)
{
    uint8_t req[8];
    uint16_t crc = 0u;
    uint16_t expected_len = 0u;
    loadcell_status_t st;
    uint16_t resp_crc = 0u;
    uint16_t calc_crc = 0u;

    if ((count == 0u) || (out == NULL))
    {
        return LOADCELL_ERR_PARAM;
    }

    req[0] = g_loadcell_addr;
    req[1] = LOADCELL_FUNC_READ_HOLDING;
    req[2] = (uint8_t)(start_reg >> 8);
    req[3] = (uint8_t)(start_reg & 0xFFu);
    req[4] = (uint8_t)(count >> 8);
    req[5] = (uint8_t)(count & 0xFFu);
    crc = loadcell_crc16(req, 6u);
    req[6] = (uint8_t)(crc & 0xFFu);
    req[7] = (uint8_t)(crc >> 8);

    expected_len = (uint16_t)(5u + (count * 2u));
    if (out_len < expected_len)
    {
        return LOADCELL_ERR_PARAM;
    }

    st = loadcell_uart_txrx(req, (uint16_t)sizeof(req), out, expected_len);
    if (st != LOADCELL_OK)
    {
        return st;
    }

    if ((out[0] != g_loadcell_addr) || (out[1] != LOADCELL_FUNC_READ_HOLDING))
    {
        return LOADCELL_ERR_RESP;
    }

    if (out[2] != (uint8_t)(count * 2u))
    {
        return LOADCELL_ERR_RESP;
    }

    resp_crc = (uint16_t)out[expected_len - 2u] | ((uint16_t)out[expected_len - 1u] << 8);
    calc_crc = loadcell_crc16(out, (uint16_t)(expected_len - 2u));
    if (resp_crc != calc_crc)
    {
        return LOADCELL_ERR_CRC;
    }

    return LOADCELL_OK;
}

static int32_t loadcell_parse_low_high_words_int32(const uint8_t *data)
{
    uint16_t low_word = 0u;
    uint16_t high_word = 0u;

    if (data == NULL)
    {
        return 0;
    }

    low_word = (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
    high_word = (uint16_t)(((uint16_t)data[2] << 8) | data[3]);

    return (int32_t)((((uint32_t)high_word) << 16) | low_word);
}

static void loadcell_pack_cdab_uint32(uint32_t value, uint8_t *out)
{
    uint8_t a = (uint8_t)(value >> 24);
    uint8_t b = (uint8_t)(value >> 16);
    uint8_t c = (uint8_t)(value >> 8);
    uint8_t d = (uint8_t)value;

    out[0] = c;
    out[1] = d;
    out[2] = a;
    out[3] = b;
}

static float loadcell_apply_decimal(int32_t raw, uint8_t decimal)
{
    float value = (float)raw;

    switch (decimal)
    {
    case 1u:
        value /= 10.0f;
        break;
    case 2u:
        value /= 100.0f;
        break;
    case 3u:
        value /= 1000.0f;
        break;
    case 4u:
        value /= 10000.0f;
        break;
    default:
        break;
    }

    return value;
}

static loadcell_status_t loadcell_refresh_meta(uint8_t *decimal, uint16_t *unit)
{
    uint8_t resp[9];
    loadcell_status_t st;
    uint8_t addr = g_loadcell_addr;

    if ((decimal == NULL) || (unit == NULL))
    {
        return LOADCELL_ERR_PARAM;
    }

    st = loadcell_read_regs(LOADCELL_REG_DECIMAL, 2u, resp, (uint16_t)sizeof(resp));
    if (st != LOADCELL_OK)
    {
        return st;
    }

    *decimal = resp[4];
    *unit = (uint16_t)(((uint16_t)resp[5] << 8) | resp[6]);

    g_loadcell_cached_decimal[addr] = *decimal;
    g_loadcell_cached_unit[addr] = *unit;
    g_loadcell_meta_valid[addr] = 1u;
    g_loadcell_meta_refresh_countdown[addr] = LOADCELL_META_REFRESH_INTERVAL;

    return LOADCELL_OK;
}

static loadcell_status_t loadcell_read_raw_reg32_internal(uint16_t start_reg,
                                                          int32_t *out,
                                                          uint16_t *low_word,
                                                          uint16_t *high_word)
{
    uint8_t resp[9];
    loadcell_status_t st;

    if (out == NULL)
    {
        return LOADCELL_ERR_PARAM;
    }

    st = loadcell_read_regs(start_reg, 2u, resp, (uint16_t)sizeof(resp));
    if (st != LOADCELL_OK)
    {
        return st;
    }

    if (low_word != NULL)
    {
        *low_word = (uint16_t)(((uint16_t)resp[3] << 8) | resp[4]);
    }

    if (high_word != NULL)
    {
        *high_word = (uint16_t)(((uint16_t)resp[5] << 8) | resp[6]);
    }

    *out = loadcell_parse_low_high_words_int32(&resp[3]);
    return LOADCELL_OK;
}

static loadcell_status_t loadcell_read_value(uint16_t reg, loadcell_value_t *out)
{
    int32_t raw = 0;
    uint8_t decimal = 0u;
    uint16_t unit = 0u;
    loadcell_status_t st;
    uint8_t addr = g_loadcell_addr;

    if (out == NULL)
    {
        return LOADCELL_ERR_PARAM;
    }

    st = loadcell_read_raw_reg32_internal(reg, &raw, &out->low_word, &out->high_word);
    if (st != LOADCELL_OK)
    {
        out->low_word = 0u;
        out->high_word = 0u;
        return st;
    }

    if ((g_loadcell_meta_valid[addr] == 0u) || (g_loadcell_meta_refresh_countdown[addr] == 0u))
    {
        st = loadcell_refresh_meta(&decimal, &unit);
        if (st != LOADCELL_OK)
        {
            if (g_loadcell_meta_valid[addr] != 0u)
            {
                decimal = g_loadcell_cached_decimal[addr];
                unit = g_loadcell_cached_unit[addr];
            }
            else
            {
                decimal = 0u;
                unit = 0u;
            }
        }
    }
    else
    {
        decimal = g_loadcell_cached_decimal[addr];
        unit = g_loadcell_cached_unit[addr];
        g_loadcell_meta_refresh_countdown[addr]--;
    }

    out->raw = raw;
    out->decimal = decimal;
    out->unit = unit;
    out->value = loadcell_apply_decimal(raw, decimal);

    return LOADCELL_OK;
}

void loadcell_init(UART_HandleTypeDef *huart)
{
    g_loadcell_uart = huart;
    g_loadcell_addr = 0u;
    memset(g_loadcell_meta_valid, 0, sizeof(g_loadcell_meta_valid));
    memset(g_loadcell_cached_decimal, 0, sizeof(g_loadcell_cached_decimal));
    memset(g_loadcell_cached_unit, 0, sizeof(g_loadcell_cached_unit));
    memset(g_loadcell_meta_refresh_countdown, 0, sizeof(g_loadcell_meta_refresh_countdown));
}

void loadcell_set_slave_addr(uint8_t addr)
{
    g_loadcell_addr = addr;
}

uint8_t loadcell_get_slave_addr(void)
{
    return g_loadcell_addr;
}

loadcell_status_t loadcell_read_raw_reg32(uint16_t start_reg, int32_t *out)
{
    return loadcell_read_raw_reg32_internal(start_reg, out, NULL, NULL);
}

loadcell_status_t loadcell_read_reg16(uint16_t reg, uint16_t *out)
{
    uint8_t resp[7];
    loadcell_status_t st;

    if (out == NULL)
    {
        return LOADCELL_ERR_PARAM;
    }

    st = loadcell_read_regs(reg, 1u, resp, (uint16_t)sizeof(resp));
    if (st != LOADCELL_OK)
    {
        return st;
    }

    *out = (uint16_t)(((uint16_t)resp[3] << 8) | resp[4]);
    return LOADCELL_OK;
}

loadcell_status_t loadcell_write_reg16(uint16_t reg, uint16_t value)
{
    uint8_t req[8];
    uint8_t resp[8];
    uint16_t crc = 0u;
    uint16_t resp_crc = 0u;
    uint16_t calc_crc = 0u;
    loadcell_status_t st;

    req[0] = g_loadcell_addr;
    req[1] = LOADCELL_FUNC_WRITE_SINGLE;
    req[2] = (uint8_t)(reg >> 8);
    req[3] = (uint8_t)(reg & 0xFFu);
    req[4] = (uint8_t)(value >> 8);
    req[5] = (uint8_t)(value & 0xFFu);
    crc = loadcell_crc16(req, 6u);
    req[6] = (uint8_t)(crc & 0xFFu);
    req[7] = (uint8_t)(crc >> 8);

    st = loadcell_uart_txrx(req, (uint16_t)sizeof(req), resp, (uint16_t)sizeof(resp));
    if (st != LOADCELL_OK)
    {
        return st;
    }

    resp_crc = (uint16_t)resp[6] | ((uint16_t)resp[7] << 8);
    calc_crc = loadcell_crc16(resp, 6u);
    if (resp_crc != calc_crc)
    {
        return LOADCELL_ERR_CRC;
    }

    if (memcmp(req, resp, 6u) != 0)
    {
        return LOADCELL_ERR_RESP;
    }

    return LOADCELL_OK;
}

loadcell_status_t loadcell_write_reg32(uint16_t reg, uint32_t value)
{
    uint8_t req[13];
    uint8_t resp[8];
    uint16_t crc = 0u;
    uint16_t resp_crc = 0u;
    uint16_t calc_crc = 0u;
    loadcell_status_t st;

    req[0] = g_loadcell_addr;
    req[1] = LOADCELL_FUNC_WRITE_MULTI;
    req[2] = (uint8_t)(reg >> 8);
    req[3] = (uint8_t)(reg & 0xFFu);
    req[4] = 0x00u;
    req[5] = 0x02u;
    req[6] = 0x04u;
    loadcell_pack_cdab_uint32(value, &req[7]);
    crc = loadcell_crc16(req, 11u);
    req[11] = (uint8_t)(crc & 0xFFu);
    req[12] = (uint8_t)(crc >> 8);

    st = loadcell_uart_txrx(req, (uint16_t)sizeof(req), resp, (uint16_t)sizeof(resp));
    if (st != LOADCELL_OK)
    {
        return st;
    }

    resp_crc = (uint16_t)resp[6] | ((uint16_t)resp[7] << 8);
    calc_crc = loadcell_crc16(resp, 6u);
    if (resp_crc != calc_crc)
    {
        return LOADCELL_ERR_CRC;
    }

    if ((resp[0] != g_loadcell_addr) || (resp[1] != LOADCELL_FUNC_WRITE_MULTI))
    {
        return LOADCELL_ERR_RESP;
    }

    if ((resp[2] != req[2]) || (resp[3] != req[3]) || (resp[4] != req[4]) || (resp[5] != req[5]))
    {
        return LOADCELL_ERR_RESP;
    }

    return LOADCELL_OK;
}

loadcell_status_t loadcell_send_cmd(uint16_t cmd)
{
    return loadcell_write_reg16(LOADCELL_REG_CMD, cmd);
}

loadcell_status_t loadcell_set_unit(uint16_t unit)
{
    loadcell_status_t st = loadcell_write_reg16(LOADCELL_REG_UNIT, unit);

    if (st == LOADCELL_OK)
    {
        g_loadcell_meta_valid[g_loadcell_addr] = 0u;
        g_loadcell_meta_refresh_countdown[g_loadcell_addr] = 0u;
    }

    return st;
}

loadcell_status_t loadcell_set_decimal(uint16_t decimal)
{
    loadcell_status_t st = loadcell_write_reg16(LOADCELL_REG_DECIMAL, decimal);

    if (st == LOADCELL_OK)
    {
        g_loadcell_meta_valid[g_loadcell_addr] = 0u;
        g_loadcell_meta_refresh_countdown[g_loadcell_addr] = 0u;
    }

    return st;
}

loadcell_status_t loadcell_read_net(loadcell_value_t *out)
{
    return loadcell_read_value(LOADCELL_REG_CH2, out);
}

loadcell_status_t loadcell_read_gross(loadcell_value_t *out)
{
    return loadcell_read_value(LOADCELL_REG_CH1, out);
}

loadcell_status_t loadcell_read_peak(loadcell_value_t *out)
{
    return loadcell_read_value(LOADCELL_REG_PEAK, out);
}

loadcell_status_t loadcell_read_channel1(loadcell_value_t *out)
{
    return loadcell_read_value(LOADCELL_REG_CH1, out);
}

loadcell_status_t loadcell_read_channel2(loadcell_value_t *out)
{
    return loadcell_read_value(LOADCELL_REG_CH2, out);
}
