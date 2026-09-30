#ifndef __OLED_H_
#define __OLED_H_

#include "stdint.h"

#define OLED_IIC_ADDR  0x7a

#define Brightness 0x6f
#define OLED_13 0
#define Max_Column 128
#define Max_Row 64

extern uint8_t OLED_GRAM[128][8];
void OLED_Refresh(void);
void OLED_UpdateColumn20(void);  // 建议改名，明确功能
void OLED_WrDat(uint8_t IIC_Data);
void OLED_WrCmd(uint8_t IIC_Command);
void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_Fill(uint8_t bmp_dat);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x, uint8_t y);
void OLED_ClearPoint(uint8_t x, uint8_t y);
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void OLED_Init(void);
extern uint32_t OLED_pow(uint8_t m, uint8_t n);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size2);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t mode, uint8_t SIZE);
void OLED_ShowStr(uint16_t x, uint16_t y, uint8_t *str, uint8_t size1, uint8_t mode);
void OLED_ShowStr_Center(uint16_t x, uint16_t y, uint8_t *str, uint8_t size1, uint8_t mode);
void OLED_DrawFont16(uint16_t x, uint16_t y, uint8_t *s, uint8_t mode);
void OLED_DrawFont16_Str(uint16_t x, uint16_t y, uint8_t *str, uint8_t mode);
void OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[]);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t size2);

#endif /* __SSD1306_H_ */
