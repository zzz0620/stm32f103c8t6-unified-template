#include "iic_task.h"

#include "cmsis_os.h"
#include "oled_ui.h"

#define IIC_TASK_OLED_INIT_DELAY_MS    200U
#define IIC_TASK_OLED_REFRESH_MS       1U

void i2c_task(void const *argument)
{
    (void)argument;

    osDelay(IIC_TASK_OLED_INIT_DELAY_MS);
    oled_ui_init();

    for (;;)
    {
        oled_ui_task();
        vTaskDelay(IIC_TASK_OLED_REFRESH_MS);
    }
}
