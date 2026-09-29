# STM32F103C8T6 Unified Template

[![MCU](https://img.shields.io/badge/MCU-STM32F103C8T6-03234B)](https://www.st.com/en/microcontrollers-microprocessors/stm32f103c8.html)
[![Toolchain](https://img.shields.io/badge/toolchain-Keil%20MDK%20%7C%20GNU%20Arm-blue)](#已验证基线)
[![License](https://img.shields.io/badge/license-AGPL--3.0--only-orange)](LICENSE)

面向 STM32F103C8T6 的低耦合、可上板、可学习的统一工程模板。

> GitHub 仓库标准名称：`stm32f103c8t6-unified-template`  
> 默认固件 Target：`stm32f103_template`

仓库地址：<https://github.com/zzz0620/stm32f103c8t6-unified-template>

这是将 **NUN_DX STM32F103 开源库**与 **rm2026 飞镖称重工程**整合后的可部署模板。默认固件保留称重板的 CubeMX、FreeRTOS、双串口称重、OLED、CAN、PWM 和 GPIO 配置，同时把 NUN_DX 的驱动、传感器、执行器、PID 与状态机迁入统一组件目录。

目标不是把所有功能强耦合在 `main.c`，而是提供稳定的应用入口与可替换的板级适配层：

```text
应用代码
  -> stm32_template_init / stm32_template_read
  -> Components/Template（稳定接口）
  -> Core/Src/stm32_template_port.c（板级适配）
  -> NUN_DX / 称重任务 / STM32 HAL
```

## 特性

- 一个稳定的应用入口：初始化和数据读取不暴露具体 CubeMX 句柄。
- 一个板级适配文件：换板时集中修改映射，不改设备层和业务层。
- 两套可重建工程：新 Keil MDK 工程与 GCC/CMake 工程。
- 一个中央模块配置：按需启用传感器、显示、电机和控制模块。
- 完整来源可追溯：保留两个输入工程的配置、源码、库与原 README。
- 面向真实部署：记录引脚、电气、资源、许可和实物验证边界。

## 导航

- [已验证基线](#已验证基线)
- [最快上手：Keil](#最快上手keil)
- [最快上手：统一读取接口](#最快上手统一读取接口)
- [启用模块](#启用模块)
- [GCC 构建](#gcc-构建)
- [文档索引](#文档索引)
- [许可](#许可)

## 已验证基线

- MCU：STM32F103C8T6，Cortex-M3，72 MHz，64 KB Flash，20 KB SRAM
- Keil MDK-ARM：重新建立的工程可完整 Rebuild，0 error / 0 warning
- GCC：CMake + `arm-none-eabi-gcc` 可构建 ELF、HEX、BIN，0 warning
- 默认固件：FreeRTOS + 双路称重 + OLED UI + CAN + USART1/USART3
- CubeMX：`firmware/stm32f103_template.ioc`

本仓库完成的是静态检查和交叉编译验证；没有连接用户实物进行传感器、CAN、称重标定或烧录测试。第一次上板请先核对 [引脚与冲突说明](docs/PINOUT.md)。

## 最快上手：Keil

1. 克隆仓库：`git clone https://github.com/zzz0620/stm32f103c8t6-unified-template.git`。
2. 安装 Keil MDK 5 与 STM32F1 Device Family Pack。
3. 打开 `firmware/MDK-ARM/stm32f103_template.uvprojx`。
4. 选择 `stm32f103_template` Target，执行 Rebuild。
5. 在 Options for Target 中选择自己的 ST-Link/J-Link，确认芯片为 STM32F103C8Tx。
6. 烧录后用 USART3（PB10/PB11，115200 8N1）观察 `printf` 输出；USART1（PB6/PB7，230400 8N1）用于称重模块通信。

Keil 工程可重复生成：

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\generate_keil_project.ps1
```

## 最快上手：统一读取接口

默认 `main.c` 已在所有 CubeMX 外设初始化后调用一次 `stm32_template_init()`。业务任务中只需要：

```c
#include "stm32_template.h"

stm32_template_data_t data;
uint32_t valid = stm32_template_read(&data);

if ((valid & STM32_TEMPLATE_DATA_LOADCELL) != 0U)
{
    printf("total=%.2f\r\n", data.loadcell_total);
}
```

`valid_mask` 表示本次哪些数据有效。接口返回的数据包括双路称重、合计、状态、单位、小数位，以及按配置启用的 AHT10、AS5600、BH1750、MPU6050 数据。传感器读取是同步调用，不应放在中断函数中。

## 启用模块

只修改 `firmware/Core/Inc/stm32_template_config.h` 中的开关，例如：

```c
#define DX_USE_AHT10    1
#define DX_USE_BH1750   1
#define DX_USE_MPU6050  1
```

默认 I2C 设备复用 CubeMX 已初始化的 I2C1（PB8/PB9），不会再次初始化外设。需要换板时，通常只需修改：

- `firmware/Core/Src/stm32_template_port.c`：映射 HAL 句柄和业务数据源；
- `firmware/Core/Inc/stm32_template_config.h`：功能和总线选择；
- `.ioc`：实际时钟、GPIO、DMA 和中断配置。

需要初始化参数的执行器（舵机、TB6612、ST7789、按键、ZDT Emm_V5）保留各自清晰的直接 API，不被强塞进通用读数结构。调用示例和返回值见 [模块手册](docs/MODULES.md)。

## GCC 构建

依赖 CMake、GNU Arm Embedded Toolchain，以及 Ninja 或 MinGW Make。推荐使用仓库脚本，它会选择当前可用的生成器：

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build_gcc.ps1
```

输出位于 `build/stm32f103_template.{elf,hex,bin}`。可使用 STM32CubeProgrammer、OpenOCD 或调试器烧录；烧录地址为 `0x08000000`。

命令行验证 Keil 工程：

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build_keil.ps1
```

## 目录

```text
firmware/
  Components/Template/       稳定公共 API，不依赖具体任务全局变量
  Components/NUN_DX/         已适配的 NUN_DX 模块
  Core/                      CubeMX 代码与当前板级 port
  Drivers/                   STM32 HAL/CMSIS
  Middlewares/               FreeRTOS
  MDK-ARM/                   新 Keil 工程
  Object/                    原称重/OLED 业务模块
legacy/nun_dx_original/      第一份工程的完整可审计源码快照
docs/original/               两份输入工程的原始 README
tools/                       工程生成与验证脚本
```

`firmware/Object/flash.c/.h` 是第二份工程遗留的 STM32F4 片外 Flash 草稿，未进入构建且不能直接用于 F103；保留它仅用于溯源，详见 [迁移审计](docs/SOURCE_AUDIT.md)。

## 文档索引

- [模块与 API 手册](docs/MODULES.md)
- [默认引脚与资源冲突](docs/PINOUT.md)
- [开源许可评估与发布清单](docs/OPEN_SOURCE_COMPLIANCE.md)
- [第三方组件声明](THIRD_PARTY_NOTICES.md)
- [来源与迁移审计](docs/SOURCE_AUDIT.md)
- [项目总结与验证结果](docs/PROJECT_SUMMARY.md)

## 许可

第一份源工程仓库根目录提供 GNU AGPL v3 文本，因此本整合仓库按 **GNU AGPL v3** 发布，完整条款见 [LICENSE](LICENSE)。保留了原作者信息和修改说明。公开分发前还必须确认第二份称重工程及其中所有非 ST/Arm/FreeRTOS 代码确由发布者拥有或已经获得兼容授权；详见 [开源合规说明](docs/OPEN_SOURCE_COMPLIANCE.md)。

