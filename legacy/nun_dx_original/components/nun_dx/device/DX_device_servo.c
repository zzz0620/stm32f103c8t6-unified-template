#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_SERVO

/**
 * @file    DX_device_servo.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   舵机设备源文件
 *          基于底层 PWM 驱动实现舵机初始化与角度控制
 */

#include "DX_device_servo.h"
#include "DX_common_headfile.h"
/**
 * @brief 舵机引脚映射结构体
 *        用于建立舵机引脚枚举到底层 PWM 资源的对应关系
 */
typedef struct
{
    tim_index_enum   tim;      /* 定时器索引     */
    pwm_channel_enum ch;       /* PWM 通道       */
    pwm_pin_enum     pwm_pin;  /* PWM 引脚映射   */
} servo_map_t;

/**
 * @brief 舵机引脚映射表
 *        以 servo_pin_enum 为索引,记录每个引脚对应的定时器、通道及 PWM 引脚
 */
static const servo_map_t servo_table[] =
{
    [SERVO_TIM1_CH1_PA8]  = { TIM_1, PWM_CH_1, PWM_TIM1_CH1_PA8 },   /* TIM1_CH1 PA8  */
    [SERVO_TIM1_CH2_PA9]  = { TIM_1, PWM_CH_2, PWM_TIM1_CH2_PA9 },   /* TIM1_CH2 PA9  */
    [SERVO_TIM1_CH3_PA10] = { TIM_1, PWM_CH_3, PWM_TIM1_CH3_PA10 },  /* TIM1_CH3 PA10 */
    [SERVO_TIM1_CH4_PA11] = { TIM_1, PWM_CH_4, PWM_TIM1_CH4_PA11 },  /* TIM1_CH4 PA11 */

    [SERVO_TIM2_CH1_PA0]  = { TIM_2, PWM_CH_1, PWM_TIM2_CH1_PA0 },   /* TIM2_CH1 PA0  */
    [SERVO_TIM2_CH2_PA1]  = { TIM_2, PWM_CH_2, PWM_TIM2_CH2_PA1 },   /* TIM2_CH2 PA1  */
    [SERVO_TIM2_CH3_PA2]  = { TIM_2, PWM_CH_3, PWM_TIM2_CH3_PA2 },   /* TIM2_CH3 PA2  */
    [SERVO_TIM2_CH4_PA3]  = { TIM_2, PWM_CH_4, PWM_TIM2_CH4_PA3 },   /* TIM2_CH4 PA3  */

    [SERVO_TIM3_CH1_PA6]  = { TIM_3, PWM_CH_1, PWM_TIM3_CH1_PA6 },   /* TIM3_CH1 PA6  */
    [SERVO_TIM3_CH2_PA7]  = { TIM_3, PWM_CH_2, PWM_TIM3_CH2_PA7 },   /* TIM3_CH2 PA7  */
    [SERVO_TIM3_CH3_PB0]  = { TIM_3, PWM_CH_3, PWM_TIM3_CH3_PB0 },   /* TIM3_CH3 PB0  */
    [SERVO_TIM3_CH4_PB1]  = { TIM_3, PWM_CH_4, PWM_TIM3_CH4_PB1 },   /* TIM3_CH4 PB1  */
    [SERVO_TIM3_CH1_PB4]  = { TIM_3, PWM_CH_1, PWM_TIM3_CH1_PB4 },   /* TIM3_CH1 PB4  */
    [SERVO_TIM3_CH2_PB5]  = { TIM_3, PWM_CH_2, PWM_TIM3_CH2_PB5 },   /* TIM3_CH2 PB5  */
    [SERVO_TIM3_CH3_PC8]  = { TIM_3, PWM_CH_3, PWM_TIM3_CH3_PC8 },   /* TIM3_CH3 PC8  */
    [SERVO_TIM3_CH4_PC9]  = { TIM_3, PWM_CH_4, PWM_TIM3_CH4_PC9 },   /* TIM3_CH4 PC9  */

    [SERVO_TIM4_CH1_PB6]  = { TIM_4, PWM_CH_1, PWM_TIM4_CH1_PB6 },   /* TIM4_CH1 PB6  */
    [SERVO_TIM4_CH2_PB7]  = { TIM_4, PWM_CH_2, PWM_TIM4_CH2_PB7 },   /* TIM4_CH2 PB7  */
    [SERVO_TIM4_CH3_PB8]  = { TIM_4, PWM_CH_3, PWM_TIM4_CH3_PB8 },   /* TIM4_CH3 PB8  */
    [SERVO_TIM4_CH4_PB9]  = { TIM_4, PWM_CH_4, PWM_TIM4_CH4_PB9 },   /* TIM4_CH4 PB9  */
};

/**
 * @brief  初始化舵机对应的 PWM 通道
 * @param  pin : 舵机引脚编号,见 servo_pin_enum
 * @retval 无
 */
void servo_init (servo_pin_enum pin)
{
    const servo_map_t *s = &servo_table[pin];
    pwm_init(s->tim, s->ch, SERVO_FREQ_HZ, s->pwm_pin);   /* 以 50Hz 初始化 PWM */
    pwm_start(s->tim, s->ch);                              /* 启动 PWM 输出      */
}

/**
 * @brief  设置舵机角度
 * @param  pin   : 舵机引脚编号,见 servo_pin_enum
 * @param  angle : 目标角度,范围 0~180°(超出上限将被限制为 180°)
 * @retval 无
 * @note   通过线性映射将角度转换为 PWM 占空比:
 *         0°   -> 0.5ms 高电平 (SERVO_DUTY_MIN)
 *         180° -> 2.5ms 高电平 (SERVO_DUTY_MAX)
 */
void servo_set_angle (servo_pin_enum pin, uint8 angle)
{
    if (angle > 180) angle = 180;                          /* 角度限幅,最大 180° */

    const servo_map_t *s = &servo_table[pin];
    uint32 duty = SERVO_DUTY_MIN + ((uint32)angle * (SERVO_DUTY_MAX - SERVO_DUTY_MIN)) / 180;  /* 角度线性映射为占空比 */
    pwm_set_duty(s->tim, s->ch, duty);                     /* 设置 PWM 占空比 */
}

/**
 * @brief  舵机测试函数
 *         以 SERVO_TIM1_CH1_PA8 为例,初始化后设置角度为 90°
 * @retval 无
 */
void servo_test(void)
{
    servo_init      (SERVO_TIM1_CH1_PA8);           /* TIM1 做 PWM(舵机)   */
    servo_set_angle (SERVO_TIM1_CH1_PA8, 90);       /* 初始 90°            */

#if DX_USE_KEY
    uint8_t angle = 90;

    key_bind(KEY_1, GPIOB, GPIO_PIN_0);             /* 绑定按键            */
    tim_trigger_start(TIM_2, 10, TIM_UNIT_MS);      /* TIM2 做 10ms 触发    */

    while (1)
    {
        if (key_get(KEY_1) == KEY_PRESSED)          /* 短按事件            */
        {
            angle += 10;
            if (angle > 180) angle = 0;
            servo_set_angle(SERVO_TIM1_CH1_PA8, angle);
        }
    }
#endif
}

#endif /* DX_USE_SERVO */
