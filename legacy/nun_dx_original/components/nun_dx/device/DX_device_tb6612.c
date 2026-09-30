#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_TB6612

/**
 * @file    DX_device_tb6612.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   TB6612 电机驱动源文件
 *          基于 GPIO 与 PWM 驱动实现 TB6612 双 H 桥电机的方向控制与调速
 */

#include "DX_device_tb6612.h"
#include "DX_driver_gpio.h"

#define TB6612_MOTOR_COUNT      8       /* 电机通道总数             */
#define TB6612_PWM_FREQ_HZ      10000   /* PWM 频率 10kHz           */

/**
 * @brief PWM 配置结构体
 *        描述每路电机对应的定时器、通道及引脚映射
 */
typedef struct
{
    tim_index_enum   tim;       /* 定时器索引       */
    pwm_channel_enum ch;        /* PWM 通道         */
    pwm_pin_enum     pwm_pin;   /* PWM 输出引脚     */
} pwm_config_t;

/* 电机通道与 PWM 资源映射表 */
static const pwm_config_t pwm_table[] =
{
    [TB6612_M1_TIM1_CH1] = { TIM_1, PWM_CH_1, PWM_TIM1_CH1_PA8 },
    [TB6612_M2_TIM1_CH2] = { TIM_1, PWM_CH_2, PWM_TIM1_CH2_PA9 },
    [TB6612_M3_TIM2_CH1] = { TIM_2, PWM_CH_1, PWM_TIM2_CH1_PA0 },
    [TB6612_M4_TIM2_CH2] = { TIM_2, PWM_CH_2, PWM_TIM2_CH2_PA1 },
    [TB6612_M5_TIM3_CH1] = { TIM_3, PWM_CH_1, PWM_TIM3_CH1_PA6 },
    [TB6612_M6_TIM3_CH2] = { TIM_3, PWM_CH_2, PWM_TIM3_CH2_PA7 },
    [TB6612_M7_TIM4_CH1] = { TIM_4, PWM_CH_1, PWM_TIM4_CH1_PB6 },
    [TB6612_M8_TIM4_CH2] = { TIM_4, PWM_CH_2, PWM_TIM4_CH2_PB7 },
};

/**
 * @brief GPIO 引脚描述结构体
 */
typedef struct
{
    GPIO_TypeDef *port;     /* GPIO 端口    */
    uint16        pin;      /* 引脚号       */
} gpio_t;

/**
 * @brief 电机运行上下文结构体
 *        保存每路电机的方向控制引脚及初始化标志
 */
typedef struct
{
    gpio_t in1;             /* 方向控制引脚 IN1  */
    gpio_t in2;             /* 方向控制引脚 IN2  */
    uint8  init_flag;       /* 初始化完成标志    */
} motor_ctx_t;

static motor_ctx_t motor_ctx[TB6612_MOTOR_COUNT];   /* 电机上下文数组     */
static gpio_t      stby_io;                         /* STBY 待机控制引脚  */

/**
 * @brief  初始化指定电机
 *         配置方向控制引脚(IN1/IN2)为推挽输出,并启动对应通道的 PWM 输出
 * @param  motor    : 电机通道,见 tb6612_motor_enum
 * @param  in1_port : IN1 方向控制引脚端口
 * @param  in1_pin  : IN1 方向控制引脚号
 * @param  in2_port : IN2 方向控制引脚端口
 * @param  in2_pin  : IN2 方向控制引脚号
 * @retval 无
 */
void tb6612_init (tb6612_motor_enum motor,
                  GPIO_TypeDef *in1_port, uint16 in1_pin,
                  GPIO_TypeDef *in2_port, uint16 in2_pin)
{
    if (motor >= TB6612_MOTOR_COUNT) return;     /* 通道越界保护 */

    motor_ctx_t *ctx = &motor_ctx[motor];
    ctx->in1.port = in1_port; ctx->in1.pin = in1_pin;   /* 记录 IN1 引脚 */
    ctx->in2.port = in2_port; ctx->in2.pin = in2_pin;   /* 记录 IN2 引脚 */

    DX_GPIO_Init(in1_port, in1_pin, DX_OUTPUT_PP);      /* IN1 推挽输出  */
    DX_GPIO_Init(in2_port, in2_pin, DX_OUTPUT_PP);      /* IN2 推挽输出  */

    const pwm_config_t *pwm = &pwm_table[motor];
    pwm_init(pwm->tim, pwm->ch, TB6612_PWM_FREQ_HZ, pwm->pwm_pin);  /* 初始化 PWM */
    pwm_start(pwm->tim, pwm->ch);                       /* 启动 PWM 输出 */

    ctx->init_flag = 1;                                 /* 标记初始化完成 */
}

/**
 * @brief  配置 STBY 待机引脚
 *         将 STBY 引脚配置为推挽输出,用于控制芯片使能/待机
 * @param  port: STBY 引脚端口
 * @param  pin : STBY 引脚号
 * @retval 无
 * @note   调用 tb6612_enable / tb6612_disable 前需先调用本函数
 */
void tb6612_set_stby (GPIO_TypeDef *port, uint16 pin)
{
    stby_io.port = port; stby_io.pin = pin;     /* 记录 STBY 引脚 */
    DX_GPIO_Init(port, pin, DX_OUTPUT_PP);      /* 配置为推挽输出 */
}

/**
 * @brief  使能 TB6612(退出待机模式)
 *         拉高 STBY 引脚,芯片进入工作状态
 * @retval 无
 */
void tb6612_enable (void)
{
    DX_GPIO_SetHigh(stby_io.port, stby_io.pin); /* STBY 拉高,使能 */
}

/**
 * @brief  禁用 TB6612(进入待机模式)
 *         拉低 STBY 引脚,芯片进入低功耗待机状态
 * @retval 无
 */
void tb6612_disable (void)
{
    DX_GPIO_SetLow(stby_io.port, stby_io.pin);  /* STBY 拉低,待机 */
}

/**
 * @brief  电机正转
 *         IN1=1, IN2=0,电机正方向旋转
 * @param  motor: 电机通道,见 tb6612_motor_enum
 * @retval 无
 */
void tb6612_forward (tb6612_motor_enum motor)
{
    if (motor >= TB6612_MOTOR_COUNT) return;    /* 通道越界保护 */
    motor_ctx_t *ctx = &motor_ctx[motor];
    if (!ctx->init_flag) return;                /* 未初始化则返回 */
    DX_GPIO_SetHigh(ctx->in1.port, ctx->in1.pin);   /* IN1 高 */
    DX_GPIO_SetLow (ctx->in2.port, ctx->in2.pin);   /* IN2 低 */
}

/**
 * @brief  电机反转
 *         IN1=0, IN2=1,电机反方向旋转
 * @param  motor: 电机通道,见 tb6612_motor_enum
 * @retval 无
 */
void tb6612_backward (tb6612_motor_enum motor)
{
    if (motor >= TB6612_MOTOR_COUNT) return;    /* 通道越界保护 */
    motor_ctx_t *ctx = &motor_ctx[motor];
    if (!ctx->init_flag) return;                /* 未初始化则返回 */
    DX_GPIO_SetLow (ctx->in1.port, ctx->in1.pin);   /* IN1 低 */
    DX_GPIO_SetHigh(ctx->in2.port, ctx->in2.pin);   /* IN2 高 */
}

/**
 * @brief  电机制动
 *         IN1=1, IN2=1,输出端短路,实现快速制动
 * @param  motor: 电机通道,见 tb6612_motor_enum
 * @retval 无
 */
void tb6612_brake (tb6612_motor_enum motor)
{
    if (motor >= TB6612_MOTOR_COUNT) return;    /* 通道越界保护 */
    motor_ctx_t *ctx = &motor_ctx[motor];
    if (!ctx->init_flag) return;                /* 未初始化则返回 */
    DX_GPIO_SetHigh(ctx->in1.port, ctx->in1.pin);   /* IN1 高 */
    DX_GPIO_SetHigh(ctx->in2.port, ctx->in2.pin);   /* IN2 高 */
}

/**
 * @brief  电机滑行(自由停止)
 *         IN1=0, IN2=0,输出高阻,电机自由滑行至停止
 * @param  motor: 电机通道,见 tb6612_motor_enum
 * @retval 无
 */
void tb6612_stop (tb6612_motor_enum motor)
{
    if (motor >= TB6612_MOTOR_COUNT) return;    /* 通道越界保护 */
    motor_ctx_t *ctx = &motor_ctx[motor];
    if (!ctx->init_flag) return;                /* 未初始化则返回 */
    DX_GPIO_SetLow(ctx->in1.port, ctx->in1.pin);   /* IN1 低 */
    DX_GPIO_SetLow(ctx->in2.port, ctx->in2.pin);   /* IN2 低 */
}

/**
 * @brief  设置电机 PWM 占空比(调速)
 * @param  motor : 电机通道,见 tb6612_motor_enum
 * @param  speed : PWM 占空比,范围 0~10000
 * @retval 无
 */
void tb6612_set_speed (tb6612_motor_enum motor, uint32 speed)
{
    if (motor >= TB6612_MOTOR_COUNT) return;            /* 通道越界保护 */
    if (!motor_ctx[motor].init_flag) return;            /* 未初始化则返回 */

    const pwm_config_t *pwm = &pwm_table[motor];
    pwm_set_duty(pwm->tim, pwm->ch, speed);             /* 设置占空比 */
}

#endif /* DX_USE_TB6612 */
