/**
 * @file stm32_template.h
 * @brief Stable, application-facing API for optional template modules.
 */

#ifndef STM32_TEMPLATE_H
#define STM32_TEMPLATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    STM32_TEMPLATE_DATA_LOADCELL = (1UL << 0),
    STM32_TEMPLATE_DATA_AHT10    = (1UL << 1),
    STM32_TEMPLATE_DATA_AS5600   = (1UL << 2),
    STM32_TEMPLATE_DATA_BH1750   = (1UL << 3),
    STM32_TEMPLATE_DATA_MPU6050  = (1UL << 4)
};

typedef struct
{
    uint32_t timestamp_ms;
    uint32_t valid_mask;

    float loadcell_channel1;
    float loadcell_channel2;
    float loadcell_total;
    uint8_t loadcell_channel1_status;
    uint8_t loadcell_channel2_status;
    uint16_t loadcell_unit;
    uint8_t loadcell_decimal;

    float temperature_c;
    float humidity_percent;
    float light_lux;
    float angle_deg;

    int16_t accel_raw[3];
    int16_t gyro_raw[3];
    int16_t mpu_temperature_raw;
} stm32_template_data_t;

/** Bind the NUN_DX adapter to CubeMX handles and initialize enabled sensors. */
void stm32_template_init(void);

/**
 * Copy current load-cell results and synchronously read enabled sensors.
 * Returns the same bit mask written to data->valid_mask.
 */
uint32_t stm32_template_read(stm32_template_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* STM32_TEMPLATE_H */

