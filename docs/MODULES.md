# 模块与 API 手册

## 统一入口

头文件：`legacy/nun_dx_original/components/template/stm32_template.h`

| API | 用途 | 约束 |
| --- | --- | --- |
| `stm32_template_init()` | 绑定 CubeMX 外设句柄并初始化已启用的无参数传感器 | HAL 外设初始化后调用一次 |
| `stm32_template_read(&data)` | 主动轮询测力模块并读取启用的传感器 | 返回 `valid_mask`；同步 UART/I2C，不在 ISR 调用 |
| `stm32_template_port_get()` | 提供板级外设和业务数据适配 | 新板只实现这个窄接口 |

统一结构 `stm32_template_data_t` 有固定 ABI。未启用或读取失败的字段保持 0，并通过 `valid_mask` 判断，不能仅靠数值判断有效性。

## 功能开关和数据返回

| 开关 | 模块 | 总线/资源 | 统一返回 |
| --- | --- | --- | --- |
| `TEMPLATE_USE_LOADCELL` | 双路称重 | USART1，裸机同步 Modbus | 通道值、合计、状态、单位、小数位 |
| `DX_USE_AHT10` | 温湿度 | I2C，0x38 | ℃、%RH |
| `DX_USE_AS5600` | 磁编码器 | I2C，0x36 | 角度 °；无磁铁不置有效位 |
| `DX_USE_BH1750` | 光照 | I2C，0x23 | lux；通信失败不置有效位 |
| `DX_USE_MPU6050` | 六轴 IMU | I2C，0x68 | 加速度/陀螺/温度原始值 |
| `DX_USE_OLED` | NUN_DX 0.9 寸 OLED | I2C，0x3C | 绘图 API，无统一读数 |
| `DX_USE_AT24C64` | 8 KB EEPROM | I2C，0x50 | 读写 API |
| `DX_USE_KEY` | 多按键 | 任意 GPIO | 按下/释放/长按事件 |
| `DX_USE_SERVO` | 舵机 | 定时器 PWM | 0–180° 控制 |
| `DX_USE_ST7789` | 240×240 LCD | SPI + 4 GPIO | RGB565 绘图 API |
| `DX_USE_TB6612` | 直流电机 | PWM + GPIO | 方向/制动/速度控制 |
| `DX_USE_ZDT_EMM_V5` | 闭环步进驱动 | UART，默认 USART3 | 命令发送 API |

此外，PID、FIFO、状态机、ADC/GPIO/I2C/SPI/UART/PWM/定时器等基础模块始终保留在源码中，由链接器移除未引用代码。

## 直接 API 示例

### AT24C64

```c
uint8_t tx[4] = {1, 2, 3, 4};
uint8_t rx[4];

if (at24c64_write(0x0100, tx, sizeof(tx)) == 0U)
{
    (void)at24c64_read(0x0100, rx, sizeof(rx));
}
```

### 按键

```c
key_bind(KEY_1, GPIOB, GPIO_PIN_0);

/* 每 10 ms 调用 */
key_scan();
if (key_get(KEY_1) == KEY_PRESSED)
{
    /* 处理一次短按事件 */
}
```

### 舵机

```c
servo_init(SERVO_TIM2_CH4_PA3);
servo_set_angle(SERVO_TIM2_CH4_PA3, 90U);
```

当前板 PA3 已由称重工程配置为 TIM2_CH4，但若现有任务正在使用同一 PWM，应用必须统一资源所有权，不能重复初始化。

### TB6612

```c
tb6612_init(TB6612_M3_TIM2_CH1, GPIOA, GPIO_PIN_4, GPIOA, GPIO_PIN_5);
tb6612_set_stby(GPIOB, GPIO_PIN_0);
tb6612_enable();
tb6612_forward(TB6612_M3_TIM2_CH1);
tb6612_set_speed(TB6612_M3_TIM2_CH1, 5000U); /* 50% */
```

### ST7789

```c
st7789_cfg_t lcd = {
    .spi = SPI_1,
    .mode = SPI_MODE_3,
    .prescaler = SPI_BAUDRATEPRESCALER_8,
    .pin = SPI1_SCK_PA5_MOSI_PA7,
    .cs_port = GPIOB, .cs_pin = GPIO_PIN_0,
    .dc_port = GPIOB, .dc_pin = GPIO_PIN_1,
    .rst_port = GPIOB, .rst_pin = GPIO_PIN_12,
    .bl_port = GPIOB, .bl_pin = GPIO_PIN_13,
};
st7789_init(&lcd);
st7789_clear(0x0000U);
```

具体 SPI 枚举以 `DX_driver_spi.h` 为准。启用前必须在 CubeMX 或 NUN_DX 层完成一致的时钟和引脚规划。

### ZDT Emm_V5

```c
Emm_V5_En_Control(1U, true, false);
Emm_V5_Pos_Control(1U, 0U, 500U, 20U, 3200U, 0U, false);
```

发送通道由 `TEMPLATE_DX_UART_BUS` 指定，默认复用 USART3。现有封装侧重命令发送；若项目需要解析驱动器回包，应在板级 UART 接收回调中增加独立协议解析器，避免与 `printf` 并发写串口。

## 移植到另一块 F103 板

1. 在 `.ioc` 中配置时钟和实际外设。
2. 在 `stm32_template_port.c` 中替换句柄映射；业务模块不感知 `hi2cX/huartX` 名称。
3. 修改 `TEMPLATE_DX_I2C_BUS` / `TEMPLATE_DX_UART_BUS`。
4. 若 CubeMX 管理中断，保持 `DX_STANDALONE_IRQ_HANDLERS=0`。
5. 重新生成 Keil 工程并进行全量 Rebuild。
6. 逐个模块启用、上板验证后再组合，持续检查 Flash/RAM 和引脚冲突。

F103C8 资源有限，库中“全部模块可用”表示模块均被迁移并可按需选用，不表示全部模块应在一个 C8 固件中同时启用。

