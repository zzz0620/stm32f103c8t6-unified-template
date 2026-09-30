# 工作流

1. 阅读 `AGENTS.md`、`README.md`、项目总结和相关模块文档。
2. 在 `legacy/nun_dx_original/Core/Inc/stm32_template_config.h` 确认功能边界，检查 `docs/PINOUT.md` 的资源冲突。
3. 最小化修改公共 facade；板差异优先落在 `stm32_template_port.c`。
4. 执行 GCC Release 构建并检查 Flash/RAM。
5. 涉及 Keil 文件时运行 `tools/generate_keil_project.ps1` 后全量 Rebuild。
6. 能连接硬件时按“供电/时钟 -> SWD -> RS485 A/B -> 单模块 -> 组合功能”顺序验证。
7. 更新 `docs/PROJECT_SUMMARY.md`，记录结果、未验证项和风险。

