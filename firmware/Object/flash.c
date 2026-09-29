//#include "flash.h"
//#include "stm32f4xx_hal_flash.h"

// uint32_t data;
//uint32_t rawData_1[4] ;
//uint8_t output_test[4];
///**
//   * @brief  根据输入的地址给出它所在的sector
//   *         例如：
//            uwStartSector = GetSector(FLASH_USER_START_ADDR);
//            uwEndSector = GetSector(FLASH_USER_END_ADDR);
//   * @param  Address：地址
//   * @retval 地址所在的sector
//   */
//	 
//static uint32_t GetSector(uint32_t Address)
//{
//   uint32_t sector = 0;

//if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
//         sector = FLASH_SECTOR_0;
//} else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1)) {
//         sector = FLASH_SECTOR_1;
//   } else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2)) {
//         sector = FLASH_SECTOR_2;
//   } else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3)) {
//         sector = FLASH_SECTOR_3;
//   } else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4)) {
//      sector = FLASH_SECTOR_4;
//   } else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5)) {
//      sector = FLASH_SECTOR_5;
//   } else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6)) {
//      sector = FLASH_SECTOR_6;
//   } else { /*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/
//      sector = FLASH_SECTOR_7;
//   }
//   return sector;
//}


///**
// * @brief 写入float数组到Flash
// * @param data_array：要写入的float数组（长度4）
// * @retval 0成功，-1失败
// */
//uint32_t Address_1;
//int Flash_WriteArray(float data_array[4])
//{
//    /* 解锁并擦除 */
//    HAL_FLASH_Unlock();
//	
//    /* 擦除配置 */
//    uint32_t FirstSector = GetSector(FLASH_USER_START_ADDR);
//    uint32_t NbOfSectors = GetSector(FLASH_USER_END_ADDR) - FirstSector + 1;
//    FLASH_EraseInitTypeDef EraseInitStruct = 
//	 {
//        .TypeErase = FLASH_TYPEERASE_SECTORS,
//        .VoltageRange = FLASH_VOLTAGE_RANGE_3,
//        .Sector = FirstSector,
//        .NbSectors = NbOfSectors
//    };
//    uint32_t SECTORError = 0;


//    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) 
//		{
//        return -1;
//    }

//    /* 写入数组 */
//    
//		uint32_t Address=FLASH_USER_START_ADDR;
//    for (int i = 0; i < 4; i++) 
//		{
//         rawData_1[i] = *(uint32_t*)&data_array[i];
//        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, rawData_1[i]) == HAL_OK) 
//				{
//					Address += 4;
//        }
//				else
//				  return -1;
//        
//    }
//    for (int i = 0; i < 4; i++) 
//		{
//        float read_back = *(__IO float*)(FLASH_USER_START_ADDR + i*4);
//        if (read_back != data_array[i]) {
//            HAL_FLASH_Lock();
//            return -2; // 校验失败
//        }
//    }
//    HAL_FLASH_Lock();
//    return 0;
//}

///**
// * @brief 从Flash读取float数组
// * @param output_array：存储读取结果的数组（长度4）
// */
//void Flash_ReadArray(float output_array[4])
//{
//    uint32_t Address = FLASH_USER_START_ADDR;
//	  Address_1 = *(uint32_t*)(FLASH_USER_START_ADDR+4);
//    for (int i = 0; i < 4; i++) 
//	  {
//        uint32_t addr = FLASH_ARRAY_START_ADDR + (i * 4);
//        if (*(uint32_t*)addr == 0xFFFFFFFF) 
//				{ // 首次运行，写入默认值
//					float defaults[4] = {28.5f, 30.0f, 28.0f, 24.0f};
//            Flash_WriteArray(defaults);			
//				}
//				else
//				{
//					uint32_t rawData = *(__IO uint32_t*)Address;
//					output_array[i] = *(float*)&rawData;
//					output_test[i]= *(float*)&rawData;
//					Address += 4;				
//				}

//    }
//}

////int InternalFlash_Test(float data)
////{
////   /*要擦除的起始扇区(包含)及结束扇区(不包含)，如8-12，表示擦除8、9、10、11扇区*/
////   uint32_t FirstSector = 0;
////   uint32_t NbOfSectors = 0;

////   uint32_t SECTORError = 0;

////   uint32_t Address = 0;

////   __IO uint32_t Data32 = 0;
////   __IO uint32_t MemoryProgramStatus = 0;
////    FLASH_EraseInitTypeDef EraseInitStruct;

////   /* FLASH 解锁 ********************************/
////   /* 使能访问FLASH控制寄存器 */
////   HAL_FLASH_Unlock();

////   FirstSector = GetSector(FLASH_USER_START_ADDR);
////   NbOfSectors = GetSector(FLASH_USER_END_ADDR)- FirstSector + 1;

////   /* 擦除用户区域 (用户区域指程序本身没有使用的空间，可以自定义)**/
////   /* Fill EraseInit structure*/
////   EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
////   EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;/* 以“字”的大小进行操作 */
////   EraseInitStruct.Sector        = FirstSector;
////   EraseInitStruct.NbSectors     = NbOfSectors;
////   /* 开始擦除操作 */
////   if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
////      /*擦除出错，返回，实际应用中可加入处理 */
////      return -1;
////   }

////   /* 以“字”的大小为单位写入数据 ********************************/
////   Address = FLASH_USER_START_ADDR;

////   while (Address < FLASH_USER_END_ADDR) {
////      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, data) == HAL_OK) {
////            Address = Address + 4;
////      } else {
////            /*写入出错，返回，实际应用中可加入处理 */
////            return -1;
////      }
////   }

////   /* 给FLASH上锁，防止内容被篡改*/
////   HAL_FLASH_Lock();

////   /* 从FLASH中读取出数据进行校验***************************************/
////   /*  MemoryProgramStatus = 0: 写入的数据正确
////      MemoryProgramStatus != 0: 写入的数据错误，其值为错误的个数 */
////   Address = FLASH_USER_START_ADDR;
////   MemoryProgramStatus = 0;

////   while (Address < FLASH_USER_END_ADDR) {
////      data = *(__IO uint32_t*)Address;

////      if (data != data) {
////            MemoryProgramStatus++;
////      }
////         Address = Address + 4;
////   }
////   /* 数据校验不正确 */
////   if (MemoryProgramStatus) {
////         return -1;
////   } else { /*数据校验正确*/
////         return 0;
////   }
////}
