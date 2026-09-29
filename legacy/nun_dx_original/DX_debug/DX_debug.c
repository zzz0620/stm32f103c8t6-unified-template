/**
 * @file    DX_debug.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   调试模块源文件
 *          实现基于 UART 的调试输出、断言、日志、DMA 发送及 printf 重定向
 */

#include <stdarg.h>
#include "DX_common_typedef.h"
#include "DX_common_interrupt.h"
#include "DX_debug.h"

static volatile uint8       debug_init_flag = 0;         /* 调试模块初始化完成标志         */
static volatile uint8       debug_assert_enable_flag = 1;/* 断言检查使能标志(默认使能)   */

/**
 * @brief  通过调试串口发送单个字符
 * @param  ch: 待发送字符
 * @retval 无
 */
static void debug_uart_putc (char ch)
{
    uart_write_byte(DEBUG_UART_INDEX, (uint8)ch);
}

/**
 * @brief  通过调试串口发送字符串
 * @param  str: 待发送字符串(以 '\0' 结尾)
 * @retval 无
 */
static void debug_uart_puts (const char *str)
{
    uart_write_string(DEBUG_UART_INDEX, str);
}

/**
 * @brief  软件延时(用于断言失败后的闪烁/等待)
 * @retval 无
 */
static void debug_delay (void)
{
    for (volatile uint32_t i = 0; i < 0xFFFFF; i++)
    {
        __NOP();
    }
}

/**
 * @brief  格式化输出调试信息
 *         先输出类型标签,再输出文件名、行号及附加信息(如有)
 * @param  type: 类型标签字符串(如 "Assert error")
 * @param  file: 源文件名
 * @param  line: 行号
 * @param  str : 附加信息(可为 NULL)
 * @retval 无
 */
static void debug_output (char *type, char *file, int line, char *str)
{
    char output_buffer[256];

    debug_uart_puts(type);

    if (NULL != str)
    {
        sprintf(output_buffer, "\r\nfile: %s, line: %d, info: %s\r\n", file, line, str);
    }
    else
    {
        sprintf(output_buffer, "\r\nfile: %s, line: %d\r\n", file, line);
    }
    debug_uart_puts(output_buffer);
}

/**
 * @brief  阻塞方式发送一段缓冲区
 * @param  buff: 数据缓冲区指针
 * @param  len : 数据长度
 * @retval 固定返回 0
 */
uint32 debug_send_buffer (const uint8 *buff, uint32 len)
{
    uart_write_buffer(DEBUG_UART_INDEX, buff, len);
    return 0;
}

/**
 * @brief  DMA 方式发送一段缓冲区
 * @param  buff: 数据缓冲区指针
 * @param  len : 数据长度
 * @retval 无
 */
void debug_send_dma (const uint8 *buff, uint32 len)
{
    uart_dma_tx_start(DEBUG_UART_INDEX, buff, len);
}

/**
 * @brief  查询 DMA 发送是否忙
 * @retval 1: 正在发送; 0: 空闲
 */
uint8 debug_dma_busy (void)
{
    return uart_dma_tx_busy(DEBUG_UART_INDEX);
}

/**
 * @brief  DMA 方式格式化打印(类似 printf)
 * @param  fmt: 格式字符串
 * @param  ...: 可变参数
 * @retval 无
 * @note   内部使用静态缓冲区,非线程安全;依赖 DMA 发送
 */
void debug_printf_dma (const char *fmt, ...)
{
    static char buf[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    uart_dma_tx_start(DEBUG_UART_INDEX, (uint8 *)buf, strlen(buf));
}

#if DEBUG_UART_USE_INTERRUPT

/**
 * @brief  从环形缓冲区读取数据(中断模式下使用)
 * @param  buff: 读取数据存放缓冲区
 * @param  len : 期望读取的最大长度
 * @retval 实际读取的字节数
 */
uint32 debug_read_ring_buffer (uint8 *buff, uint32 len)
{
    uint32 i;
    for (i = 0; i < len; i++)
    {
        if (uart_query_byte(DEBUG_UART_INDEX, &buff[i]) == 0)
            break;
    }
    return i;
}

/**
 * @brief  调试串口中断处理函数(中断模式下由 ISR 调用)
 * @retval 无
 */
void debug_interrupt_handler (void)
{
    uart_irq_handler(DEBUG_UART_INDEX);
}

#endif

/* 非微库模式下需要重定义标准库的 FILE 结构及 _sys_exit,以支持 printf 重定向 */
#if !defined(__MICROLIB)
#if (__ARMCLIB_VERSION <= 6000000)
struct __FILE
{
    int handle;
};
#endif

FILE __stdout;

/**
 * @brief  标准库退出函数(重定向用,空实现)
 * @param  x: 退出码
 * @retval 无
 */
void _sys_exit(int x)
{
    x = x;
}
#endif

/**
 * @brief  printf 字符输出重定向
 *         调试模块完成初始化后,printf 输出将经调试串口发送
 * @param  ch: 待输出字符
 * @param  f : 文件指针(标准库约定,未使用)
 * @retval 输出的字符
 */
int fputc (int ch, FILE *f)
{
    if (debug_init_flag)
    {
        debug_uart_putc((char)(ch & 0xFF));
    }
    return ch;
}

/**
 * @brief  scanf 字符输入重定向
 *         调试模块完成初始化后,从调试串口读取输入
 * @param  f: 文件指针(标准库约定,未使用)
 * @retval 读取到的字节
 */
int fgetc (FILE *f)
{
    uint8 data = 0;
    if (debug_init_flag)
    {
        uart_read_byte(DEBUG_UART_INDEX, &data);
    }
    return data;
}

/**
 * @brief  使能断言检查
 * @retval 无
 */
void debug_assert_enable (void)
{
    debug_assert_enable_flag = 1;
}

/**
 * @brief  禁用断言检查
 * @retval 无
 */
void debug_assert_disable (void)
{
    debug_assert_enable_flag = 0;
}

/**
 * @brief  断言处理函数
 *         当 pass 为假(0)且断言已使能时,关闭全局中断并输出断言信息后死循环
 * @param  pass: 断言条件(0 表示失败)
 * @param  file: 源文件名
 * @param  line: 行号
 * @retval 无
 * @note   通过 assert_nest_index 防止断言嵌套进入二次断言
 */
void debug_assert_handler (uint8 pass, char *file, int line)
{
    do
    {
        if (pass || !debug_assert_enable_flag)   /* 条件成立或断言已禁用,直接退出 */
        {
            break;
        }

        static uint8 assert_nest_index = 0;      /* 断言嵌套计数,防止二次进入     */

        if (0 != assert_nest_index)
        {
            while (1);
        }
        assert_nest_index++;

        interrupt_global_disable();              /* 关闭全局中断                   */

        debug_output("Assert error", file, line, NULL);  /* 输出断言错误信息       */

        while (1)
        {
            debug_delay();
        }
    } while (0);
}

/**
 * @brief  日志处理函数
 *         当 pass 为假(0)时输出一条日志信息(不会死循环)
 * @param  pass: 条件(0 表示触发日志)
 * @param  str : 日志内容字符串
 * @param  file: 源文件名
 * @param  line: 行号
 * @retval 无
 */
void debug_log_handler (uint8 pass, char *str, char *file, int line)
{
    do
    {
        if (pass)
        {
            break;
        }
        if (debug_init_flag)
        {
            debug_output("Log message", file, line, str);
        }
    } while (0);
}

/**
 * @brief  调试模块初始化
 *         初始化调试串口,并根据配置使能接收/空闲中断
 * @retval 无
 */
void debug_init (void)
{
    uart_init(DEBUG_UART_INDEX, DEBUG_UART_BAUDRATE, DEBUG_UART_PIN);

#if DEBUG_UART_USE_INTERRUPT
    uart_set_interrupt_config(DEBUG_UART_INDEX, UART_INTERRUPT_RX | UART_INTERRUPT_IDLE);
#endif

    debug_init_flag = 1;                         /* 标记初始化完成,启用 printf 重定向 */
}



/**
 * @brief  调试测试函数
 *         通过 printf 发送字符、整数、浮点数、十六进制及字符串,验证调试串口
 *         及 printf 重定向是否正常工作
 * @retval 无
 */
void debug_test (void)
{
    uint8 i;

    printf("===== Debug Test =====\r\n");
    printf("Char : %c%c%c\r\n", 'D', 'X', '!');      /* 字符输出测试            */
    printf("Int  : %d\r\n", 1234);                    /* 整数输出测试            */
    printf("Float: %.2f\r\n", 3.14f);                 /* 浮点数输出测试          */
    printf("Hex  : 0x%02X\r\n", 0xAB);                /* 十六进制输出测试        */
    printf("Str  : %s\r\n", "STM32");                 /* 字符串输出测试          */

    for (i = 0; i < 5; i++)                          /* 循环计数输出测试        */
    {
        printf("Count: %d\r\n", i);
    }

    printf("===== Test End =====\r\n");
}

/**
 * @brief  断言功能测试函数
 *         演示 DX_assert 宏在防御性编程中的优势:对参数合法性、指针非空、
 *         计算结果等进行即时检查,任一条件不满足时立即输出文件名与行号
 *         并停机,避免错误继续向后传播导致难以定位的后继故障
 * @param  trigger_fail: 0-仅演示正常通过路径;
 *                       1-故意触发一次断言失败(会输出断言信息并进入死循环,
 *                         用于观察失败时的输出格式)
 * @retval 无
 * @note   trigger_fail 为 1 时函数不会返回,请确保已连接调试串口观察输出;
 *         可通过 debug_assert_disable() 临时关闭断言以验证"禁用后不触发"的效果
 */
void debug_assert_test (uint8 trigger_fail)
{
    int8  index = 3;
    int8  result;
    uint8 buffer[8] = {0};
    uint8 *ptr = buffer;

    /* 场景 1:参数/索引范围检查,确保 index 在 buffer 有效范围内 */
    DX_assert(index >= 0 && index < 8);

    /* 场景 2:指针非空检查,确保资源已正确分配 */
    DX_assert(ptr != NULL);

    /* 场景 3:计算结果合法性检查,确保运算结果在预期范围内 */
    result = index + 1;
    DX_assert(result <= 8);

    /* 正常路径:所有断言均通过,可安全使用资源 */
    buffer[index] = result;
    printf("Assert pass: buffer[%d] = %d\r\n", index, result);

    /* 场景 4:故意触发断言失败,演示错误捕获效果(失败时输出文件名+行号并死循环) */
    if (trigger_fail)
    {
        printf("Will trigger assert fail...\r\n");
        index = 100;                                       /* 越界值                */
        DX_assert(index >= 0 && index < 8);                /* 此处将失败并死循环     */
    }
}

/**
 * @brief  日志功能测试函数
 *         演示 DX_log 宏在调试追踪中的优势:条件不满足时自动输出文件名、行号
 *         及自定义信息,但不会中断程序运行(与 DX_assert 的关键区别),适合
 *         记录"软错误"或非致命异常,便于事后定位而不影响主流程
 * @retval 无
 * @note   与 DX_assert 对比:
 *         - DX_assert:失败即停机,用于致命错误;
 *         - DX_log   :失败仅记录,用于非致命异常或调试追踪
 */
void debug_log_test (void)
{
   
    uint8_t  index;

    printf("===== DX_log Test =====\r\n");

	/* 场景 1:条件不成立输出日志,避免噪音 */
	  index = 3;
	  DX_log(index >= 8, "index >= 8, log triggered");       /* 条件不成立(index=3),日志输出 */

    printf("===== Log Test End =====\r\n");
	
}
