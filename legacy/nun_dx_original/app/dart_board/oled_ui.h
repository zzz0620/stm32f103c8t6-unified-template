#ifndef __OLED_UI_H__
#define __OLED_UI_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void oled_ui_init(void);
void oled_ui_task(void);

extern volatile uint32_t g_oled_ui_init_count;
extern volatile uint32_t g_oled_ui_task_count;
extern volatile uint32_t g_oled_ui_refresh_count;
extern volatile uint32_t g_oled_ui_send_attempt_count;
extern volatile uint32_t g_oled_ui_can_get_u16_count;
extern volatile uint32_t g_oled_ui_can_put_u16_count;

#ifdef __cplusplus
}
#endif

#endif /* __OLED_UI_H__ */
