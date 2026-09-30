#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_AT24C64

/**
 * @file    DX_device_at24c64.c
 * @author  YCZ
 * @date    2026-08-23
 * @brief   AT24C64 EEPROM 设备驱动源文件
 *          基于 I2C 驱动 16 位寻址接口(i2c_dev_mem_write16/read16)实现
 *          8KB EEPROM 读写:多字节写入自动按 32 字节页边界拆分,
 *          每页写完后等待内部写周期
 */

#include "DX_device_at24c64.h"
#include "DX_driver_delay.h"

/* AT24C64 设备句柄:总线、地址、超时一处定义,总线由应用层 i2c_init 初始化 */
static const i2c_dev_t at24c64_dev = { TEMPLATE_DX_I2C_BUS, AT24C64_ADDR, 100 };

/**
 * @brief  地址范围检查
 * @param  addr : 起始地址
 * @param  len  : 数据长度
 * @retval 0: 合法; 1: 越界
 */
static uint8 at24c64_addr_check (uint16 addr, uint16 len)
{
    if ((uint32)addr + len > AT24C64_SIZE) return 1;
    return 0;
}

/**
 * @brief  单页写入(调用方保证不跨页边界)
 * @param  addr : 页内起始地址
 * @param  buf  : 待写入数据缓冲区
 * @param  len  : 写入长度(不超过页剩余空间)
 * @retval 0: 成功; 非0: 失败
 */
static uint8 at24c64_page_write (uint16 addr, const uint8 *buf, uint16 len)
{
    if (i2c_dev_mem_write16(&at24c64_dev, addr, buf, len) != 0) return 1;
    DX_Delay_ms(AT24C64_WRITE_CYCLE_MS);      /* 等待内部写周期完成 */
    return 0;
}

/**
 * @brief  写单个字节
 * @param  addr : 目标地址(0x0000 ~ 0x1FFF)
 * @param  data : 待写入数据
 * @retval 0: 成功; 非0: 失败
 */
uint8 at24c64_write_byte (uint16 addr, uint8 data)
{
    return at24c64_write(addr, &data, 1);
}

/**
 * @brief  读单个字节
 * @param  addr : 目标地址(0x0000 ~ 0x1FFF)
 * @retval 读取的数据(0~255); 失败返回 -1
 */
int16 at24c64_read_byte (uint16 addr)
{
    uint8 data;

    if (at24c64_read(addr, &data, 1) != 0) return -1;
    return (int16)data;
}

/**
 * @brief  多字节写入(自动跨页拆分)
 *         超过页剩余空间时按 32 字节页边界拆分多次页写,
 *         每次页写完成后自动等待内部写周期
 * @param  addr : 起始地址(0x0000 ~ 0x1FFF)
 * @param  buf  : 待写入数据缓冲区
 * @param  len  : 写入长度
 * @retval 0: 成功; 非0: 失败(参数非法、地址越界或 I2C 通信错误)
 */
uint8 at24c64_write (uint16 addr, const uint8 *buf, uint16 len)
{
    if (buf == NULL || len == 0) return 1;
    if (at24c64_addr_check(addr, len)) return 1;

    while (len > 0)
    {
        uint16 page_remain = AT24C64_PAGE_SIZE - (addr % AT24C64_PAGE_SIZE);  /* 当前页剩余空间 */
        uint16 chunk = (len < page_remain) ? len : page_remain;               /* 本次写入长度   */

        if (at24c64_page_write(addr, buf, chunk) != 0) return 1;

        addr += chunk;
        buf  += chunk;
        len  -= chunk;
    }
    return 0;
}

/**
 * @brief  多字节读取
 *         AT24C64 顺序读自动跨页连续,一次读完
 * @param  addr : 起始地址(0x0000 ~ 0x1FFF)
 * @param  buf  : 接收数据缓冲区
 * @param  len  : 读取长度
 * @retval 0: 成功; 非0: 失败(参数非法、地址越界或 I2C 通信错误)
 */
uint8 at24c64_read (uint16 addr, uint8 *buf, uint16 len)
{
    if (buf == NULL || len == 0) return 1;
    if (at24c64_addr_check(addr, len)) return 1;

    return i2c_dev_mem_read16(&at24c64_dev, addr, buf, len);
}

/**
 * @brief  AT24C64 测试函数
 *         初始化 I2C 总线,从 0x001F 写入 70 字节(跨越页 0/1/2 边界,
 *         验证自动拆分),读回后逐字节比对,再验证单字节读写
 * @retval 无
 */
void at24c64_test (void)
{
    uint8 wbuf[70], rbuf[70];
    uint8 i, ok = 1;

    i2c_init(I2C_1, 400000, I2C1_SCL_PB8_SDA_PB9);   /* 总线初始化(应用层负责) */

    for (i = 0; i < 70; i++) wbuf[i] = i * 3;        /* 测试数据:0,3,6,... */

    /* 跨页多字节写读测试:0x001F + 70B 跨越页 0/1/2,验证自动拆分 */
    if (at24c64_write(0x001F, wbuf, 70) != 0)
    {
        printf("AT24C64 write failed\r\n");
        return;
    }

    if (at24c64_read(0x001F, rbuf, 70) != 0)
    {
        printf("AT24C64 read failed\r\n");
        return;
    }

    for (i = 0; i < 70; i++)
    {
        if (rbuf[i] != wbuf[i])
        {
            ok = 0;
            printf("Mismatch @0x%04X: w=%u r=%u\r\n", 0x001F + i, wbuf[i], rbuf[i]);
            break;
        }
    }

    if (ok)
        printf("AT24C64 test pass: 70 bytes @0x001F (cross 3 pages)\r\n");
    else
        printf("AT24C64 test fail\r\n");

    /* 单字节读写测试 */
    if (at24c64_write_byte(0x0000, 0xAA) == 0 && at24c64_read_byte(0x0000) == 0xAA)
        printf("AT24C64 byte rw pass\r\n");
    else
        printf("AT24C64 byte rw fail\r\n");
}

#endif /* DX_USE_AT24C64 */
