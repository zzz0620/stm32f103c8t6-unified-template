# Project working rules

- 每次修改前先阅读根 README、`docs/PROJECT_SUMMARY.md`、`docs/ALIGNMENT.md` 和相关模块文档。
- 唯一可部署基线是 `legacy/nun_dx_original/stm32f103_template.ioc` 与同目录下的新 Keil 工程。
- 公共 API 保持在 `components/template`，板级句柄只进入 `stm32_template_port.c`。
- 新模块默认关闭，启用前记录引脚、Flash、RAM、中断和总线冲突。
- 默认固件保持裸机，不重新引入 RTOS；不升级 HAL/CMSIS 版本，确需变更时先说明兼容性和回退方案。
- 不删除唯一工程或来源审计记录；不得把 `source_audit/` 和构建产物提交到公开仓库。
- 修改后至少完成 GCC 构建；影响 Keil/CubeMX/启动代码时同时完成 Keil Rebuild。
- 完成任务后更新 `docs/PROJECT_SUMMARY.md`；出现偏好纠正时更新 `docs/ALIGNMENT.md`。

