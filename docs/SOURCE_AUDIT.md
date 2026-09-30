# 来源与迁移审计

## 输入归档

| 归档 | SHA-256 | 发现 |
| --- | --- | --- |
| `NUN_DX_STM32F1c8t6__-open-source-library.zip` | `4135936A008D10F2451152979805126880D8735704B3C1B110E8CC58D6C9B4D9` | 含完整 Git 元数据、AGPL v3 文本、NUN_DX 源码和 CMSIS/HAL 资料 |
| `rm2026-dartloadcell-stm32.zip` | `E21EB0E5542E288FA4B4861FE790EB23D35494A42BE8537FFC38CB671DC93259` | 含可编译的 Keil/CubeMX/FreeRTOS 称重工程；未发现顶层许可证 |

NUN_DX 内嵌 Git HEAD：`1c6ce771d3f1bc943774c563be059d6255e27147`，提交说明“修复一些存在问题”，提交日期 2026-09-02。

## 最终迁移策略

- 两份工程合并为 `legacy/nun_dx_original/` 下的唯一可部署工程，不再保留第二份源码副本。
- 称重工程提供 STM32F103C8 CubeMX 板级配置；NUN_DX 的可用模块位于 `components/nun_dx/`，通过中央配置宏选择。
- 原始来源的两个 README 保留在 `docs/original/`；输入 ZIP 的 SHA-256 和 NUN_DX Git 提交号用于复核。
- 迁移前目录状态可从本仓库提交 `08b1ea2` 审计或恢复。
- 不提交 `source_audit/`、IDE 用户状态和任何构建输出。

## 已处理的遗留项

- F4 风格 `flash.*`/`bsp_flash.*` 与 STM32F103 不兼容且从未进入原 Keil 构建，已删除。
- FreeRTOS、旧任务文件与裸机直接轮询方案重复，已删除；测力读取不再依赖任务全局变量。
- ZDT 原始目录是特定平台 UART 示例且未被工程引用，已删除；保留并构建 `DX_device_zdt_emm_v5.*` 的 STM32 HAL 适配版本。
- 重复 Keil/CubeMX 项目、旧 NUN 独立工程、重复驱动头和 IDE 用户文件均已删除。
- NUN_DX 的部分设备 `*_test()` 会自行重配引脚/外设，生产固件应使用正常 API 和已附着的 CubeMX 句柄。
- 第二份工程和 ZDT 材料未附独立许可证；发布者已明确确认拥有或获授权公开，授权凭据应由发布者长期保存。

## 排除的非源码内容

- 原 `.git/` 对象库和引用；
- Keil `Objects/Listings`、GCC build、HEX/BIN/AXF/MAP 等可重建产物；
- `.uvguix.*`、`.uvoptx`、IDE 缓存和操作系统元数据。

这些排除项不影响从仓库重建固件。

