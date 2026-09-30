/**
 * @file stm32_template_port.c
 * @brief CubeMX/bare-metal adapter for the included dart load-cell board.
 */

#include "stm32_template_port.h"

#include "i2c.h"
#include "usart.h"
#include "stm32_template_config.h"

#if TEMPLATE_USE_LOADCELL
#include "loadcell_service.h"
#endif

static void board_init_loadcell(void *context)
{
#if TEMPLATE_USE_LOADCELL
    (void)context;
    loadcell_service_init(&huart1, TEMPLATE_LOADCELL_SLAVE_ADDRESS);
#else
    (void)context;
#endif
}
static uint8_t board_read_loadcell(
    void *context,
    stm32_template_loadcell_sample_t *sample)
{
#if TEMPLATE_USE_LOADCELL
    loadcell_service_sample_t value;

    (void)context;
    if (sample == NULL)
    {
        return 0U;
    }

    if (loadcell_service_read(&value) == 0U)
    {
        sample->channel1_status = (int8_t)value.channel1_status;
        sample->channel2_status = (int8_t)value.channel2_status;
        return 0U;
    }

    sample->channel1 = value.channel1.value;
    sample->channel2 = value.channel2.value;
    sample->total = value.total;
    sample->channel1_status = (int8_t)value.channel1_status;
    sample->channel2_status = (int8_t)value.channel2_status;

    if (value.channel1_status == LOADCELL_OK)
    {
        sample->unit = value.channel1.unit;
        sample->decimal = value.channel1.decimal;
    }
    else if (value.channel2_status == LOADCELL_OK)
    {
        sample->unit = value.channel2.unit;
        sample->decimal = value.channel2.decimal;
    }

    return 1U;
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
    port.init_loadcell = board_init_loadcell;
    port.read_loadcell = board_read_loadcell;
    port.loadcell_context = NULL;
    return &port;
}

void loadcell_rs485_set_tx(uint8_t enable)
{
#if TEMPLATE_LOADCELL_RS485_DE_ENABLED
    GPIO_PinState state;

    if (TEMPLATE_LOADCELL_RS485_DE_ACTIVE_HIGH)
    {
        state = (enable != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    }
    else
    {
        state = (enable != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    }
    HAL_GPIO_WritePin(
        TEMPLATE_LOADCELL_RS485_DE_GPIO_PORT,
        TEMPLATE_LOADCELL_RS485_DE_GPIO_PIN,
        state);
#else
    (void)enable;
#endif
}
