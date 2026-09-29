#ifndef __USART_TSAK_H__
#define __USART_TSAK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../../Object/loadcell.h"

#include <stdint.h>

extern loadcell_value_t g_loadcell_addr1_value;
extern loadcell_value_t g_loadcell_addr2_value;
extern float total_loadcell_value;
extern uint16_t g_loadcell_unit;
extern uint8_t g_loadcell_decimal;
extern loadcell_status_t g_loadcell_addr1_status;
extern loadcell_status_t g_loadcell_addr2_status;
extern uint8_t g_loadcell_test_addr;
extern loadcell_status_t g_loadcell_test_status;
extern loadcell_value_t g_loadcell_test_value;

void usart_tsak(void const *argument);

#ifdef __cplusplus
}
#endif

#endif
