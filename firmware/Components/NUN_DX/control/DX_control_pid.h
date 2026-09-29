/**
 * @file    DX_control_pid.h
 * @author  YCZ
 * @date    2026-08-02
 * @brief   PID 控制器头文件
 *          提供位置式与增量式 PID 算法接口,支持抗积分饱和与输出限幅
 *          供 DX_device 层(如电机、舵机)调用,实现闭环控制
 */

#ifndef __DX_CONTROL_PID_H
#define __DX_CONTROL_PID_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief PID 工作模式枚举
 *        选择 PID 计算方式
 */
typedef enum
{
    PID_MODE_POSITION = 0,  /* 位置式 PID */
    PID_MODE_INCREMENT,     /* 增量式 PID */
} pid_mode_enum;

/**
 * @brief PID 控制器结构体
 *        保存 PID 参数、运行状态及限幅配置
 */
typedef struct
{
    /* ---- 参数 ---- */
    float kp;                /* 比例系数 Kp            */
    float ki;                /* 积分系数 Ki            */
    float kd;                /* 微分系数 Kd            */

    /* ---- 设定值 ---- */
    float set;               /* 目标值                 */
    float feedback;          /* 实际反馈值             */

    /* ---- 中间变量 ---- */
    float err;               /* 当前误差    (e_k)      */
    float err_last;          /* 上一次误差  (e_{k-1})  */
    float err_prev;          /* 上上次误差  (e_{k-2})  */
    float integral;          /* 积分累加项             */

    /* ---- 输出 ---- */
    float out;               /* PID 计算输出           */
    float out_max;           /* 输出上限               */
    float out_min;           /* 输出下限               */

    /* ---- 抗积分饱和 ---- */
    float integral_max;      /* 积分项上限             */
    float integral_min;      /* 积分项下限             */

    /* ---- 配置 ---- */
    pid_mode_enum mode;      /* 工作模式               */
    uint8  enable;           /* 使能标志(1=运行)     */
} pid_t;

/* Exported functions prototypes ---------------------------------------------*/

void  pid_init       (pid_t *pid, pid_mode_enum mode, float kp, float ki, float kd);   /* PID 初始化                    */
void  pid_set_param  (pid_t *pid, float kp, float ki, float kd);                      /* 设置 Kp/Ki/Kd                 */
void  pid_set_target (pid_t *pid, float target);                                      /* 设置目标值                    */
void  pid_set_limit  (pid_t *pid, float out_min, float out_max);                      /* 设置输出限幅                  */
void  pid_set_integral_limit (pid_t *pid, float i_min, float i_max);                  /* 设置积分限幅(抗饱和)        */
void  pid_reset      (pid_t *pid);                                                    /* 重置 PID 状态                 */
void  pid_enable     (pid_t *pid, uint8 en);                                          /* 使能/失能                     */
float pid_calculate  (pid_t *pid, float feedback);                                    /* 执行一次 PID 计算,返回输出   */

#ifdef __cplusplus
}
#endif

#endif /* __DX_CONTROL_PID_H */
