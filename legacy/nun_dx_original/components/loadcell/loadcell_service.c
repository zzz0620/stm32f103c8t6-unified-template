/**
 * @file loadcell_service.c
 * @brief RTOS-independent sampling, retry and latest-value storage.
 */

#include "loadcell_service.h"

#include <string.h>

#include "stm32_template_config.h"

static uint8_t g_slave_address = TEMPLATE_LOADCELL_SLAVE_ADDRESS;
static loadcell_service_sample_t g_latest;

typedef loadcell_status_t (*loadcell_reader_t)(loadcell_value_t *value);

static loadcell_status_t loadcell_service_read_one(
    loadcell_reader_t reader,
    loadcell_value_t *value)
{
    loadcell_status_t status = LOADCELL_ERR_PARAM;
    uint8_t attempt;

    if ((reader == NULL) || (value == NULL))
    {
        return LOADCELL_ERR_PARAM;
    }

    for (attempt = 0u; attempt < TEMPLATE_LOADCELL_RETRY_COUNT; ++attempt)
    {
        status = reader(value);
        if (status == LOADCELL_OK)
        {
            return status;
        }

        if ((attempt + 1u) < TEMPLATE_LOADCELL_RETRY_COUNT)
        {
            HAL_Delay(TEMPLATE_LOADCELL_RETRY_DELAY_MS);
        }
    }

    memset(value, 0, sizeof(*value));
    return status;
}

void loadcell_service_init(UART_HandleTypeDef *uart, uint8_t slave_address)
{
    memset(&g_latest, 0, sizeof(g_latest));
    g_latest.channel1_status = LOADCELL_ERR_PARAM;
    g_latest.channel2_status = LOADCELL_ERR_PARAM;
    g_slave_address = slave_address;
    loadcell_init(uart);
    loadcell_set_slave_addr(g_slave_address);
}

uint8_t loadcell_service_read(loadcell_service_sample_t *sample)
{
    loadcell_service_sample_t next;

    if (sample == NULL)
    {
        return 0u;
    }

    memset(&next, 0, sizeof(next));
    loadcell_set_slave_addr(g_slave_address);
    next.channel1_status = loadcell_service_read_one(
        loadcell_read_channel1,
        &next.channel1);

    HAL_Delay(TEMPLATE_LOADCELL_INTER_CHANNEL_DELAY_MS);

    next.channel2_status = loadcell_service_read_one(
        loadcell_read_channel2,
        &next.channel2);

    if (next.channel1_status == LOADCELL_OK)
    {
        next.total += next.channel1.value;
    }
    if (next.channel2_status == LOADCELL_OK)
    {
        next.total += next.channel2.value;
    }

    g_latest = next;
    *sample = next;

    return ((next.channel1_status == LOADCELL_OK) ||
            (next.channel2_status == LOADCELL_OK)) ? 1u : 0u;
}

const loadcell_service_sample_t *loadcell_service_latest(void)
{
    return &g_latest;
}
