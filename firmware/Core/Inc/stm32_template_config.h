/**
 * @file stm32_template_config.h
 * @brief Central feature switches for the STM32F103C8 template.
 *
 * Keep this file as the only normal place where modules are enabled.  A value
 * of 1 includes the public API and implementation; 0 removes the device layer
 * from the image through preprocessing and linker garbage collection.
 */

#ifndef STM32_TEMPLATE_CONFIG_H
#define STM32_TEMPLATE_CONFIG_H

/* Integrated dart/load-cell application ----------------------------------- */
#ifndef TEMPLATE_USE_FREERTOS
#define TEMPLATE_USE_FREERTOS       1
#endif
#ifndef TEMPLATE_USE_LOADCELL
#define TEMPLATE_USE_LOADCELL       1
#endif
#ifndef TEMPLATE_USE_DART_OLED_UI
#define TEMPLATE_USE_DART_OLED_UI   1
#endif
#ifndef TEMPLATE_USE_DART_CAN_LINK
#define TEMPLATE_USE_DART_CAN_LINK  1
#endif

/* NUN_DX device layer ------------------------------------------------------ */
#ifndef DX_USE_OLED
#define DX_USE_OLED                 0
#endif
#ifndef DX_USE_AHT10
#define DX_USE_AHT10                0
#endif
#ifndef DX_USE_AT24C64
#define DX_USE_AT24C64              0
#endif
#ifndef DX_USE_AS5600
#define DX_USE_AS5600               0
#endif
#ifndef DX_USE_BH1750
#define DX_USE_BH1750               0
#endif
#ifndef DX_USE_MPU6050
#define DX_USE_MPU6050              0
#endif
#ifndef DX_USE_KEY
#define DX_USE_KEY                  0
#endif
#ifndef DX_USE_SERVO
#define DX_USE_SERVO                0
#endif
#ifndef DX_USE_ST7789
#define DX_USE_ST7789               0
#endif
#ifndef DX_USE_TB6612
#define DX_USE_TB6612               0
#endif
#ifndef DX_USE_ZDT_EMM_V5
#define DX_USE_ZDT_EMM_V5           0
#endif

/*
 * 0: CubeMX owns the interrupt vector and NUN_DX is used through attached HAL
 *    handles.  This is the integrated-template default.
 * 1: NUN_DX emits its original standalone DMA/EXTI/UART handlers.  Do not set
 *    this while CubeMX emits a handler for the same vector.
 */
#ifndef DX_STANDALONE_IRQ_HANDLERS
#define DX_STANDALONE_IRQ_HANDLERS   0
#endif

/* Existing CubeMX handles used by the adapter layer. */
#ifndef TEMPLATE_DX_I2C_BUS
#define TEMPLATE_DX_I2C_BUS          I2C_1
#endif
#ifndef TEMPLATE_DX_UART_BUS
#define TEMPLATE_DX_UART_BUS         UART_3
#endif

#define TEMPLATE_DX_I2C_ENABLED \
    (DX_USE_OLED || DX_USE_AHT10 || DX_USE_AT24C64 || DX_USE_AS5600 || \
     DX_USE_BH1750 || DX_USE_MPU6050)
#define TEMPLATE_DX_UART_ENABLED     (DX_USE_ZDT_EMM_V5)

#endif /* STM32_TEMPLATE_CONFIG_H */

