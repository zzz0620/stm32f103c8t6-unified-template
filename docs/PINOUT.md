# 默认引脚与资源冲突

以下来自 `legacy/nun_dx_original/stm32f103_template.ioc`，是默认称重板基线。实际 PCB 若不同，应先修改 `.ioc`，再调整板级 port。

| 引脚 | 默认功能 | 说明 |
| --- | --- | --- |
| PD0 / PD1 | HSE IN / OUT | 外部晶振，PLL ×9 得到 72 MHz |
| PA13 / PA14 | SWDIO / SWCLK | 调试下载，建议始终保留 |
| PB6 / PB7 | USART1 TX / RX（重映射） | 230400 8N1，称重通信 |
| PB10 / PB11 | USART3 TX / RX | 115200 8N1，日志；也可给 ZDT 使用 |
| PB8 / PB9 | I2C1 SCL / SDA（重映射） | 400 kHz，NUN_DX I2C 设备共享 |
| PA11 / PA12 | CAN RX / TX | 需要外部 CAN 收发器 |
| PA3 | TIM2_CH4 | 原工程 PWM |
| PB4 | TIM3_CH1 | 原工程 PWM；占用 JTAG 复用资源，保留 SWD |
| PA8 / PA9 / PA10 | LED_3 / LED_4 / LED_1 | 推挽输出 |
| PA2 | EXTI2 | 外部输入 |
| PC13 | EXTI13 | 外部输入/按键 |

## 常见冲突

- USART3 当前同时是 `printf` 日志端口。若启用 ZDT，建议把日志改到另一串口，或为发送建立互斥和帧队列。
- PB6/PB7 已用于 USART1，不能同时选择 TIM4_CH1/CH2 舵机或 TB6612 PWM。
- PB8/PB9 已用于 I2C1，不能同时选择 TIM4_CH3/CH4 PWM。
- PA8/PA9/PA10 是 LED，不能直接同时选择 TIM1_CH1/CH2/CH3 PWM，除非修改板级配置。
- PA11/PA12 是 CAN，不应再分配给 TIM1_CH4 或其他功能。
- PA2 是 EXTI，PA3 是 TIM2_CH4；选择这些引脚的 UART2/舵机功能前必须重配。
- PB4 是 TIM3_CH1；调试必须使用 SWD，不能恢复完整 JTAG 后仍占用 PB4。
- ST7789 没有默认布线；SPI 与 CS/DC/RST/BL 必须由使用者根据 PCB 明确配置。

## 电气注意事项

- STM32F103 工作在 3.3 V 逻辑电平；确认称重模块、OLED、传感器和电机驱动输入电平兼容。
- I2C 的 PB8/PB9 需要合适的上拉电阻；多个模块共享时确认地址不冲突和总线电容。
- CAN 必须外接收发器和终端电阻，MCU 引脚不能直接连接 CANH/CANL。
- 舵机、电机和背光不要由 MCU 引脚直接供电，功率地与逻辑地应正确共地并做好去耦。
- 烧录前先确认目标芯片 Flash/RAM 容量；部分标称 C8 的板卡可能容量不同，但工程按官方 64 KB/20 KB 链接。

