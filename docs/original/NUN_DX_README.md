# STM32F1c8t6_Open-source library

#### 介绍
这是北方民族大学 STM32 培训库,基于 STM32F103C8T6 微控制器(Cortex-M3 内核,主频 72MHz),采用 HAL 库 + CMSIS 架构,提供了一套分层清晰、易于移植和复用的外设驱动与设备抽象层,适用于教学、竞赛以及快速原型开发。

---

## 目录结构

```
STM32F1/
└── NUN_DX_STM32/                 # 主工程目录
    ├── Core/                     # STM32 HAL 库核心代码
    │   ├── Inc/                  # HAL 库头文件
    │   └── Src/                  # HAL 库源文件及主程序(main.c)
    ├── DX_common/                # 通用模块层
    │   ├── DX_common_headfile.h  # 统一头文件(汇总所有模块包含)
    │   ├── DX_common_typedef.h   # 公共类型定义
    │   ├── DX_common_fifo.*      # 环形 FIFO 实现
    │   ├── DX_common_font.*      # 字库(用于 OLED 显示)
    │   └── DX_common_interrupt.* # 中断回调统一管理
    ├── DX_debug/                 # 调试模块(printf 重定向、DMA 串口打印)
    ├── DX_device/                # 设备级驱动层(基于外设驱动封装具体传感器/模块)
    │   ├── DX_device_0.9_i2c_oled.*  # 0.9 寸 OLED 显示屏(I2C)
    │   ├── DX_device_aht10.*         # AHT10 温湿度传感器
    │   ├── DX_device_as5600.*        # AS5600 磁编码器(角度测量)
    │   ├── DX_device_bh1750.*        # BH1750 光照强度传感器
    │   ├── DX_device_i2c_mpu6050.*   # MPU6050 六轴姿态传感器
    │   ├── DX_device_key.*           # 按键驱动(支持消抖)
    │   ├── DX_device_servo.*         # 舵机驱动(PWM 控制)
    │   └── DX_device_tb6612.*        # TB6612 双 H 桥电机驱动
    ├── DX_driver/                # 底层外设驱动层(对 HAL 的二次封装)
    │   ├── DX_driver_adc.*       # ADC 采样
    │   ├── DX_driver_delay.*     # 延时(us/ms)
    │   ├── DX_driver_dma.*       # DMA 数据传输
    │   ├── DX_driver_exti.*      # 外部中断
    │   ├── DX_driver_gpio.*      # GPIO 操作
    │   ├── DX_driver_i2c.*       # 软/硬件 I2C
    │   ├── DX_driver_pwm.*       # PWM 输出
    │   ├── DX_driver_spi.*       # SPI 通信
    │   ├── DX_driver_timer.*     # 定时器
    │   ├── DX_driver_timer_trigger.*  # 定时器触发回调
    │   └── DX_driver_uart.*      # 串口通信
    ├── Drivers/                  # ST 官方 CMSIS 库
    │   └── CMSIS/                # 含 DSP 库、Cortex-M3 内核支持
    ├── .vscode/                  # VSCode 工程配置
    └── .mxproject                # STM32CubeMX 工程文件
```

## 软件架构

项目采用 **三层架构**,自下而上依次为:

1. **HAL/CMSIS 层**(`Core/`、`Drivers/`):ST 官方硬件抽象层,提供寄存器级封装。
2. **驱动层**(`DX_driver/`):对 HAL 进行二次封装,提供更简洁、易用的 API,屏蔽底层细节。
3. **设备层**(`DX_device/`):基于驱动层封装具体外设模块(传感器、显示屏、电机驱动等),面向应用直接调用。

调用关系:`应用层 → DX_device → DX_driver → HAL → 寄存器`

## 硬件环境

- **主控芯片**:STM32F103C8T6(ARM Cortex-M3,72MHz,64KB Flash,20KB SRAM)
- **时钟配置**:HSE 外部晶振 + PLL 9 倍频 = 72MHz;APB1 = 36MHz,APB2 = 72MHz
- **调试接口**:SWD

## 开发环境

- **STM32CubeMX**:用于初始化代码生成与外设配置
- **VSCode**:编辑器(已配置 `c_cpp_properties.json`,使用 windows-gcc-x64 IntelliSense)
- **ARM GCC 工具链**:交叉编译
- **烧录工具**:ST-Link / J-Link(配合 OpenOCD 或 STM32CubeProgrammer)

## 快速上手

1. **克隆仓库**

   ```bash
   git clone <仓库地址>
   ```

2. **打开工程**

   使用 VSCode 打开 `NUN_DX_STM32` 目录,IntelliSense 配置将自动加载。

3. **修改/编写代码**

   - 用户代码主要写在 `Core/Src/main.c` 中
   - 通过包含 `DX_common_headfile.h` 即可使用全部驱动与设备 API:

     ```c
     #include "DX_common_headfile.h"

     int main(void)
     {
         HAL_Init();
         SystemClock_Config();

         debug_init();               /* 调试串口初始化           */
         uart_dma_tx_init(UART_1);   /* UART1 配置 DMA 发送      */
         printf("System ready\r\n");
         debug_dma_str("Hello\r\n"); /* DMA 方式发送字符串        */

         aht10_test();               /* 调用设备层示例            */

         while (1)
         {
             /* 用户主循环逻辑 */
         }
     }
     ```

4. **编译与烧录**

   使用 ARM GCC 工具链编译,通过 ST-Link 烧录。具体命令视本地环境而定。

## 已支持设备列表

| 模块 | 通信方式 | 说明 |
| --- | --- | --- |
| 0.9 寸 OLED | I2C | 单色显示屏,支持字库显示 |
| AHT10 | I2C | 高精度温湿度传感器 |
| AS5600 | I2C | 12 位磁编码器,用于角度/位置反馈 |
| BH1750 | I2C | 数字光照强度传感器 |
| MPU6050 | I2C | 六轴加速度+陀螺仪 |
| 按键 | GPIO | 支持软件消抖 |
| 舵机 | PWM | 标准舵机角度控制 |
| TB6612 | PWM + GPIO | 双 H 桥直流电机驱动 |

## 已支持外设驱动

ADC、Delay(μs/ms)、DMA、EXTI、GPIO、I2C、PWM、SPI、Timer、UART(含 DMA 收发)。

## 目录约定

- 所有自研模块统一以 `DX_` 前缀命名
- `DX_common_headfile.h` 为统一头文件,业务代码仅需包含此文件
- 代码注释使用中文,符合项目代码规范

## 许可证

详见 [LICENSE](./LICENSE) 文件。

## 作者

YCZ
