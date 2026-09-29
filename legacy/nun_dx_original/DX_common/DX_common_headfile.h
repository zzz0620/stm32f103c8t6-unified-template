#ifndef __DX_COMMON_HEADFILE_H
#define __DX_COMMON_HEADFILE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

#include "DX_common_typedef.h"

#include "DX_driver_gpio.h"
#include "DX_driver_delay.h"
#include "DX_driver_uart.h"
#include "DX_driver_adc.h"
#include "DX_driver_i2c.h"
#include "DX_driver_spi.h"
#include "DX_driver_timer.h"
#include "DX_driver_timer_trigger.h"
#include "DX_driver_timer_encoder.h"
#include "DX_driver_pwm.h"
#include "DX_driver_exti.h"
#include "DX_driver_dma.h"
#if DX_USE_MPU6050
#include "DX_device_i2c_mpu6050.h"
#endif /* DX_USE_MPU6050 */
#if DX_USE_OLED
#include "DX_device_0.9_i2c_oled.h"
#endif /* DX_USE_OLED */
#if DX_USE_TB6612
#include "DX_device_tb6612.h"
#endif /* DX_USE_TB6612 */
#if DX_USE_SERVO
#include "DX_device_servo.h"
#endif /* DX_USE_SERVO */
#if DX_USE_KEY
#include "DX_device_key.h"
#endif /* DX_USE_KEY */
#if DX_USE_AS5600
#include "DX_device_as5600.h"
#endif /* DX_USE_AS5600 */
#if DX_USE_AHT10
#include "DX_device_aht10.h"
#endif /* DX_USE_AHT10 */
#if DX_USE_AT24C64
#include "DX_device_at24c64.h"
#endif /* DX_USE_AT24C64 */
#if DX_USE_BH1750
#include "DX_device_bh1750.h"
#endif /* DX_USE_BH1750 */
#if DX_USE_ST7789
#include "DX_device_st7789.h"
#endif /* DX_USE_ST7789 */
#if DX_USE_ZDT_EMM_V5
#include "DX_device_zdt_emm_v5.h"
#endif /* DX_USE_ZDT_EMM_V5 */

#include "DX_control_pid.h"

#include "DX_common_interrupt.h"
#include "DX_debug.h"
#include "DX_common_fifo.h"

#ifdef __cplusplus
}
#endif

#endif /* __DX_COMMON_HEADFILE_H */
