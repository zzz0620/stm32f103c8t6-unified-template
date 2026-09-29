cmake_minimum_required(VERSION 3.20)

project(stm32f103_template C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(FW ${CMAKE_CURRENT_SOURCE_DIR}/firmware)

file(GLOB NUN_COMMON_SOURCES CONFIGURE_DEPENDS
    ${FW}/Components/NUN_DX/common/*.c
    ${FW}/Components/NUN_DX/control/*.c
    ${FW}/Components/NUN_DX/debug/*.c
    ${FW}/Components/NUN_DX/device/*.c
    ${FW}/Components/NUN_DX/driver/*.c
    ${FW}/Components/NUN_DX/state_machine/*.c)

set(CORE_SOURCES
    ${FW}/Core/Src/main.c
    ${FW}/Core/Src/gpio.c
    ${FW}/Core/Src/dma.c
    ${FW}/Core/Src/i2c.c
    ${FW}/Core/Src/usart.c
    ${FW}/Core/Src/tim.c
    ${FW}/Core/Src/can.c
    ${FW}/Core/Src/freertos.c
    ${FW}/Core/Src/iic_task.c
    ${FW}/Core/Src/usart_tsak.c
    ${FW}/Core/Src/stm32_template_port.c
    ${FW}/Core/Src/syscalls.c
    ${FW}/Core/Src/stm32f1xx_it.c
    ${FW}/Core/Src/stm32f1xx_hal_msp.c
    ${FW}/Core/Src/stm32f1xx_hal_timebase_tim.c
    ${FW}/Core/Src/system_stm32f1xx.c)

set(APP_SOURCES
    ${FW}/Components/Template/stm32_template.c
    ${FW}/Object/loadcell.c
    ${FW}/Object/font.c
    ${FW}/Object/oled.c
    ${FW}/Object/oled_ui.c)

set(HAL_SOURCES
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc_ex.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_can.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c)

set(FREERTOS_SOURCES
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/list.c
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/queue.c
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/timers.c
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c)

add_executable(stm32f103_template
    ${CORE_SOURCES}
    ${APP_SOURCES}
    ${NUN_COMMON_SOURCES}
    ${HAL_SOURCES}
    ${FREERTOS_SOURCES}
    ${FW}/Startup/startup_stm32f103xb.s)

target_include_directories(stm32f103_template PRIVATE
    ${FW}/Core/Inc
    ${FW}/Object
    ${FW}/Components/Template
    ${FW}/Components/NUN_DX/common
    ${FW}/Components/NUN_DX/control
    ${FW}/Components/NUN_DX/debug
    ${FW}/Components/NUN_DX/device
    ${FW}/Components/NUN_DX/driver
    ${FW}/Components/NUN_DX/state_machine
    ${FW}/Components/NUN_DX/zdt
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${FW}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    ${FW}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${FW}/Drivers/CMSIS/Include
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
    ${FW}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3)

target_compile_definitions(stm32f103_template PRIVATE
    USE_HAL_DRIVER
    STM32F103xB)

target_compile_options(stm32f103_template PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -ffunction-sections
    -fdata-sections
    -fstack-usage
    -Wall
    -Wextra
    -Wshadow
    -Wundef
    -Wno-missing-braces
    -Wno-missing-field-initializers
    -Wno-unused-parameter
    $<$<CONFIG:Debug>:-Og;-g3>
    $<$<NOT:$<CONFIG:Debug>>:-Os>)

target_link_options(stm32f103_template PRIVATE
    -mcpu=cortex-m3
    -mthumb
    --specs=nano.specs
    --specs=nosys.specs
    -T${FW}/STM32F103C8TX_FLASH.ld
    -Wl,--gc-sections
    -Wl,--no-warn-rwx-segments
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/stm32f103_template.map
    -Wl,--print-memory-usage
    -Wl,-u,_printf_float)

target_link_libraries(stm32f103_template PRIVATE m c gcc)

add_custom_command(TARGET stm32f103_template POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:stm32f103_template>
            ${CMAKE_CURRENT_BINARY_DIR}/stm32f103_template.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:stm32f103_template>
            ${CMAKE_CURRENT_BINARY_DIR}/stm32f103_template.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:stm32f103_template>
    COMMENT "Generating HEX/BIN and printing image size")

