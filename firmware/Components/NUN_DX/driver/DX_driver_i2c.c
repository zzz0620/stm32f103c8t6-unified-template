/**
 * @file    DX_driver_i2c.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   I2C 驱动源文件
 *          基于 HAL 库封装 STM32 硬件 I2C:
 *          总线级 i2c_init 由应用层负责初始化,设备级 i2c_dev_* 通过
 *          设备句柄绑定总线、地址与超时,便于同一总线上挂载多个设备
 */

#include "DX_driver_i2c.h"

/**
 * @brief I2C 引脚配置结构体
 *        保存某组 I2C 引脚的端口、引脚号及 AFIO 重映射信息
 */
typedef struct
{
    GPIO_TypeDef *port;     /* GPIO 端口                      */
    uint16 scl_pin;        /* SCL 引脚位掩码                 */
    uint16 sda_pin;        /* SDA 引脚位掩码                 */
    uint32 remap_val;      /* AFIO 重映射写入值              */
    uint32 remap_mask;     /* AFIO 重映射掩码                */
} i2c_pin_config_t;

/* I2C 引脚配置表,按 i2c_pin_enum 索引 */
static const i2c_pin_config_t i2c_pin_table[] =
{
    [I2C1_SCL_PB6_SDA_PB7]   = { GPIOB, GPIO_PIN_6,  GPIO_PIN_7,  0x00000000, AFIO_MAPR_I2C1_REMAP_Msk },
    [I2C1_SCL_PB8_SDA_PB9]   = { GPIOB, GPIO_PIN_8,  GPIO_PIN_9,  AFIO_MAPR_I2C1_REMAP_Msk, AFIO_MAPR_I2C1_REMAP_Msk },
    [I2C2_SCL_PB10_SDA_PB11] = { GPIOB, GPIO_PIN_10, GPIO_PIN_11, 0x00000000, 0 },
};

/**
 * @brief I2C 上下文结构体
 *        保存 HAL 句柄及初始化完成标志
 */
typedef struct
{
    I2C_HandleTypeDef owned_handle; /* 独立模式自有 HAL 句柄     */
    I2C_HandleTypeDef *handle;      /* 当前使用的 HAL 句柄       */
    uint8             init_flag;/* 初始化完成标志              */
} i2c_ctx_t;

static i2c_ctx_t i2c_ctx[I2C_COUNT];  /* 各 I2C 实例上下文数组 */

/* I2C 实例基址表,按 i2c_index_enum 索引 */
static I2C_TypeDef *const i2c_instance[] =
{
    I2C1, I2C2
};

/**
 * @brief  I2C 初始化
 *         配置引脚复用、AFIO 重映射,并按指定速率初始化 HAL I2C 句柄
 * @param  idx   : I2C 实例索引,见 i2c_index_enum
 * @param  speed : I2C 时钟频率 (单位 Hz)
 * @param  pin   : I2C 引脚组合,见 i2c_pin_enum
 * @retval 无
 */
 void i2c_init (i2c_index_enum idx, uint32 speed, i2c_pin_enum pin)
{
    if (idx >= I2C_COUNT) return;

    i2c_ctx_t *ctx = &i2c_ctx[idx];
    ctx->init_flag = 0;

    const i2c_pin_config_t *pincfg = &i2c_pin_table[pin];

    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    MODIFY_REG(AFIO->MAPR, pincfg->remap_mask, pincfg->remap_val);

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = pincfg->scl_pin | pincfg->sda_pin;
    gpio.Mode  = GPIO_MODE_AF_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(pincfg->port, &gpio);

    if (idx == I2C_1) __HAL_RCC_I2C1_CLK_ENABLE();
    if (idx == I2C_2) __HAL_RCC_I2C2_CLK_ENABLE();

    I2C_HandleTypeDef *hi2c = &ctx->owned_handle;
    hi2c->Instance             = i2c_instance[idx];
    hi2c->Init.ClockSpeed      = speed;
    hi2c->Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c->Init.OwnAddress1     = 0;
    hi2c->Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c->Init.OwnAddress2     = 0;
    hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c->Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(hi2c) != HAL_OK) return;

    ctx->handle = hi2c;
    ctx->init_flag = 1;
}

void i2c_attach (i2c_index_enum idx, I2C_HandleTypeDef *handle)
{
    if ((idx >= I2C_COUNT) || (handle == NULL)) return;
    i2c_ctx[idx].handle = handle;
    i2c_ctx[idx].init_flag = 1;
}

/**
 * @brief  获取已初始化的 I2C 上下文
 * @param  idx : I2C 实例索引
 * @retval 上下文指针; 索引越界或未初始化时返回 NULL
 */
static i2c_ctx_t *i2c_get_ctx (i2c_index_enum idx)
{
    if (idx >= I2C_COUNT) return NULL;
    i2c_ctx_t *ctx = &i2c_ctx[idx];
    if (!ctx->init_flag) return NULL;
    return ctx;
}

/**
 * @brief  I2C 向设备写入数据(无寄存器地址)
 *        用于 BH1750/AHT10 等无寄存器寻址协议的命令写入
 * @param  dev : 设备句柄(总线、地址、超时)
 * @param  buf : 待发送数据缓冲区
 * @param  len : 发送数据长度
 * @retval 0: 成功; 非0: 失败(HAL 状态码)
 */
uint8 i2c_dev_write (const i2c_dev_t *dev, const uint8 *buf, uint16 len)
{
    if (dev == NULL) return 1;
    i2c_ctx_t *ctx = i2c_get_ctx(dev->bus);
    if (ctx == NULL) return 1;
    return HAL_I2C_Master_Transmit(ctx->handle, dev->addr << 1, (uint8 *)buf, len, dev->timeout);
}

/**
 * @brief  I2C 从设备读取数据(无寄存器地址)
 *        用于 BH1750/AHT10 等无寄存器寻址协议的数据读取
 * @param  dev : 设备句柄(总线、地址、超时)
 * @param  buf : 接收数据缓冲区
 * @param  len : 读取数据长度
 * @retval 0: 成功; 非0: 失败(HAL 状态码)
 */
uint8 i2c_dev_read (const i2c_dev_t *dev, uint8 *buf, uint16 len)
{
    if (dev == NULL) return 1;
    i2c_ctx_t *ctx = i2c_get_ctx(dev->bus);
    if (ctx == NULL) return 1;
    return HAL_I2C_Master_Receive(ctx->handle, dev->addr << 1, buf, len, dev->timeout);
}

/**
 * @brief  I2C 向设备寄存器写入数据
 * @param  dev : 设备句柄(总线、地址、超时)
 * @param  reg : 设备寄存器地址
 * @param  buf : 待写入数据缓冲区
 * @param  len : 写入数据长度
 * @retval 0: 成功; 非0: 失败(HAL 状态码)
 */
uint8 i2c_dev_mem_write (const i2c_dev_t *dev, uint8 reg, const uint8 *buf, uint16 len)
{
    if (dev == NULL) return 1;
    i2c_ctx_t *ctx = i2c_get_ctx(dev->bus);
    if (ctx == NULL) return 1;
    return HAL_I2C_Mem_Write(ctx->handle, dev->addr << 1, reg, I2C_MEMADD_SIZE_8BIT, (uint8 *)buf, len, dev->timeout);
}

/**
 * @brief  I2C 从设备寄存器读取数据
 * @param  dev : 设备句柄(总线、地址、超时)
 * @param  reg : 设备寄存器地址
 * @param  buf : 接收数据缓冲区
 * @param  len : 读取数据长度
 * @retval 0: 成功; 非0: 失败(HAL 状态码)
 */
uint8 i2c_dev_mem_read (const i2c_dev_t *dev, uint8 reg, uint8 *buf, uint16 len)
{
    if (dev == NULL) return 1;
    i2c_ctx_t *ctx = i2c_get_ctx(dev->bus);
    if (ctx == NULL) return 1;
    return HAL_I2C_Mem_Read(ctx->handle, dev->addr << 1, reg, I2C_MEMADD_SIZE_8BIT, buf, len, dev->timeout);
}

/**
 * @brief  I2C 向设备 16 位寄存器地址写入数据
 *        用于 AT24C64 等 16 位字地址寻址的 EEPROM 写入
 * @param  dev : 设备句柄(总线、地址、超时)
 * @param  reg : 设备 16 位寄存器(字)地址
 * @param  buf : 待写入数据缓冲区
 * @param  len : 写入数据长度
 * @retval 0: 成功; 非0: 失败(HAL 状态码)
 */
uint8 i2c_dev_mem_write16 (const i2c_dev_t *dev, uint16 reg, const uint8 *buf, uint16 len)
{
    if (dev == NULL) return 1;
    i2c_ctx_t *ctx = i2c_get_ctx(dev->bus);
    if (ctx == NULL) return 1;
    return HAL_I2C_Mem_Write(ctx->handle, dev->addr << 1, reg, I2C_MEMADD_SIZE_16BIT, (uint8 *)buf, len, dev->timeout);
}

/**
 * @brief  I2C 从设备 16 位寄存器地址读取数据
 *        用于 AT24C64 等 16 位字地址寻址的 EEPROM 读取
 * @param  dev : 设备句柄(总线、地址、超时)
 * @param  reg : 设备 16 位寄存器(字)地址
 * @param  buf : 接收数据缓冲区
 * @param  len : 读取数据长度
 * @retval 0: 成功; 非0: 失败(HAL 状态码)
 */
uint8 i2c_dev_mem_read16 (const i2c_dev_t *dev, uint16 reg, uint8 *buf, uint16 len)
{
    if (dev == NULL) return 1;
    i2c_ctx_t *ctx = i2c_get_ctx(dev->bus);
    if (ctx == NULL) return 1;
    return HAL_I2C_Mem_Read(ctx->handle, dev->addr << 1, reg, I2C_MEMADD_SIZE_16BIT, buf, len, dev->timeout);
}
