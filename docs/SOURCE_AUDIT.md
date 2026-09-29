# 来源与迁移审计

## 输入归档

| 归档 | SHA-256 | 发现 |
| --- | --- | --- |
| `NUN_DX_STM32F1c8t6__-open-source-library.zip` | `4135936A008D10F2451152979805126880D8735704B3C1B110E8CC58D6C9B4D9` | 含完整 Git 元数据、AGPL v3 文本、NUN_DX 源码和 CMSIS/HAL 资料 |
| `rm2026-dartloadcell-stm32.zip` | `E21EB0E5542E288FA4B4861FE790EB23D35494A42BE8537FFC38CB671DC93259` | 含可编译的 Keil/CubeMX/FreeRTOS 称重工程；未发现顶层许可证 |

NUN_DX 内嵌 Git HEAD：`1c6ce771d3f1bc943774c563be059d6255e27147`，提交说明“修复一些存在问题”，提交日期 2026-09-02。

## 迁移策略

- 第二份工程作为 `firmware/` 的可部署硬件基线。
- NUN_DX 的可用源码迁入 `firmware/Components/NUN_DX/` 并做 CubeMX 共存适配。
- 第一份工程的完整源码/库资料保留在 `legacy/nun_dx_original/`，排除其嵌入 `.git`、旧编译产物和每用户 IDE 状态。
- 两份原 README 保留在 `docs/original/`。
- 不提交解压审计临时目录和任何构建输出；输入 ZIP 的哈希用于复核。

## 已知遗留项

- `firmware/Object/flash.c/.h` 引用 STM32F4 风格 API 和缺失的 `struct_typedef.h`，原 Keil 工程也未编译它们。为避免删除用户来源文件，现保留但明确排除在新 Keil/CMake 构建之外。
- ZDT 原始目录中含特定平台 UART 示例；集成构建使用 `DX_device_zdt_emm_v5.*` 的 STM32 HAL 适配版本，原文件仅作来源参考。
- NUN_DX 的部分设备 `*_test()` 会自行重配引脚/外设，集成项目不要直接在生产固件调用这些测试函数；应使用正常 API 和已附着的 CubeMX 句柄。
- 第二份工程缺少许可证，ZDT 示例也未找到独立授权文本；公开发布前需按合规清单确认。

## 排除的非源码内容

- 原 `.git/` 对象库和引用；
- Keil `Objects/Listings`、GCC build、HEX/BIN/AXF/MAP 等可重建产物；
- `.uvguix.*`、`.uvoptx`、IDE 缓存和操作系统元数据。

这些排除项不影响从仓库重建固件。

