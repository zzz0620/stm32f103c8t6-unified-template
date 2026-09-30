#ifndef LOADCELL_H
#define LOADCELL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#include <stdint.h>

typedef enum
{
    LOADCELL_OK = 0,
    LOADCELL_ERR_PARAM = -1,
    LOADCELL_ERR_UART = -2,
    LOADCELL_ERR_TIMEOUT = -3,
    LOADCELL_ERR_RESP = -4,
    LOADCELL_ERR_CRC = -5
} loadcell_status_t;

typedef struct
{
    int32_t raw;
    float value;
    uint8_t decimal;
    uint16_t unit;
    uint16_t low_word;
    uint16_t high_word;
} loadcell_value_t;

void loadcell_init(UART_HandleTypeDef *huart);
void loadcell_set_slave_addr(uint8_t addr);
uint8_t loadcell_get_slave_addr(void);

loadcell_status_t loadcell_read_raw_reg32(uint16_t start_reg, int32_t *out);
loadcell_status_t loadcell_read_reg16(uint16_t reg, uint16_t *out);
loadcell_status_t loadcell_write_reg16(uint16_t reg, uint16_t value);
loadcell_status_t loadcell_write_reg32(uint16_t reg, uint32_t value);

loadcell_status_t loadcell_send_cmd(uint16_t cmd);
loadcell_status_t loadcell_set_unit(uint16_t unit);
loadcell_status_t loadcell_set_decimal(uint16_t decimal);

loadcell_status_t loadcell_read_net(loadcell_value_t *out);
loadcell_status_t loadcell_read_gross(loadcell_value_t *out);
loadcell_status_t loadcell_read_peak(loadcell_value_t *out);
loadcell_status_t loadcell_read_channel1(loadcell_value_t *out);
loadcell_status_t loadcell_read_channel2(loadcell_value_t *out);

void loadcell_rs485_set_tx(uint8_t enable);

#ifdef __cplusplus
}
#endif /* LOADCELL_H */

#endif
