# Third-Party Notices

This repository contains or derives from the following components. Copyright notices in individual files remain in force.

## NUN_DX STM32F103 library

- Upstream: `https://gitee.com/yang-changzhi11/NUN_DX_STM32F1c8t6__-open-source-library.git`
- Audited commit: `1c6ce771d3f1bc943774c563be059d6255e27147`
- Author markings: YCZ / 杨长治
- License indication in the supplied archive: GNU Affero General Public License version 3
- Preserved source snapshot: `legacy/nun_dx_original/`

## STM32CubeF1 HAL

- Upstream tag: STM32CubeF1 `v1.8.6`
- STM32CubeF1 commit: `42472d601caa457b02d849c7de5f9792629ee324`
- HAL submodule commit: `77fbb30b7a1d02533980400083e48c559aae5a4f`
- License file: `firmware/Drivers/STM32F1xx_HAL_Driver/LICENSE.txt`
- Copyright: STMicroelectronics

The ADC, ADC extension and SPI files missing from the supplied load-cell archive were restored from the matching official v1.8.6 source; the package was not upgraded.

## Arm CMSIS and STM32F1 device CMSIS

- License: Apache License 2.0 and/or the notices carried in individual ST files
- License file: `firmware/Drivers/CMSIS/LICENSE.txt`
- Copyright: Arm Limited and STMicroelectronics

## FreeRTOS Kernel

- Version family: 10.3.1, matching the supplied CubeMX project
- Upstream tag: `V10.3.1-kernel-only`
- Audited commit: `88e32327e975ddde97c390bc5b6c1f8e7d9d239e`
- License: MIT
- License file: `firmware/Middlewares/Third_Party/FreeRTOS/Source/LICENSE`

The GCC Cortex-M3 portable layer was restored from this matching official tag; the RTOS was not upgraded.

## ZDT Emm_V5 example/protocol code

Source comments attribute the original example to ZHANGDATOU / 张大头闭环伺服. No standalone license was found in the supplied material. Public redistribution should be confirmed with the relevant rightsholder; see `docs/OPEN_SOURCE_COMPLIANCE.md`.

