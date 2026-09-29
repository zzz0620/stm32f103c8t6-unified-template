/**
 * @file    DX_driver_timer_callback.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   定时器回调与中断服务源文件
 *          提供各定时器(TIM1~TIM4)的弱定义周期回调及中断服务函数
 */

#include "DX_driver_timer.h"
#include "DX_common_headfile.h"

#if defined(HAL_TIM_MODULE_ENABLED)

/**
 * @brief  TIM1 周期触发回调(弱定义)
 *         用户可在其他文件中重定义此函数以实现自定义逻辑
 * @retval 无
 */
__weak void tim1_trigger_callback (void) {



//	printf("tim1_trigger_callback_test\r\n");  /* 打印TIM1 周期触发回调测试                          */	
	
}
	
/**
 * @brief  TIM2 周期触发回调(弱定义)
 *         用户可在其他文件中重定义此函数以实现自定义逻辑
 * @retval 无
 */
__weak void tim2_trigger_callback (void) {

  key_scan();
	
}
	
/**
 * @brief  TIM3 周期触发回调(弱定义)
 *         用户可在其他文件中重定义此函数以实现自定义逻辑
 * @retval 无
 */
__weak void tim3_trigger_callback (void) {

}
	
/**
 * @brief  TIM4 周期触发回调(弱定义)
 *         用户可在其他文件中重定义此函数以实现自定义逻辑
 * @retval 无
 */
__weak void tim4_trigger_callback (void) {


}
	
	
	
	
	
	
	
/**
 * @brief  HAL 定时器周期溢出回调
 *         HAL 库在中断处理中调用,根据定时器实例分发到对应的回调函数
 * @param  htim: 定时器句柄指针
 * @retval 无
 */
void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim)
{
    if      (htim->Instance == TIM1) tim1_trigger_callback();   /* TIM1 触发 */
    else if (htim->Instance == TIM2) tim2_trigger_callback();   /* TIM2 触发 */
    else if (htim->Instance == TIM3) tim3_trigger_callback();   /* TIM3 触发 */
    else if (htim->Instance == TIM4) tim4_trigger_callback();   /* TIM4 触发 */
}

/**
 * @brief  TIM1 更新中断服务函数
 * @retval 无
 * @note   默认不启用(留给 PWM);定义 DX_TIM1_TRIGGER_ENABLE 后启用
 */
#ifdef DX_TIM1_TRIGGER_ENABLE
void TIM1_UP_IRQHandler (void) { HAL_TIM_IRQHandler(tim_get_handle(TIM_1)); }
#endif

/**
 * @brief  TIM2 中断服务函数
 * @retval 无
 * @note   默认不启用(留给 PWM);定义 DX_TIM2_TRIGGER_ENABLE 后启用
 */
#ifdef DX_TIM2_TRIGGER_ENABLE
void TIM2_IRQHandler (void)    { HAL_TIM_IRQHandler(tim_get_handle(TIM_2)); }
#endif

/**
 * @brief  TIM3 中断服务函数
 * @retval 无
 * @note   默认不启用(留给 PWM);定义 DX_TIM3_TRIGGER_ENABLE 后启用
 */
#ifdef DX_TIM3_TRIGGER_ENABLE
void TIM3_IRQHandler (void)    { HAL_TIM_IRQHandler(tim_get_handle(TIM_3)); }
#endif

/**
 * @brief  TIM4 中断服务函数
 * @retval 无
 * @note   默认不启用(留给 PWM);定义 DX_TIM4_TRIGGER_ENABLE 后启用
 */
#ifdef DX_TIM4_TRIGGER_ENABLE
void TIM4_IRQHandler (void)    { HAL_TIM_IRQHandler(tim_get_handle(TIM_4)); }
#endif

#endif /* HAL_TIM_MODULE_ENABLED */
