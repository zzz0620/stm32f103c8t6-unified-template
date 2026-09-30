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

/*
 * Module switches ----------------------------------------------------------
 * Change only this section for normal use.  The default image is deliberately
 * small and bare-metal: it reads the dual-channel load cell through USART1.
 */
#ifndef TEMPLATE_USE_LOADCELL
#define TEMPLATE_USE_LOADCELL       1
#endif
#ifndef TEMPLATE_USE_DART_OLED_UI
#define TEMPLATE_USE_DART_OLED_UI   0
#endif
#ifndef TEMPLATE_USE_DART_CAN_LINK
#define TEMPLATE_USE_DART_CAN_LINK  0
#endif
#ifndef TEMPLATE_USE_CONSOLE_UART
#define TEMPLATE_USE_CONSOLE_UART   0
#endif
#ifndef TEMPLATE_USE_BOARD_PWM
#define TEMPLATE_USE_BOARD_PWM      0
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

/* Load-cell board settings ------------------------------------------------- */
#ifndef TEMPLATE_LOADCELL_SLAVE_ADDRESS
#define TEMPLATE_LOADCELL_SLAVE_ADDRESS          1U
#endif
#ifndef TEMPLATE_LOADCELL_POLL_INTERVAL_MS
#define TEMPLATE_LOADCELL_POLL_INTERVAL_MS       20U
#endif
#ifndef TEMPLATE_LOADCELL_INTER_CHANNEL_DELAY_MS
#define TEMPLATE_LOADCELL_INTER_CHANNEL_DELAY_MS 5U
#endif
#ifndef TEMPLATE_LOADCELL_RETRY_COUNT
#define TEMPLATE_LOADCELL_RETRY_COUNT            2U
#endif
#ifndef TEMPLATE_LOADCELL_RETRY_DELAY_MS
#define TEMPLATE_LOADCELL_RETRY_DELAY_MS         1U
#endif
#ifndef TEMPLATE_LOADCELL_UART_TIMEOUT_MS
#define TEMPLATE_LOADCELL_UART_TIMEOUT_MS         8U
#endif

/*
 * 0: the RS485 module controls direction automatically (default board).
 * 1: provide TEMPLATE_LOADCELL_RS485_DE_GPIO_PORT and
 *    TEMPLATE_LOADCELL_RS485_DE_GPIO_PIN below for a manual DE/RE signal.
 */
#ifndef TEMPLATE_LOADCELL_RS485_DE_ENABLED
#define TEMPLATE_LOADCELL_RS485_DE_ENABLED       0
#endif
#ifndef TEMPLATE_LOADCELL_RS485_DE_ACTIVE_HIGH
#define TEMPLATE_LOADCELL_RS485_DE_ACTIVE_HIGH   1
#endif

#if (TEMPLATE_LOADCELL_RETRY_COUNT < 1U)
#error "TEMPLATE_LOADCELL_RETRY_COUNT must be at least 1"
#endif
#if TEMPLATE_LOADCELL_RS485_DE_ENABLED
#if !defined(TEMPLATE_LOADCELL_RS485_DE_GPIO_PORT) || \
    !defined(TEMPLATE_LOADCELL_RS485_DE_GPIO_PIN)
#error "Define the RS485 DE GPIO port and pin when manual direction is enabled"
#endif
#endif

#define TEMPLATE_DX_I2C_ENABLED \
    (DX_USE_OLED || DX_USE_AHT10 || DX_USE_AT24C64 || DX_USE_AS5600 || \
     DX_USE_BH1750 || DX_USE_MPU6050)
#define TEMPLATE_DX_UART_ENABLED     (DX_USE_ZDT_EMM_V5)

/* Derived board-peripheral ownership used by main.c. */
#define TEMPLATE_BOARD_I2C1_ENABLED  \
    (TEMPLATE_DX_I2C_ENABLED || TEMPLATE_USE_DART_OLED_UI)
#define TEMPLATE_BOARD_USART1_ENABLED (TEMPLATE_USE_LOADCELL)
#define TEMPLATE_BOARD_USART3_ENABLED \
    (TEMPLATE_DX_UART_ENABLED || TEMPLATE_USE_CONSOLE_UART)
#define TEMPLATE_BOARD_CAN_ENABLED   \
    (TEMPLATE_USE_DART_CAN_LINK || TEMPLATE_USE_DART_OLED_UI)
#define TEMPLATE_BOARD_DMA_ENABLED   \
    (TEMPLATE_BOARD_I2C1_ENABLED || TEMPLATE_BOARD_USART1_ENABLED || \
     TEMPLATE_BOARD_USART3_ENABLED)

#endif /* STM32_TEMPLATE_CONFIG_H */
