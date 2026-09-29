/**
 * @file stm32_template_port.h
 * @brief Narrow board/application port consumed by the template facade.
 *
 * A new board should implement only stm32_template_port_get().  Device modules
 * and business code do not need to know CubeMX handle names or RTOS globals.
 */

#ifndef STM32_TEMPLATE_PORT_H
#define STM32_TEMPLATE_PORT_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

typedef struct
{
    float channel1;
    float channel2;
    float total;
    uint8_t channel1_status;
    uint8_t channel2_status;
    uint16_t unit;
    uint8_t decimal;
} stm32_template_loadcell_sample_t;

typedef uint8_t (*stm32_template_loadcell_reader_t)(
    void *context,
    stm32_template_loadcell_sample_t *sample);

typedef struct
{
    I2C_HandleTypeDef *i2c1;
    UART_HandleTypeDef *uart1;
    UART_HandleTypeDef *uart3;
    UART_HandleTypeDef *console_uart;
    stm32_template_loadcell_reader_t read_loadcell;
    void *loadcell_context;
} stm32_template_port_t;

/** Return a persistent port descriptor, or NULL when no board port exists. */
const stm32_template_port_t *stm32_template_port_get(void);

#endif /* STM32_TEMPLATE_PORT_H */

