# Third-Party Notices

This repository contains or derives from the following components. Copyright notices in individual files remain in force.

## NUN_DX STM32F103 library

- Upstream: `https://gitee.com/yang-changzhi11/NUN_DX_STM32F1c8t6__-open-source-library.git`
- Audited commit: `1c6ce771d3f1bc943774c563be059d6255e27147`
- Author markings: YCZ / 杨长治
- License indication in the supplied archive: GNU Affero General Public License version 3
- Integrated source: `legacy/nun_dx_original/components/nun_dx/`

## STM32CubeF1 HAL

- Upstream tag: STM32CubeF1 `v1.8.6`
- STM32CubeF1 commit: `42472d601caa457b02d849c7de5f9792629ee324`
- HAL submodule commit: `77fbb30b7a1d02533980400083e48c559aae5a4f`
- License file: `legacy/nun_dx_original/Drivers/STM32F1xx_HAL_Driver/LICENSE.txt`
- Copyright: STMicroelectronics

The ADC, ADC extension and SPI files missing from the supplied load-cell archive were restored from the matching official v1.8.6 source; the package was not upgraded.

## Arm CMSIS and STM32F1 device CMSIS

- License: Apache License 2.0 and/or the notices carried in individual ST files
- License file: `legacy/nun_dx_original/Drivers/CMSIS/LICENSE.txt`
- Copyright: Arm Limited and STMicroelectronics

## ZDT Emm_V5 example/protocol code

Source comments attribute the protocol implementation to ZHANGDATOU / 张大头闭环伺服. The repository retains only the STM32 HAL adapter at `legacy/nun_dx_original/components/nun_dx/device/DX_device_zdt_emm_v5.*`; the unreferenced platform example was removed. No standalone license was found in the supplied material. The publisher confirmed ownership or authorization for public redistribution; see `docs/OPEN_SOURCE_COMPLIANCE.md`.

