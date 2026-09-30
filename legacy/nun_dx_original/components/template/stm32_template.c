/**
 * @file stm32_template.c
 * @brief Integration adapter for CubeMX, the load-cell application and NUN_DX.
 */

#include "stm32_template.h"

#include <string.h>

#include "stm32_template_port.h"
#include "stm32_template_config.h"
#include "DX_common_headfile.h"

static const stm32_template_port_t *template_port;

__weak const stm32_template_port_t *stm32_template_port_get(void)
{
    return NULL;
}

void stm32_template_init(void)
{
    template_port = stm32_template_port_get();
    if (template_port == NULL)
    {
        return;
    }

    /* Reuse board-owned peripherals instead of initializing them twice. */
#if TEMPLATE_DX_I2C_ENABLED
    if (template_port->i2c1 != NULL)
    {
        i2c_attach(TEMPLATE_DX_I2C_BUS, template_port->i2c1);
    }
#endif
#if TEMPLATE_DX_UART_ENABLED
    if (template_port->uart3 != NULL)
    {
        uart_attach(TEMPLATE_DX_UART_BUS, template_port->uart3);
    }
#endif

#if TEMPLATE_USE_LOADCELL
    if (template_port->init_loadcell != NULL)
    {
        template_port->init_loadcell(template_port->loadcell_context);
    }
#endif

#if DX_USE_AHT10
    aht10_init();
#endif
#if DX_USE_AS5600
    as5600_init();
#endif
#if DX_USE_MPU6050
    mpu6050_init(TEMPLATE_DX_I2C_BUS);
#endif
#if DX_USE_OLED
    oled_init();
#endif
}

uint32_t stm32_template_read(stm32_template_data_t *data)
{
    if (data == NULL)
    {
        return 0U;
    }

    memset(data, 0, sizeof(*data));
    data->timestamp_ms = HAL_GetTick();

#if TEMPLATE_USE_LOADCELL
    if ((template_port != NULL) && (template_port->read_loadcell != NULL))
    {
        stm32_template_loadcell_sample_t sample;
        memset(&sample, 0, sizeof(sample));
        if (template_port->read_loadcell(template_port->loadcell_context, &sample) != 0U)
        {
            data->valid_mask |= STM32_TEMPLATE_DATA_LOADCELL;
        }
        data->loadcell_channel1 = sample.channel1;
        data->loadcell_channel2 = sample.channel2;
        data->loadcell_total = sample.total;
        data->loadcell_channel1_status = sample.channel1_status;
        data->loadcell_channel2_status = sample.channel2_status;
        data->loadcell_unit = sample.unit;
        data->loadcell_decimal = sample.decimal;
    }
#endif

#if DX_USE_AHT10
    {
        aht10_data_t value;
        if (aht10_read(&value) == 0U)
        {
            data->temperature_c = value.temp;
            data->humidity_percent = value.hum;
            data->valid_mask |= STM32_TEMPLATE_DATA_AHT10;
        }
    }
#endif

#if DX_USE_AS5600
    if (as5600_is_magnet() != 0U)
    {
        data->angle_deg = as5600_angle_deg();
        data->valid_mask |= STM32_TEMPLATE_DATA_AS5600;
    }
#endif

#if DX_USE_BH1750
    data->light_lux = bh1750_read_lux();
    if (data->light_lux >= 0.0f)
    {
        data->valid_mask |= STM32_TEMPLATE_DATA_BH1750;
    }
#endif

#if DX_USE_MPU6050
    {
        mpu6050_raw_data_t accel;
        mpu6050_raw_data_t gyro;
        mpu6050_read_accel(&accel);
        mpu6050_read_gyro(&gyro);
        data->accel_raw[0] = accel.x;
        data->accel_raw[1] = accel.y;
        data->accel_raw[2] = accel.z;
        data->gyro_raw[0] = gyro.x;
        data->gyro_raw[1] = gyro.y;
        data->gyro_raw[2] = gyro.z;
        data->mpu_temperature_raw = mpu6050_read_temp();
        data->valid_mask |= STM32_TEMPLATE_DATA_MPU6050;
    }
#endif

    return data->valid_mask;
}

