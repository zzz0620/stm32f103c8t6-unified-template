//#ifndef __FLASH_H
//#define __FLASH_H
//#include "stdint.h"
//#include "stm32f4xx_hal.h"  


//#define FLASH_ARRAY_START_ADDR  ADDR_FLASH_SECTOR_2  // 数组存储的起始地址

//#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_5
///* 要擦除内部FLASH的结束地址 */
//#define FLASH_USER_END_ADDR     ADDR_FLASH_SECTOR_7


///* Base address of the Flash sectors */
//#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* 32 Kbytes */
//#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08008000) /* 32 Kbytes */
//#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08010000) /* 32 Kbytes */
//#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x08018000) /* 32 Kbytes */
//#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08020000) /* 128 Kbytes */
//#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08040000) /* 256 Kbytes */
//#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08080000) /* 256 Kbytes */
//#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x080C0000) /* 256 Kbytes */

//static uint32_t GetSector(uint32_t Address);
////int InternalFlash_Test(float data);
//void Flash_ReadArray(float output_array[4]);
//int Flash_WriteArray(float data_array[4]);
//#endif