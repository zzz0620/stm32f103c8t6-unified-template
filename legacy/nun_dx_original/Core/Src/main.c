/**
 * @file    main.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   主程序入口
 *          完成系统时钟配置、外设初始化,并进入主循环
 */

#include "main.h"
#include "DX_common_headfile.h"


void SystemClock_Config(void);
int main(void)
{

  HAL_Init();                  /* HAL 库初始化(配置 SysTick 等)            */

  SystemClock_Config();        /* 系统时钟配置(72MHz)                      */
	
  debug_init();                /* 调试串口初始化                            */
	
  printf("System ready\r\n");  /* 打印系统就绪信息                          */

	
//	DX_GPIO_Test(); /* GPIO 测试函数       */
// 	adc_test ();	/* 读取 ADC 通道测试函数           */	
// tim_trigger_test ();  /* 定时器触发测试        */	
	key_test();				/* 按键触发触发测试        */	
	
	

  while (1)
  {

		
  }

}


































void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


  /* 振荡器配置:使用 HSE 外部晶振,PLL 倍频 9 倍 */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }


  /* 时钟源配置:SYSCLK 取自 PLL,AHB 不分频,APB1 二分频,APB2 不分频 */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}



/**
 * @brief  错误处理函数
 *         在 HAL 库初始化失败时调用,关闭全局中断后死循环
 * @retval 无
 */
void Error_Handler(void)
{

  __disable_irq();             /* 关闭全局中断 */
  while (1)
  {
  }

}




#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
