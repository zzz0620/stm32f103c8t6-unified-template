# STM32F103C8T6 Unified Template

[![MCU](https://img.shields.io/badge/MCU-STM32F103C8T6-03234B)](https://www.st.com/en/microcontrollers-microprocessors/stm32f103c8.html)
[![Toolchain](https://img.shields.io/badge/toolchain-Keil%20MDK%20%7C%20GNU%20Arm-blue)](#构建)
[![License](https://img.shields.io/badge/license-AGPL--3.0--only-orange)](LICENSE)

这是 NUN_DX STM32F103 模块库与 rm2026 双通道测力工程合并后的**单一可部署工程**。工程默认使用裸机轮询，不依赖 FreeRTOS；只开启测力模块，其余设备通过一个配置头文件按需启用。

- 仓库：<https://github.com/zzz0620/stm32f103c8t6-unified-template>
- 唯一工程目录：`legacy/nun_dx_original`
- CubeMX：`legacy/nun_dx_original/stm32f103_template.ioc`
- Keil：`legacy/nun_dx_original/MDK-ARM/stm32f103_template.uvprojx`
- 模块开关：`legacy/nun_dx_original/Core/Inc/stm32_template_config.h`

## 快速使用测力传感器

默认配置已经完成以下工作：

1. STM32F103C8T6 运行于 72 MHz。
2. USART1 重映射到 PB6/PB7，参数为 230400、8N1。
3. 从站地址默认为 1，轮询周期为 20 ms。
4. 依次读取 Modbus 保持寄存器 `0x0000`（通道 1）和 `0x0002`（通道 2），校验 CRC16，并根据 `0x0008/0x0009` 返回小数位和单位。
5. `main.c` 持续更新全局变量 `g_stm32_template_data`，便于调试器直接观察。

自动方向 RS485 模块接线：

| STM32F103 | RS485 模块 | 说明 |
| --- | --- | --- |
| PB6 / USART1_TX | DI / TXD | MCU 发送到收发器 |
| PB7 / USART1_RX | RO / RXD | 收发器返回 MCU |
| 3.3 V | VCC | 必须确认所用模块支持 3.3 V 逻辑 |
| GND | GND | MCU、收发器、传感器共地 |
| — | A / B | 接测力变送器 RS485 A/B，若无响应先核对极性 |

如果收发器需要手动 DE/RE，在配置头中把 `TEMPLATE_LOADCELL_RS485_DE_ENABLED` 改为 `1`，并定义实际 GPIO 端口和引脚。工程不会猜测未知 PCB 的 DE 引脚。

常用测力参数也集中在同一配置头：`TEMPLATE_LOADCELL_SLAVE_ADDRESS`（从站地址）、`TEMPLATE_LOADCELL_POLL_INTERVAL_MS`（轮询周期）、`TEMPLATE_LOADCELL_RETRY_COUNT`（重试次数）和 `TEMPLATE_LOADCELL_UART_TIMEOUT_MS`（单次 UART 超时）。

应用也可以在需要数据时同步读取：

```c
#include "stm32_template.h"

stm32_template_data_t data;
uint32_t valid = stm32_template_read(&data);

if ((valid & STM32_TEMPLATE_DATA_LOADCELL) != 0U)
{
    float left = data.loadcell_channel1;
    float right = data.loadcell_channel2;
    float total = data.loadcell_total;
}
```

`loadcell_channel1_status` 和 `loadcell_channel2_status` 的含义：`0` 成功，`-1` 参数错误，`-2` UART 错误，`-3` 超时，`-4` 回包格式错误，`-5` CRC 错误。不能仅凭数值是否为 0 判断有效性，应检查 `valid_mask` 和状态字段。

## 模块开关与默认引脚

正常使用只修改 `legacy/nun_dx_original/Core/Inc/stm32_template_config.h`。`1` 表示启用，`0` 表示关闭。STM32F103C8 资源有限，建议一次只开启实际需要的模块。

| 模块名称 | 启用宏 | 默认总线/引脚定义 | 地址或接口 | 统一返回 |
| --- | --- | --- | --- | --- |
| 双通道测力传感器 | `TEMPLATE_USE_LOADCELL` | USART1：PB6 TX、PB7 RX | 230400 8N1；Modbus 地址 1 | 通道 1、通道 2、合计、状态、单位、小数位 |
| 飞镖板 OLED UI | `TEMPLATE_USE_DART_OLED_UI` | I2C1：PB8 SCL、PB9 SDA；并使用 CAN | 原业务 OLED/键盘协议 | 独立 UI，无统一读数 |
| 飞镖板 CAN 链路 | `TEMPLATE_USE_DART_CAN_LINK` | CAN1：PA11 RX、PA12 TX | 需要外部 CAN 收发器 | 独立遥测接口 |
| 串口日志 | `TEMPLATE_USE_CONSOLE_UART` | USART3：PB10 TX、PB11 RX | 115200 8N1 | `printf`/自定义日志 |
| 板载 PWM | `TEMPLATE_USE_BOARD_PWM` | PA3/TIM2_CH4，PB4/TIM3_CH1 | CubeMX PWM | 独立 PWM 接口 |
| 0.9 寸 I2C OLED | `DX_USE_OLED` | I2C1：PB8/PB9 | 0x3C | 绘图 API |
| AHT10 温湿度 | `DX_USE_AHT10` | I2C1：PB8/PB9 | 0x38 | 温度、湿度 |
| AT24C64 EEPROM | `DX_USE_AT24C64` | I2C1：PB8/PB9 | 0x50 | 读写 API |
| AS5600 磁编码器 | `DX_USE_AS5600` | I2C1：PB8/PB9 | 0x36 | 角度 |
| BH1750 光照 | `DX_USE_BH1750` | I2C1：PB8/PB9 | 0x23 | lux |
| MPU6050 六轴 IMU | `DX_USE_MPU6050` | I2C1：PB8/PB9 | 0x68 | 加速度、陀螺、温度原始值 |
| GPIO 按键 | `DX_USE_KEY` | 调用 `key_bind()` 指定；示例 PB0 | GPIO 输入 | 按键事件 |
| 舵机 | `DX_USE_SERVO` | 调用 `servo_init()` 选择 TIM/PWM 引脚 | 多组 PA/PB/PC PWM 映射 | 角度控制 |
| ST7789 LCD | `DX_USE_ST7789` | 调用配置结构指定 SPI、CS/DC/RST/BL | SPI1/SPI2 可选 | RGB565 绘图 API |
| TB6612 直流电机 | `DX_USE_TB6612` | PWM 可选 PA8/PA9、PA0/PA1、PA6/PA7、PB6/PB7；方向脚调用时指定 | GPIO + PWM | 方向、制动、速度 |
| ZDT Emm_V5 步进驱动 | `DX_USE_ZDT_EMM_V5` | USART3：PB10 TX、PB11 RX | `TEMPLATE_DX_UART_BUS` | 命令发送 API |

引脚冲突和电气注意事项见 [默认引脚与资源冲突](docs/PINOUT.md)。其中 PB6/PB7 已被默认测力通信占用；启用舵机或 TB6612 时不要再次选择 TIM4 的 PB6/PB7。

## 接口设计

```text
应用 / main.c
  -> stm32_template_init() / stm32_template_read()
  -> components/template        稳定公共接口
  -> Core/Src/stm32_template_port.c  板级句柄与引脚适配
  -> components/loadcell        测力 Modbus 与裸机轮询服务
  -> components/nun_dx          可选设备、驱动、PID、FIFO、状态机
  -> STM32 HAL
```

`stm32_template_read()` 会直接发起一次同步采样，不依赖后台任务或隐藏的全局更新。更换板卡时主要修改 `.ioc`、配置头和 `stm32_template_port.c`，设备协议层无需感知 `huart1`、`hi2c1` 等具体句柄。

## 构建

### Keil MDK

1. 安装 Keil MDK 5 和 STM32F1 Device Family Pack。
2. 打开 `legacy/nun_dx_original/MDK-ARM/stm32f103_template.uvprojx`。
3. 选择 `stm32f103_template`，执行 Rebuild。
4. 在 Target Options 中确认器件为 STM32F103C8Tx，并配置 ST-Link/J-Link。

工程文件内只使用相对路径，可以整体移动仓库。构建脚本会自动寻找常见 Keil 安装位置；若安装在其他目录，显式传入 `-Uv4Path 'X:\path\UV4.exe'`，不要把本机绝对路径写进 `.uvprojx`。

可重新生成并验证工程：

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\generate_keil_project.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build_keil.ps1
```

### GCC/CMake

需要 CMake、GNU Arm Embedded Toolchain，以及 Ninja 或 MinGW Make：

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build_gcc.ps1
```

输出位于 `build/stm32f103_template.{elf,hex,bin}`，烧录地址为 `0x08000000`。

## 目录结构

```text
legacy/nun_dx_original/          唯一固件工程
  app/dart_board/                 可选飞镖板 OLED/CAN 业务
  components/loadcell/            测力 Modbus 驱动与裸机服务
  components/template/            稳定公共 API
  components/nun_dx/              NUN_DX 可选模块
  Core/                          CubeMX 板级代码与统一配置
  Drivers/                       STM32 HAL/CMSIS
  MDK-ARM/                       唯一 Keil 工程
  Startup/                       GCC 启动文件
  stm32f103_template.ioc         唯一 CubeMX 配置
docs/                            模块、引脚、许可、迁移记录
tools/                           工程生成与构建脚本
```

自研目录采用小写 `snake_case`，STM32Cube 的 `Core`、`Drivers`、`Startup` 和 Keil 的 `MDK-ARM` 保留工具链惯例。NUN_DX 上游的 `DX_*` 公共文件名与 API 为兼容性保留；其中原含小数点的 OLED 文件名已规范化。

旧 NUN 独立工程、FreeRTOS、旧任务文件、未引用的原始 ZDT 串口示例、F4 Flash 草稿、重复 Keil/CubeMX 工程和编译产物均已删除；迁移前内容仍可从 Git 提交 `08b1ea2` 审计或恢复。

## 验证状态

- GCC 默认裸机配置：0 warning；Flash 27,920 B（42.60%），RAM 3,608 B（17.62%）。
- Keil ARMCC 5.06 默认裸机配置：0 error / 0 warning；Code 13,326 B，RO 310 B，RW 40 B，ZI 2,856 B。
- 全模块宏组合 GCC 检查：0 warning；Flash 45,972 B（70.15%），RAM 6,128 B（29.92%）。实际同时启用前仍需解决引脚和串口资源冲突。
- 当前环境未连接实物，因此不能替代上板通信、传感器标定和力值准确度验证。首次部署请重点核对 RS485 A/B、从站地址、波特率、单位和小数位。

## 文档

- [模块与 API 手册](docs/MODULES.md)
- [默认引脚与资源冲突](docs/PINOUT.md)
- [来源与迁移审计](docs/SOURCE_AUDIT.md)
- [开源合规说明](docs/OPEN_SOURCE_COMPLIANCE.md)
- [第三方组件声明](THIRD_PARTY_NOTICES.md)
- [项目总结](docs/PROJECT_SUMMARY.md)

## 许可

本仓库的项目所有者、最终维护者和原创整合部分权利人为 **zzz**，完整边界见 [著作权、项目权属与许可声明](COPYRIGHT.md)。

本整合仓库按 GNU AGPL v3 发布，见 [LICENSE](LICENSE)。开源许可不构成 `zzz` 将著作权、官方仓库控制权或维护者身份转让给第三方；同时，NUN_DX、STM32 HAL/CMSIS、ZDT 等第三方内容的既有权利仍归各自权利人所有。具体来源与声明见 [第三方组件声明](THIRD_PARTY_NOTICES.md)和[开源合规说明](docs/OPEN_SOURCE_COMPLIANCE.md)。
