#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */

#if DX_USE_KEY

/**
 * @file    DX_device_key.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   按键设备源文件
 *          基于状态机实现按键扫描、消抖及长按识别
 */

#include "DX_device_key.h"

#define KEY_DEBOUNCE_CNT    3       /* 消抖次数 (3×10ms=30ms)  */
#define KEY_HOLD_CNT        100     /* 长按次数 (100×10ms=1s)  */

/**
 * @brief 按键上下文结构体
 *        保存单个按键的运行时信息
 */
typedef struct
{
    GPIO_TypeDef    *port;      /* GPIO 端口              */
    uint16           pin;       /* GPIO 引脚号            */
    key_state_enum   state;     /* 当前按键状态          */
    uint16           cnt;       /* 消抖/长按计数器        */
    uint8            last;      /* 上次采样电平           */
    uint8            active;    /* 按键是否已绑定使能     */
} key_ctx_t;

static key_ctx_t key_pool[KEY_MAX_COUNT];    /* 按键上下文池 */

/**
 * @brief  绑定按键到指定 GPIO 引脚
 * @param  name : 按键编号,见 key_name_enum
 * @param  port : GPIO 端口 (如 GPIOA、GPIOB 等)
 * @param  pin  : 引脚号 (如 GPIO_PIN_0)
 * @retval 无
 * @note   绑定后默认配置为上拉输入,初始电平视为高
 */
void key_bind (key_name_enum name, GPIO_TypeDef *port, uint16 pin)
{
    if (name >= KEY_MAX_COUNT) return;      /* 越界保护 */

    key_ctx_t *k = &key_pool[name];
    k->port   = port;
    k->pin    = pin;
    k->state  = KEY_NONE;
    k->cnt    = 0;
    k->last   = 1;                          /* 上拉输入空闲态为高 */
    k->active = 1;                          /* 标记按键已启用     */

    DX_GPIO_Init(port, pin, DX_INPUT_PULLUP);   /* 配置为上拉输入 */
}

/**
 * @brief  扫描所有已绑定按键,更新状态
 *         通过连续采样实现消抖,并区分短按与长按
 * @retval 无
 * @note   需在定时周期(10ms)中调用以保证时序准确
 */
void key_scan (void)
{
    for (uint8 i = 0; i < KEY_MAX_COUNT; i++)
    {
        key_ctx_t *k = &key_pool[i];
        if (!k->active) continue;          /* 跳过未启用按键 */

        uint8 cur = DX_GPIO_Read(k->port, k->pin);   /* 读取当前电平 */

        if (cur != k->last)                /* 电平发生变化,重置计数 */
        {
            k->last = cur;
            k->cnt  = 0;
            continue;
        }

        k->cnt++;                          /* 电平稳定,累加计数 */

        if (cur == 0)                      /* 当前为低电平(按下) */
        {
            if (k->cnt == KEY_DEBOUNCE_CNT)
                k->state = KEY_PRESSED;    /* 达到消抖阈值,置按下事件 */
            else if (k->cnt >= KEY_HOLD_CNT)
                k->state = KEY_HOLD;       /* 达到长按阈值,置长按事件 */
        }
        else                              /* 当前为高电平(释放) */
        {
            if (k->cnt == KEY_DEBOUNCE_CNT)
                k->state = KEY_RELEASED;   /* 达到消抖阈值,置释放事件 */
        }
    }
}

/**
 * @brief  获取指定按键的当前状态
 * @param  name : 按键编号,见 key_name_enum
 * @retval 按键状态,见 key_state_enum;读取后状态自动清零为 KEY_NONE
 */
key_state_enum key_get (key_name_enum name)
{
    if (name >= KEY_MAX_COUNT) return KEY_NONE;   /* 越界返回无事件 */

    key_state_enum s = key_pool[name].state;
    key_pool[name].state = KEY_NONE;              /* 读后清零,防止重复触发 */
    return s;
}


void key_test (void)
{
		tim_trigger_start(TIM_2,10,TIM_UNIT_MS);
	
    key_bind(KEY_1, GPIOB, GPIO_PIN_0);

    while (1)
    {       
        if (key_get(KEY_1) == KEY_PRESSED)         /* 短按事件 */
        {
            printf("key_test\r\n");
        }
				
    }
}

#endif /* DX_USE_KEY */
