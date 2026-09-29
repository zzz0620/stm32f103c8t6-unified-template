#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_MPU6050

/**
 * @file    DX_device_i2c_mpu6050.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   MPU6050 六轴传感器驱动源文件
 *          基于 I2C 实现 MPU6050 的初始化、寄存器读写、陀螺仪/加速度计/
 *          温度数据读取及量程转换等功能
 */

#include "DX_device_i2c_mpu6050.h"
#include "DX_driver_i2c.h"
#include "DX_driver_delay.h"

#define MPU6050_REG_WHO_AM_I        0x75    /* WHO_AM_I 寄存器,存放设备 ID            */
#define MPU6050_REG_PWR_MGMT_1      0x6B    /* 电源管理寄存器 1,控制睡眠/时钟源       */
#define MPU6050_REG_SMPLRT_DIV      0x19    /* 采样率分频寄存器                        */
#define MPU6050_REG_CONFIG          0x1A    /* 配置寄存器,设置低通滤波等              */
#define MPU6050_REG_GYRO_CONFIG     0x1B    /* 陀螺仪配置寄存器,设置量程              */
#define MPU6050_REG_ACCEL_CONFIG    0x1C    /* 加速度计配置寄存器,设置量程            */
#define MPU6050_REG_ACCEL_XOUT_H    0x3B    /* 加速度计 X 轴输出高字节(起始)          */
#define MPU6050_REG_TEMP_OUT_H      0x41    /* 温度输出高字节(起始)                   */
#define MPU6050_REG_GYRO_XOUT_H     0x43    /* 陀螺仪 X 轴输出高字节(起始)            */
#define MPU6050_REG_SIGNAL_PATH     0x68    /* 信号路径复位寄存器                      */

#define MPU6050_GYRO_CONFIG_VAL     0x00    /* 陀螺仪量程 ±250°/s                      */
#define MPU6050_ACCEL_CONFIG_VAL    0x00    /* 加速度计量程 ±2g                        */
#define MPU6050_GYRO_SCALE          131.0f  /* 陀螺仪灵敏度 LSB/(°/s)                  */
#define MPU6050_ACCEL_SCALE         16384.0f/* 加速度计灵敏度 LSB/g                    */

/* MPU6050 设备句柄:总线由 init 配置,地址与超时在此定义 */
static i2c_dev_t mpu6050_dev = { I2C_1, MPU6050_ADDR, 100 };

/**
 * @brief  向 MPU6050 指定寄存器写入一个字节
 * @param  reg: 寄存器地址
 * @param  val: 待写入的字节值
 * @retval 无
 */
static void mpu6050_write_reg (uint8 reg, uint8 val)
{
    i2c_dev_mem_write(&mpu6050_dev, reg, &val, 1);
}

/**
 * @brief  从 MPU6050 指定寄存器读取一个字节
 * @param  reg: 寄存器地址
 * @retval 读取到的字节值
 */
static uint8 mpu6050_read_reg (uint8 reg)
{
    uint8 val = 0;
    i2c_dev_mem_read(&mpu6050_dev, reg, &val, 1);
    return val;
}

/**
 * @brief  从 MPU6050 指定寄存器连续读取多个字节
 * @param  reg: 起始寄存器地址
 * @param  buf: 数据存储缓冲区指针
 * @param  len: 读取的字节数
 * @retval 无
 */
static void mpu6050_read_buf (uint8 reg, uint8 *buf, uint8 len)
{
    i2c_dev_mem_read(&mpu6050_dev, reg, buf, len);
}

/**
 * @brief  MPU6050 初始化
 *         选择 I2C 总线并初始化各寄存器,唤醒芯片并设置采样率、滤波、量程等
 * @param  i2c_idx: 使用的 I2C 端口编号,见 i2c_index_enum
 *                  (总线本身由应用层 i2c_init 初始化)
 * @retval 无
 */
void mpu6050_init (i2c_index_enum i2c_idx)
{
    mpu6050_dev.bus = i2c_idx;

    mpu6050_write_reg(MPU6050_REG_PWR_MGMT_1,  0x00);/* 唤醒芯片,关闭睡眠模式     */
    mpu6050_write_reg(MPU6050_REG_SMPLRT_DIV,  0x07);/* 采样率分频 1+7=8          */
    mpu6050_write_reg(MPU6050_REG_CONFIG,       0x06);/* 低通滤波带宽 5Hz          */
    mpu6050_write_reg(MPU6050_REG_GYRO_CONFIG,  MPU6050_GYRO_CONFIG_VAL);  /* 陀螺仪量程 ±250°/s  */
    mpu6050_write_reg(MPU6050_REG_ACCEL_CONFIG, MPU6050_ACCEL_CONFIG_VAL); /* 加速度计量程 ±2g    */
    mpu6050_write_reg(MPU6050_REG_SIGNAL_PATH,  0x00);/* 信号路径复位              */
}

/**
 * @brief  读取 MPU6050 设备 ID
 * @retval WHO_AM_I 寄存器值(应为 0x68)
 */
uint8 mpu6050_get_id (void)
{
    return mpu6050_read_reg(MPU6050_REG_WHO_AM_I);
}

/**
 * @brief  读取陀螺仪三轴原始数据
 * @param  data: 指向存放三轴原始数据的结构体指针
 * @retval 无
 * @note   连续读取 6 字节,依次为 X/Y/Z 轴的高低字节
 */
void mpu6050_read_gyro (mpu6050_raw_data_t *data)
{
    uint8 buf[6];
    mpu6050_read_buf(MPU6050_REG_GYRO_XOUT_H, buf, 6);
    data->x = ((int16)buf[0] << 8) | buf[1];          /* X 轴原始值(高字节在前)   */
    data->y = ((int16)buf[2] << 8) | buf[3];          /* Y 轴原始值                */
    data->z = ((int16)buf[4] << 8) | buf[5];          /* Z 轴原始值                */
}

/**
 * @brief  读取加速度计三轴原始数据
 * @param  data: 指向存放三轴原始数据的结构体指针
 * @retval 无
 * @note   连续读取 6 字节,依次为 X/Y/Z 轴的高低字节
 */
void mpu6050_read_accel (mpu6050_raw_data_t *data)
{
    uint8 buf[6];
    mpu6050_read_buf(MPU6050_REG_ACCEL_XOUT_H, buf, 6);
    data->x = ((int16)buf[0] << 8) | buf[1];          /* X 轴原始值(高字节在前)   */
    data->y = ((int16)buf[2] << 8) | buf[3];          /* Y 轴原始值                */
    data->z = ((int16)buf[4] << 8) | buf[5];          /* Z 轴原始值                */
}

/**
 * @brief  读取温度原始数据
 * @retval 温度原始值(16 位有符号,高字节在前)
 * @note   实际温度换算:temp = raw/340 + 36.53
 */
int16 mpu6050_read_temp (void)
{
    uint8 buf[2];
    mpu6050_read_buf(MPU6050_REG_TEMP_OUT_H, buf, 2);
    return ((int16)buf[0] << 8) | buf[1];             /* 高字节在前拼接            */
}

/**
 * @brief  将陀螺仪原始值转换为角速度(°/s)
 * @param  raw: 陀螺仪原始值
 * @retval 对应角速度值,单位 °/s
 * @note   量程 ±250°/s 时灵敏度为 131 LSB/(°/s)
 */
float mpu6050_gyro_dps (int16 raw)
{
    return (float)raw / MPU6050_GYRO_SCALE;
}

/**
 * @brief  将加速度计原始值转换为重力加速度(g)
 * @param  raw: 加速度计原始值
 * @retval 对应加速度值,单位 g
 * @note   量程 ±2g 时灵敏度为 16384 LSB/g
 */
float mpu6050_accel_g (int16 raw)
{
    return (float)raw / MPU6050_ACCEL_SCALE;
}

/**
 * @brief  MPU6050 测试函数
 *         初始化后循环读取陀螺仪与加速度计数据并经串口打印,周期 500ms
 * @retval 无
 */
void mpu6050_test (void)
{
    mpu6050_raw_data_t gyro, accel;

    i2c_init(I2C_1, 400000, I2C1_SCL_PB8_SDA_PB9);    /* 总线初始化(应用层负责)    */
    mpu6050_init(I2C_1);                              /* 使用 I2C1                 */

    while (1)
    {
        mpu6050_read_gyro(&gyro);                      /* 读取陀螺仪数据            */
        mpu6050_read_accel(&accel);                    /* 读取加速度计数据          */

        printf("G:X=%.2f Y=%.2f Z=%.2f  A:X=%.2f Y=%.2f Z=%.2f\r\n",
               mpu6050_gyro_dps(gyro.x),
               mpu6050_gyro_dps(gyro.y),
               mpu6050_gyro_dps(gyro.z),
               mpu6050_accel_g(accel.x),
               mpu6050_accel_g(accel.y),
               mpu6050_accel_g(accel.z));

        DX_Delay_ms(500);                              /* 延时 500ms                */
    }
}

#endif /* DX_USE_MPU6050 */
