# 项目总结

更新时间：2026-10-01

公开仓库：<https://github.com/zzz0620/stm32f103c8t6-unified-template>。发布者已确认拥有或获授权公开第二份称重工程与 ZDT 代码。

项目所有者、最终维护者及原创整合部分权利人统一署名为 `zzz`；权利边界记录在根目录 `COPYRIGHT.md`，第三方组件既有权利不受影响。

## 最终成果

- 唯一可部署工程位于 `legacy/nun_dx_original`，同时提供 Keil、CubeMX 和 GCC/CMake 入口。
- 默认固件为裸机实现，不依赖 FreeRTOS；USART1 直接轮询双通道 Modbus 测力传感器。
- 所有模块统一由 `Core/Inc/stm32_template_config.h` 控制，默认仅启用测力模块。
- `components/template` 提供稳定 facade，`Core/Src/stm32_template_port.c` 隔离 CubeMX 句柄，模块之间不直接耦合。
- NUN_DX 的设备、基础驱动、PID、FIFO、状态机和已适配 ZDT 驱动均保留，可按宏选择。
- README 已给出模块名称、宏、默认引脚、接线、构建和部署方法。
- 工程自有目录使用小写 `snake_case`；CubeMX、Keil 约定目录和 NUN_DX `DX_*` 公共 API 保持兼容命名。

## 清理结果

- 删除旧 `firmware` 工程、迁移前重复快照、旧 Keil/CubeMX 工程与用户态 IDE 文件。
- 删除 FreeRTOS、旧任务层、F4 Flash 草稿、重复头文件和未参与构建的原始 ZDT 串口示例。
- 精简 HAL/CMSIS 到 STM32F103C8 当前构建需要的文件；保留许可证、来源 README 和审计记录。
- 迁移前内容仍可从 Git 提交 `08b1ea2` 审计或恢复。

## 验证结果

| 验证项 | 结果 |
| --- | --- |
| GCC Release 默认裸机配置 | 通过，0 warning；Flash 27,920 / 65,536 B，RAM 3,608 / 20,480 B |
| Keil ARMCC 5.06 默认裸机配置 | 通过，0 error / 0 warning；Code 13,326 B，RO 310 B，RW 40 B，ZI 2,856 B |
| 全模块宏组合交叉编译 | 通过，0 warning；Flash 45,972 / 65,536 B，RAM 6,128 / 20,480 B |
| 关闭测力模块的宏组合 | 通过，0 warning；确认主循环不会留下未使用变量 |
| Keil 路径检查 | 通过；源码与包含目录均为相对路径，所有路径存在，无旧目标名 |
| 实物烧录与传感器联调 | 未执行；当前环境未连接目标板 |

## 风险与部署检查

- 软件编译通过不能代替实物验证；首次上板必须核对 RS485 A/B、地址 1、230400 8N1、供电和共地。
- 默认假设 RS485 收发器自动控制方向；需要 DE/RE 的板卡必须在配置头中启用并填写实际 GPIO。
- 全模块可编译不代表可同时接线；启用前按 `docs/PINOUT.md` 解决 USART、I2C、PWM 和 CAN 引脚冲突。
- 传感器力值准确度依赖变送器标定和单位/小数位寄存器，必须用已知砝码完成现场复核。

