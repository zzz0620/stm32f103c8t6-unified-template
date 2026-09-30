/**
 * @file    DX_driver_timer_trigger.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   定时器触发驱动源文件
 *          根据指定频率/周期自动计算 PSC 与 ARR,启动定时器周期性触发
 */

#include "DX_driver_timer_trigger.h"

#define TIM_CLK_HZ  72000000UL   /* 定时器时钟频率 (72MHz) */

/**
 * @brief  启动定时器周期性触发
 *         将用户指定的频率或周期换算为定时器 PSC 与 ARR,并启动定时器中断
 * @param  idx  : 定时器索引,见 tim_index_enum
 * @param  value: 触发频率或周期值,含义由 unit 决定
 * @param  unit : 时间单位,见 tim_unit_enum
 * @retval 无
 * @note   当总分频系数超过 65536 时,自动拆分 PSC 与 ARR 以扩展定时范围
 */
void tim_trigger_start (tim_index_enum idx, uint32 value, tim_unit_enum unit)
{
    uint32 freq_hz;

    /* 将不同单位统一换算为频率 (Hz) */
    switch (unit)
    {
        case TIM_UNIT_HZ: freq_hz = value;                          break;
        case TIM_UNIT_MS: freq_hz = (value > 0) ? 1000U / value : 1; break;
        case TIM_UNIT_US: freq_hz = (value > 0) ? 1000000U / value : 1; break;
        default: return;
    }

    if (freq_hz == 0) return;   /* 频率为 0,直接退出 */

    uint32 total_div = TIM_CLK_HZ / freq_hz;   /* 总分频系数 */
    uint32 psc, arr;

    /* 根据总分频系数是否超出 16 位范围,选择是否使用预分频 */
    if (total_div <= 65536)
    {
        psc = 0;
        arr = total_div - 1;
    }
    else
    {
        psc = (total_div - 1) / 65536;
        arr = (TIM_CLK_HZ / ((psc + 1) * freq_hz)) - 1;
    }

    tim_init(idx, psc, arr);            /* 初始化定时器 */
    tim_set_callback(idx, NULL, NULL);  /* 清空回调,触发逻辑由 ISR 处理 */
    tim_enable_interrupt(idx);          /* 使能定时器中断 */
    tim_start(idx);                     /* 启动定时器 */
}

/**
 * @brief  停止定时器触发
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 无
 */
void tim_trigger_stop (tim_index_enum idx)
{
    tim_stop(idx);
}


/**
 * @brief  定时器触发测试:以 500ms 周期启动 TIM1 周期触发
 * @param  无
 * @retval 无
 */
void tim_trigger_test (void)
{
	tim_trigger_start(TIM_1,500,TIM_UNIT_MS);
}

