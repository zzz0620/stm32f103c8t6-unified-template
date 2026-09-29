# 项目总结

更新时间：2026-09-29

计划发布仓库名：`stm32f103c8t6-unified-template`（GitHub 小写短横线命名）；本地目录继续使用工作区编号规范。

公开仓库：<https://github.com/zzz0620/stm32f103c8t6-unified-template>。发布者已于 2026-09-29 确认第二工程与 ZDT 代码的公开权利。

## 当前成果

- 以 rm2026 称重项目的 STM32F103C8T6 CubeMX/FreeRTOS 配置为可部署基线。
- 迁移 NUN_DX 的基础驱动、设备、PID、状态机、调试和公共模块。
- 增加 `stm32_template_*` 稳定入口与板级 port，减少业务层和硬件句柄耦合。
- 新建 Keil 工程，并增加 GCC/CMake 构建、启动文件和链接脚本。
- 恢复与原工程版本一致的 HAL ADC/SPI 与 FreeRTOS GCC Cortex-M3 port。
- 保留原始来源、README、许可和第三方声明，记录不可直接用于 F103 的遗留文件。

## 验证结果

| 验证项 | 结果 |
| --- | --- |
| Keil MDK 全量 Rebuild | 通过，0 error / 0 warning；Code 33884，RO 2884，RW 264，ZI 13816 |
| GCC Release 构建 | 通过，0 warning；Flash 46112 / 65536，RAM 14800 / 20480 |
| 11 个 NUN_DX 设备开关逐项编译 | 全部通过；期间发现并解除舵机测试对按键模块的隐式依赖 |
| 11 个设备开关同时开启的编译/链接检查 | 通过；Flash 47676 / 65536，RAM 16752 / 20480；未引用执行器代码会被链接器回收 |
| CubeMX 配置存在 | 通过，芯片 STM32F103C8Tx、72 MHz |
| 统一 API 与 port 解耦 | 通过静态检查和双工具链编译 |
| 实物烧录与外设联调 | 未执行；当前环境未连接目标板 |

## 风险与后续

- RAM 默认占用约 72%，增加 RTOS 任务、队列或大缓冲区时必须重新检查堆栈水位与链接结果。
- 可选模块需要逐一做实物电气和协议验证，特别是电机、CAN、称重标定和 ZDT 回包。
- 公开 GitHub 前需要发布者确认第二份工程和 ZDT 示例的权利状态。
- 修改 `.ioc` 重新生成代码后，应重新运行 Keil 工程生成脚本并用两种工具链 Rebuild。

