/**
 * @file stm32_template_port.c
 * @brief CubeMX/FreeRTOS adapter for the included dart load-cell board.
 */

#include "stm32_template_port.h"

#include "i2c.h"
#include "usart.h"
#include "stm32_template_config.h"

#if TEMPLATE_USE_LOADCELL
#include "usart_tsak.h"
#endif

static uint8_t board_read_loadcell(
    void *context,
    stm32_template_loadcell_sample_t *sample)
{
#if TEMPLATE_USE_LOADCELL
    uint32_t primask;

    (void)context;
    if (sample == NULL)
    {
        return 0U;
    }

    primask = __get_PRIMASK();
    __disable_irq();
    sample->channel1 = g_loadcell_addr1_value.value;
    sample->channel2 = g_loadcell_addr2_value.value;
    sample->total = total_loadcell_value;
    sample->channel1_status = (uint8_t)g_loadcell_addr1_status;
    sample->channel2_status = (uint8_t)g_loadcell_addr2_status;
    sample->unit = g_loadcell_unit;
    sample->decimal = g_loadcell_decimal;
    if (primask == 0U)
    {
        __enable_irq();
    }

    return ((g_loadcell_addr1_status == LOADCELL_OK) ||
            (g_loadcell_addr2_status == LOADCELL_OK)) ? 1U : 0U;
#else
    (void)context;
    (void)sample;
    return 0U;
#endif
}

const stm32_template_port_t *stm32_template_port_get(void)
{
    static stm32_template_port_t port;

    port.i2c1 = &hi2c1;
    port.uart1 = &huart1;
    port.uart3 = &huart3;
    port.console_uart = &huart3;
    port.read_loadcell = board_read_loadcell;
    port.loadcell_context = NULL;
    return &port;
}

