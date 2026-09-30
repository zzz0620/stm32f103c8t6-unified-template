/**
 * @file    DX_driver_delay.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   延时驱动源文件
 *          实现 ms 级(HAL 库)与 us 级(SysTick 计数)延时
 */

#include "DX_driver_delay.h"

static uint32_t fac_us;   /* 1us 对应的 SysTick 计数值(系统主频 / 1000000) */

/**
 * @brief  延时初始化
 *         根据系统主频计算 1us 所需的 SysTick 计数值,供 DX_Delay_us 使用
 * @retval 无
 */
void DX_Delay_Init(void)
{
  fac_us = SystemCoreClock / 1000000U;
}

/**
 * @brief  毫秒级延时
 *         直接调用 HAL 库 HAL_Delay 实现,精度受 SysTick 中断影响
 * @param  ms: 延时毫秒数
 * @retval 无
 */
void DX_Delay_ms(uint32_t ms)
{
  HAL_Delay(ms);
}

/**
 * @brief  微秒级延时
 *         通过读取 SysTick 递减计数器 VAL 实现精确 us 级延时
 * @param  us: 延时微秒数
 * @retval 无
 * @note   SysTick 为递减计数器,VAL 从 LOAD 向下计数到 0 后重载
 */
void DX_Delay_us(uint32_t us)
{
  uint32_t ticks = us * fac_us;             /* 需要等待的总计数次数           */
  uint32_t start = SysTick->VAL;            /* 起始计数值                     */
  uint32_t reload = SysTick->LOAD;          /* SysTick 重载值                 */

  while (ticks)
  {
    uint32_t cur = SysTick->VAL;            /* 当前计数值                     */
    /* 计算本次轮询间隔内已计数的次数(考虑计数器翻转重载的情况) */
    uint32_t elapsed = (start >= cur) ? (start - cur) : (reload + 1 - cur + start);
    if (elapsed >= ticks) break;            /* 已达到目标延时,退出            */
    ticks -= elapsed;                       /* 扣除已计数的次数               */
    start = SysTick->LOAD;                  /* 重置起点为重载值               */
  }
}
