#include "usart_tsak.h"

#include "cmsis_os.h"
#include "usart.h"

#define LOADCELL_SLAVE_ADDR             1u
#define LOADCELL_TASK_START_DELAY_MS    50u
#define LOADCELL_TASK_PERIOD_MS         20u
#define LOADCELL_INTER_CHANNEL_DELAY_MS 5u
#define LOADCELL_RETRY_DELAY_MS         1u
#define LOADCELL_READ_RETRY_COUNT       1u

loadcell_value_t g_loadcell_addr1_value = {0};
loadcell_value_t g_loadcell_addr2_value = {0};
float total_loadcell_value = 0.0f;
uint16_t g_loadcell_unit = 0u;
uint8_t g_loadcell_decimal = 0u;
loadcell_status_t g_loadcell_addr1_status = LOADCELL_ERR_PARAM;
loadcell_status_t g_loadcell_addr2_status = LOADCELL_ERR_PARAM;
uint8_t g_loadcell_test_addr = LOADCELL_SLAVE_ADDR;
loadcell_status_t g_loadcell_test_status = LOADCELL_ERR_PARAM;
loadcell_value_t g_loadcell_test_value = {0};

static void usart_task_update_meta_from_value(const loadcell_value_t *value)
{
    if (value == NULL)
    {
        return;
    }

    g_loadcell_decimal = value->decimal;
    g_loadcell_unit = value->unit;
}


typedef loadcell_status_t (*loadcell_read_fn_t)(loadcell_value_t *value);

static loadcell_status_t usart_task_read_one(loadcell_read_fn_t reader, loadcell_value_t *value)
{
    loadcell_status_t st = LOADCELL_ERR_PARAM;
    uint8_t attempt = 0u;

    if ((reader == NULL) || (value == NULL))
    {
        return LOADCELL_ERR_PARAM;
    }

    for (attempt = 0u; attempt < LOADCELL_READ_RETRY_COUNT; attempt++)
    {
        st = reader(value);
        if (st == LOADCELL_OK)
        {
            usart_task_update_meta_from_value(value);
            return st;
        }

        if ((attempt + 1u) < LOADCELL_READ_RETRY_COUNT)
        {
            osDelay(LOADCELL_RETRY_DELAY_MS);
        }
    }

    value->raw = 0;
    value->value = 0.0f;
    value->decimal = 0u;
    value->unit = 0u;
    value->low_word = 0u;
    value->high_word = 0u;

    return st;
}

static void usart_task_update_test_result(void)
{
    loadcell_set_slave_addr(g_loadcell_test_addr);

    g_loadcell_addr1_status = usart_task_read_one(loadcell_read_channel1, &g_loadcell_addr1_value);
    osDelay(LOADCELL_INTER_CHANNEL_DELAY_MS);
    g_loadcell_addr2_status = usart_task_read_one(loadcell_read_channel2, &g_loadcell_addr2_value);

    if (g_loadcell_addr1_status == LOADCELL_OK)
    {
        g_loadcell_test_status = g_loadcell_addr1_status;
        g_loadcell_test_value = g_loadcell_addr1_value;
    }
    else
    {
        g_loadcell_test_status = g_loadcell_addr2_status;
        g_loadcell_test_value = g_loadcell_addr2_value;
    }

    total_loadcell_value = g_loadcell_addr1_value.value + g_loadcell_addr2_value.value;
}

void usart_tsak(void const *argument)
{
    (void)argument;

    osDelay(LOADCELL_TASK_START_DELAY_MS);
    loadcell_init(&huart1);

    for (;;)
    {
        usart_task_update_test_result();

        vTaskDelay(LOADCELL_TASK_PERIOD_MS);
    }
}
