/**
 * @file    DX_control_pid.c
 * @author  YCZ
 * @date    2026-08-02
 * @brief   PID 控制器源文件
 *          实现位置式与增量式 PID 算法,包含抗积分饱和与输出限幅
 */

#include "DX_control_pid.h"

/**
 * @brief 限制数值到 [min, max] 区间
 * @param  x    待限幅的值
 * @param  min  下限
 * @param  max  上限
 * @retval 限幅后的值
 */
static float pid_constrain(float x, float min, float max)
{
    if (x > max) return max;
    if (x < min) return min;
    return x;
}

/**
 * @brief PID 初始化
 *        清零运行状态,设置模式与参数,默认使能
 * @param pid   PID 句柄
 * @param mode  工作模式(位置式/增量式)
 * @param kp    比例系数
 * @param ki    积分系数
 * @param kd    微分系数
 */
void pid_init(pid_t *pid, pid_mode_enum mode, float kp, float ki, float kd)
{
    if (pid == NULL) return;

    pid->mode   = mode;
    pid->kp     = kp;
    pid->ki     = ki;
    pid->kd     = kd;

    pid->set        = 0.0f;
    pid->feedback   = 0.0f;
    pid->err        = 0.0f;
    pid->err_last   = 0.0f;
    pid->err_prev   = 0.0f;
    pid->integral   = 0.0f;
    pid->out        = 0.0f;

    /* 默认不限幅(用极大值表示) */
    pid->out_max       =  1e9f;
    pid->out_min       = -1e9f;
    pid->integral_max  =  1e9f;
    pid->integral_min  = -1e9f;

    pid->enable    = 1;
}

/**
 * @brief 设置 PID 参数
 * @param pid  PID 句柄
 * @param kp   比例系数
 * @param ki   积分系数
 * @param kd   微分系数
 */
void pid_set_param(pid_t *pid, float kp, float ki, float kd)
{
    if (pid == NULL) return;
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

/**
 * @brief 设置目标值
 * @param pid     PID 句柄
 * @param target  目标值
 */
void pid_set_target(pid_t *pid, float target)
{
    if (pid == NULL) return;
    pid->set = target;
}

/**
 * @brief 设置输出限幅
 * @param pid      PID 句柄
 * @param out_min  输出下限
 * @param out_max  输出上限
 */
void pid_set_limit(pid_t *pid, float out_min, float out_max)
{
    if (pid == NULL) return;
    pid->out_min = out_min;
    pid->out_max = out_max;
}

/**
 * @brief 设置积分项限幅(抗积分饱和)
 * @param pid    PID 句柄
 * @param i_min  积分下限
 * @param i_max  积分上限
 */
void pid_set_integral_limit(pid_t *pid, float i_min, float i_max)
{
    if (pid == NULL) return;
    pid->integral_min = i_min;
    pid->integral_max = i_max;
}

/**
 * @brief 重置 PID 状态(参数保留)
 *        清零误差、积分项与输出
 * @param pid  PID 句柄
 */
void pid_reset(pid_t *pid)
{
    if (pid == NULL) return;
    pid->err        = 0.0f;
    pid->err_last   = 0.0f;
    pid->err_prev   = 0.0f;
    pid->integral   = 0.0f;
    pid->out        = 0.0f;
    pid->feedback   = 0.0f;
}

/**
 * @brief 使能/失能 PID
 *        失能时输出保持上一次值
 * @param pid  PID 句柄
 * @param en   1=使能,0=失能
 */
void pid_enable(pid_t *pid, uint8 en)
{
    if (pid == NULL) return;
    pid->enable = en;
}

/**
 * @brief 执行一次 PID 计算
 *        根据模式自动选择位置式或增量式算法
 *        包含积分限幅与输出限幅
 * @param pid       PID 句柄
 * @param feedback  当前反馈值
 * @retval PID 输出值
 */
float pid_calculate(pid_t *pid, float feedback)
{
    if (pid == NULL || pid->enable == 0) return pid ? pid->out : 0.0f;

    pid->feedback = feedback;
    pid->err      = pid->set - pid->feedback;       /* 当前误差 e_k */

    switch (pid->mode)
    {
        case PID_MODE_POSITION:
        {
            /* 位置式 PID:u_k = Kp*e_k + Ki*Σe + Kd*(e_k - e_{k-1}) */
            pid->integral += pid->err;
            pid->integral  = pid_constrain(pid->integral,
                                           pid->integral_min,
                                           pid->integral_max);   /* 抗积分饱和   */
            pid->out       = pid->kp * pid->err
                           + pid->ki * pid->integral
                           + pid->kd * (pid->err - pid->err_last);
        }
        break;

        case PID_MODE_INCREMENT:
        {
            /* 增量式 PID:Δu = Kp*(e_k - e_{k-1}) + Ki*e_k + Kd*(e_k - 2*e_{k-1} + e_{k-2}) */
            float delta = pid->kp * (pid->err - pid->err_last)
                        + pid->ki *  pid->err
                        + pid->kd * (pid->err - 2.0f * pid->err_last + pid->err_prev);
            pid->out += delta;
        }
        break;

        default:
            break;
    }

    /* 输出限幅 */
    pid->out = pid_constrain(pid->out, pid->out_min, pid->out_max);

    /* 误差历史更新 */
    pid->err_prev  = pid->err_last;
    pid->err_last  = pid->err;

    return pid->out;
}

/* --------------------------------- 使用示例 -------------------------------- */
/*
    // 1. 定义并初始化(以位置式为例,控制电机速度)
    pid_t motor_pid;
    pid_init(&motor_pid, PID_MODE_POSITION, 2.0f, 0.5f, 0.1f);
    pid_set_target(&motor_pid, 1000.0f);          // 目标速度 1000
    pid_set_limit(&motor_pid, -1000.0f, 1000.0f); // 输出限幅 ±1000
    pid_set_integral_limit(&motor_pid, -500.0f, 500.0f);

    // 2. 在控制周期(如 10ms 定时中断)中调用
    float speed = encoder_read_speed();           // 读取反馈
    float u     = pid_calculate(&motor_pid, speed);
    motor_set_pwm((int)u);
*/
