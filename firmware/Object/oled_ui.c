#include "oled_ui.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "can.h"
#include "dart_board_link.h"
#include "i2c.h"
#include "oled.h"
#include "usart_tsak.h"

#define OLED_UI_LINE_BUF_LEN      22U
#define OLED_UI_FLASH_PAGE_ADDR   0x0800FC00U
#define OLED_UI_FLASH_MAGIC       0x44525431UL
#define OLED_UI_FLASH_VERSION     0x0003U
#define OLED_UI_SHOT_COUNT        4U
#define OLED_UI_BODY_COUNT        8U
#define OLED_UI_LONG_PRESS_MS     450U
#define OLED_UI_KEYPAD_ADDRESS    0x42U
#define OLED_UI_STEP_COUNT        6U
#define OLED_UI_HOST_TIMEOUT_MS   500U
#define OLED_UI_LINK_PERIOD_MS    20U
#define OLED_UI_LOAD_REFRESH_MS   150U
#define OLED_UI_KEY_I2C_TIMEOUT_MS 1U
#define OLED_UI_TUNE_ITEM_COUNT   2U
#define OLED_UI_MENU_ITEM_COUNT   4U

#define OLED_UI_KEY_A6_MASK       (1U << 6)
#define OLED_UI_KEY_B1_MASK       (1U << 1)
#define OLED_UI_KEY_B2_MASK       (1U << 2)
#define OLED_UI_KEY_B3_MASK       (1U << 3)
#define OLED_UI_KEY_B6_MASK       (1U << 6)

typedef enum
{
    OLED_UI_PAGE_LOAD = 0,
    OLED_UI_PAGE_MENU,
    OLED_UI_PAGE_SELECT,
    OLED_UI_PAGE_TUNE,
} oled_ui_page_e;

typedef enum
{
    OLED_UI_CONTROL_FORCE = 0,
    OLED_UI_CONTROL_CIRCLE,
} oled_ui_control_mode_e;

typedef enum
{
    OLED_UI_TUNE_ITEM_YAW = 0,
    OLED_UI_TUNE_ITEM_CIRCLE,
} oled_ui_tune_item_e;

typedef enum
{
    OLED_UI_MENU_TUNE = 0,
    OLED_UI_MENU_ORDER,
    OLED_UI_MENU_MODE,
    OLED_UI_MENU_BACK,
} oled_ui_menu_item_e;

typedef struct 
{
    uint32_t magic;
    uint16_t version;
    uint16_t reserved0;
    uint8_t order[OLED_UI_SHOT_COUNT];
    uint8_t control_mode;
    uint8_t reserved1[3];
    float body_yaw[OLED_UI_BODY_COUNT];
    float body_circle[OLED_UI_BODY_COUNT];
    uint16_t crc16;
    uint16_t reserved2;
} oled_ui_flash_blob_t;

typedef struct
{
    uint8_t order[OLED_UI_SHOT_COUNT];
    float body_yaw[OLED_UI_BODY_COUNT];
    float body_circle[OLED_UI_BODY_COUNT];
    float shot_yaw[OLED_UI_SHOT_COUNT];
    float shot_force[OLED_UI_SHOT_COUNT];
    float shot_circle[OLED_UI_SHOT_COUNT];
    uint8_t control_mode;
} oled_ui_profile_t;

typedef struct
{
    bool initialized;
    bool screen_on;
    bool profile_ready;
    bool cycle_started;
    bool flash_valid;
    bool host_online;
    bool host_cycle_active;
    bool b2_down;
    bool b2_long_fired;
    bool wait_key_release;
    bool lock_a6;
    bool lock_b6;
    bool lock_b3;
    bool lock_b1;
    uint8_t menu_item;
    uint8_t select_item;
    uint8_t tune_item;
    uint8_t tune_step_idx;
    uint8_t tune_shot_idx;
    uint8_t tune_saved_count;
    uint8_t host_shot_index;
    uint32_t b2_down_ms;
    uint32_t host_update_ms;
    uint32_t last_render_ms;
    uint16_t telemetry_seq;
    float edit_yaw;
    float edit_circle;
    oled_ui_page_e page;
    oled_ui_profile_t profile;
    oled_ui_profile_t staged;
    dart_link_command_t host_command;
} oled_ui_ctx_t;

static const float g_oled_ui_force_table[OLED_UI_BODY_COUNT][2] = {
    {28.5f, 30.5f},
    {30.0f, 32.0f},
    {28.0f, 30.0f},
    {24.0f, 26.0f},
    {28.5f, 30.5f},
    {30.0f, 32.0f},
    {28.0f, 30.0f},
    {24.0f, 26.0f},
};

static const float g_oled_ui_circle_table[OLED_UI_BODY_COUNT][2] = {
    {1.0f, 1.0f},
    {1.0f, 1.0f},
    {1.0f, 1.0f},
    {1.0f, 1.0f},
    {1.0f, 1.0f},
    {1.0f, 1.0f},
    {1.0f, 1.0f},
    {1.0f, 1.0f},
};

static const float g_oled_ui_step_table[OLED_UI_STEP_COUNT] = {0.1f, 0.5f, 1.0f, 5.0f, 10.0f, 20.0f};

static oled_ui_ctx_t g_oled_ui;
static oled_ui_flash_blob_t g_oled_ui_flash_blob;
static uint8_t g_link_tx_frame[4][8];
static uint8_t g_link_tx_pending_mask = 0U;
static uint8_t g_link_tx_next_frame = 0U;
static uint32_t g_link_last_tx_ms = 0U;

volatile uint32_t g_oled_ui_init_count = 0U;
volatile uint32_t g_oled_ui_task_count = 0U;
volatile uint32_t g_oled_ui_refresh_count = 0U;
volatile uint32_t g_oled_ui_send_attempt_count = 0U;
volatile uint32_t g_oled_ui_can_get_u16_count = 0U;
volatile uint32_t g_oled_ui_can_put_u16_count = 0U;

static uint16_t oled_ui_can_get_u16(const uint8_t *data)
{
    g_oled_ui_can_get_u16_count++;
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static void oled_ui_can_put_u16(uint8_t *data, uint16_t value)
{
    g_oled_ui_can_put_u16_count++;
    data[0] = (uint8_t)(value & 0xFFU);
    data[1] = (uint8_t)(value >> 8);
}

static bool oled_ui_can_send(uint32_t std_id, const uint8_t data[8])
{
    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox = 0U;
    HAL_StatusTypeDef tx_status;

    if (data == NULL)
    {
        return false;
    }

    g_loadcell_can_tx_last_free_level = HAL_CAN_GetTxMailboxesFreeLevel(&hcan);
    if (g_loadcell_can_tx_last_free_level == 0U)
    {
        g_loadcell_can_tx_err_count++;
        g_loadcell_can_tx_last_hal_status = (uint32_t)HAL_BUSY;
        g_loadcell_can_tx_last_error = HAL_CAN_GetError(&hcan);
        g_loadcell_can_tx_last_esr = hcan.Instance->ESR;
        g_loadcell_can_tx_last_tsr = hcan.Instance->TSR;
        return false;
    }

    memset(&tx_header, 0, sizeof(tx_header));
    tx_header.StdId = std_id;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 8U;
    tx_header.TransmitGlobalTime = DISABLE;

    tx_status = HAL_CAN_AddTxMessage(&hcan, &tx_header, data, &tx_mailbox);
    g_loadcell_can_tx_last_id = std_id;
    g_loadcell_can_tx_last_hal_status = (uint32_t)tx_status;
    g_loadcell_can_tx_last_error = HAL_CAN_GetError(&hcan);
    g_loadcell_can_tx_last_esr = hcan.Instance->ESR;
    g_loadcell_can_tx_last_tsr = hcan.Instance->TSR;

    if (tx_status == HAL_OK)
    {
        g_loadcell_can_tx_ok_count++;
        return true;
    }

    g_loadcell_can_tx_err_count++;
    return false;
}

static uint8_t oled_ui_clamp_body_id(uint32_t body_id, uint8_t fallback)
{
    if ((body_id < 1U) || (body_id > OLED_UI_BODY_COUNT))
    {
        return fallback;
    }

    return (uint8_t)body_id;
}

static uint8_t oled_ui_get_display_decimal(void)
{
    if (g_loadcell_decimal > 3U)
    {
        return 3U;
    }

    return (uint8_t)g_loadcell_decimal;
}

static float oled_ui_quantize_0p1(float value)
{
    float scaled = 0.0f;

    if (value >= 0.0f)
    {
        scaled = (float)((int32_t)(value * 10.0f + 0.5f));
    }
    else
    {
        scaled = (float)((int32_t)(value * 10.0f - 0.5f));
    }

    return scaled / 10.0f;
}

static bool oled_ui_key_pressed(uint8_t port_value, uint8_t mask)
{
    return ((port_value & mask) == 0U);
}

static bool oled_ui_any_key_pressed(uint8_t a, uint8_t b)
{
    return oled_ui_key_pressed(a, OLED_UI_KEY_A6_MASK) ||
           oled_ui_key_pressed(b, OLED_UI_KEY_B1_MASK) ||
           oled_ui_key_pressed(b, OLED_UI_KEY_B2_MASK) ||
           oled_ui_key_pressed(b, OLED_UI_KEY_B3_MASK) ||
           oled_ui_key_pressed(b, OLED_UI_KEY_B6_MASK);
}

static uint8_t oled_ui_clamp_control_mode(uint8_t mode)
{
    if (mode > (uint8_t)OLED_UI_CONTROL_CIRCLE)
    {
        return (uint8_t)OLED_UI_CONTROL_FORCE;
    }

    return mode;
}

static const char *oled_ui_get_control_mode_text(uint8_t mode)
{
    if (oled_ui_clamp_control_mode(mode) == (uint8_t)OLED_UI_CONTROL_CIRCLE)
    {
        return "CIRCLE";
    }

    return "FORCE";
}

static float oled_ui_get_active_target_value(const oled_ui_profile_t *profile, uint8_t shot_idx)
{
    if ((profile == NULL) || (shot_idx >= OLED_UI_SHOT_COUNT))
    {
        return 0.0f;
    }

    if (oled_ui_clamp_control_mode(profile->control_mode) == (uint8_t)OLED_UI_CONTROL_CIRCLE)
    {
        return profile->shot_circle[shot_idx];
    }

    return profile->shot_force[shot_idx];
}

static const char *oled_ui_get_active_target_label(const oled_ui_profile_t *profile)
{
    if ((profile != NULL) &&
        (oled_ui_clamp_control_mode(profile->control_mode) == (uint8_t)OLED_UI_CONTROL_CIRCLE))
    {
        return "CIRCLE";
    }

    return "FORCE";
}

static void oled_ui_toggle_control_mode(uint8_t *mode)
{
    if (mode == NULL)
    {
        return;
    }

    if (oled_ui_clamp_control_mode(*mode) == (uint8_t)OLED_UI_CONTROL_FORCE)
    {
        *mode = (uint8_t)OLED_UI_CONTROL_CIRCLE;
    }
    else
    {
        *mode = (uint8_t)OLED_UI_CONTROL_FORCE;
    }
}

static float oled_ui_get_body_force_n(uint8_t body_id, uint8_t distance_index)
{
    uint8_t body_index = 0U;
    uint8_t distance = distance_index;

    if ((body_id < 1U) || (body_id > OLED_UI_BODY_COUNT))
    {
        return 0.0f;
    }

    if (distance > 1U)
    {
        distance = 0U;
    }

    body_index = (uint8_t)(body_id - 1U);
    return g_oled_ui_force_table[body_index][distance];
}

static float oled_ui_get_body_circle(uint8_t body_id, uint8_t distance_index)
{
    uint8_t body_index = 0U;
    uint8_t distance = distance_index;

    if ((body_id < 1U) || (body_id > OLED_UI_BODY_COUNT))
    {
        return 0.0f;
    }

    if (distance > 1U)
    {
        distance = 0U;
    }

    body_index = (uint8_t)(body_id - 1U);
    return g_oled_ui_circle_table[body_index][distance];
}

static void oled_ui_apply_order_to_shots(oled_ui_profile_t *profile)
{
    uint8_t i = 0U;

    if (profile == NULL)
    {
        return;
    }

    for (i = 0U; i < OLED_UI_SHOT_COUNT; i++)
    {
        uint8_t body_id = oled_ui_clamp_body_id(profile->order[i], (uint8_t)(i + 1U));
        uint8_t body_index = (uint8_t)(body_id - 1U);

        profile->shot_yaw[i] = profile->body_yaw[body_index];
        profile->shot_circle[i] = profile->body_circle[body_index];
        profile->shot_force[i] = oled_ui_get_body_force_n(body_id, 0U);
    }
}

static void oled_ui_sync_tune_edit_from_body(void)
{
    uint8_t body_index = g_oled_ui.tune_shot_idx;

    if (body_index >= OLED_UI_BODY_COUNT)
    {
        body_index = 0U;
    }

    g_oled_ui.edit_yaw = g_oled_ui.staged.body_yaw[body_index];
    g_oled_ui.edit_circle = g_oled_ui.staged.body_circle[body_index];
}

static void oled_ui_show_padded_line(uint8_t row, const char *text)
{
    char line[OLED_UI_LINE_BUF_LEN];
    size_t copy_len = 0U;

    memset(line, ' ', sizeof(line) - 1U);
    line[sizeof(line) - 1U] = '\0';

    if (text != NULL)
    {
        copy_len = strlen(text);
        if (copy_len > (sizeof(line) - 1U))
        {
            copy_len = sizeof(line) - 1U;
        }
        memcpy(line, text, copy_len);
    }

    OLED_ShowStr(0, row, (uint8_t *)line, 12, 0);
}

static void oled_ui_clear_screen(void)
{
    OLED_Clear();
}

static void oled_ui_reset_nav_key_locks(void)
{
    g_oled_ui.lock_a6 = false;
    g_oled_ui.lock_b6 = false;
    g_oled_ui.lock_b3 = false;
    g_oled_ui.lock_b1 = false;
}

static void oled_ui_reset_all_key_locks(void)
{
    g_oled_ui.b2_down = false;
    g_oled_ui.b2_long_fired = false;
    g_oled_ui.wait_key_release = false;
    oled_ui_reset_nav_key_locks();
}

static void oled_ui_close_screen(void)
{
    g_oled_ui.screen_on = false;
    oled_ui_reset_nav_key_locks();
    oled_ui_clear_screen();
}

static void oled_ui_render_load_page(void)
{
    char buf[24];
    uint8_t active_idx = 0U;
    uint8_t body_id = 0U;
    float target_value = 0.0f;
    float target_yaw = 0.0f;
    uint8_t scaled_decimal = 0U;
    const char *target_label = oled_ui_get_active_target_label(&g_oled_ui.profile);

    if (g_oled_ui.profile_ready)
    {
        active_idx = g_oled_ui.host_shot_index;
        if (active_idx >= OLED_UI_SHOT_COUNT)
        {
            active_idx = 0U;
        }
    }

    body_id = g_oled_ui.profile.order[active_idx];
    target_yaw = g_oled_ui.profile.shot_yaw[active_idx];
    target_value = oled_ui_get_active_target_value(&g_oled_ui.profile, active_idx);

    scaled_decimal = (uint8_t)(oled_ui_get_display_decimal() + 2U);
    if (scaled_decimal > 3U)
    {
        scaled_decimal = 3U;
    }

    oled_ui_show_padded_line(0, "LOADCELL");

    snprintf(buf, sizeof(buf), "L:%7.*f", (int)scaled_decimal, g_loadcell_addr1_value.value / 100.0f);
    oled_ui_show_padded_line(1, buf);

    snprintf(buf, sizeof(buf), "R:%7.*f", (int)scaled_decimal, g_loadcell_addr2_value.value / 100.0f);
    oled_ui_show_padded_line(2, buf);

    snprintf(buf, sizeof(buf), "T:%7.*f", (int)scaled_decimal, total_loadcell_value / 100.0f);
    oled_ui_show_padded_line(3, buf);

    snprintf(buf, sizeof(buf), "SHOT:%d BODY:%d", (int)(active_idx + 1U), (int)body_id);
    oled_ui_show_padded_line(4, buf);

    snprintf(buf, sizeof(buf), "CTRL:%s RDY:%c",
             oled_ui_get_control_mode_text(g_oled_ui.profile.control_mode),
             g_oled_ui.profile_ready ? 'Y' : 'N');
    oled_ui_show_padded_line(5, buf);

    snprintf(buf, sizeof(buf), "%s:%4.1f Y:%4.1f", target_label, target_value, target_yaw);
    oled_ui_show_padded_line(6, buf);

    oled_ui_show_padded_line(7, "A6:MENU B2:EDIT");
}

static void oled_ui_render_select_page(void)
{
    char buf[24];

    oled_ui_show_padded_line(0, "DART ORDER");

    snprintf(buf, sizeof(buf), "%c1ST BODY:%d", (g_oled_ui.select_item == 0U) ? '>' : ' ',
             (int)g_oled_ui.staged.order[0]);
    oled_ui_show_padded_line(1, buf);

    snprintf(buf, sizeof(buf), "%c2ND BODY:%d", (g_oled_ui.select_item == 1U) ? '>' : ' ',
             (int)g_oled_ui.staged.order[1]);
    oled_ui_show_padded_line(2, buf);

    snprintf(buf, sizeof(buf), "%c3RD BODY:%d", (g_oled_ui.select_item == 2U) ? '>' : ' ',
             (int)g_oled_ui.staged.order[2]);
    oled_ui_show_padded_line(3, buf);

    snprintf(buf, sizeof(buf), "%c4TH BODY:%d", (g_oled_ui.select_item == 3U) ? '>' : ' ',
             (int)g_oled_ui.staged.order[3]);
    oled_ui_show_padded_line(4, buf);

    oled_ui_show_padded_line(5, "A6:SEL B1/B3:SET");
    oled_ui_show_padded_line(6, "B2:SAVE->TUNE");
    oled_ui_show_padded_line(7, "HOLD:EXIT");
}

static void oled_ui_render_menu_page(void)
{
    char buf[24];

    oled_ui_show_padded_line(0, "MENU");

    snprintf(buf, sizeof(buf), "%cEDIT BODY",
             (g_oled_ui.menu_item == (uint8_t)OLED_UI_MENU_TUNE) ? '>' : ' ');
    oled_ui_show_padded_line(1, buf);

    snprintf(buf, sizeof(buf), "%cBODY ORDER",
             (g_oled_ui.menu_item == (uint8_t)OLED_UI_MENU_ORDER) ? '>' : ' ');
    oled_ui_show_padded_line(2, buf);

    snprintf(buf, sizeof(buf), "%cCTRL:%s",
             (g_oled_ui.menu_item == (uint8_t)OLED_UI_MENU_MODE) ? '>' : ' ',
             oled_ui_get_control_mode_text(g_oled_ui.profile.control_mode));
    oled_ui_show_padded_line(3, buf);

    snprintf(buf, sizeof(buf), "%cBACK",
             (g_oled_ui.menu_item == (uint8_t)OLED_UI_MENU_BACK) ? '>' : ' ');
    oled_ui_show_padded_line(4, buf);

    oled_ui_show_padded_line(5, "A6/B1/B3:MOVE");
    oled_ui_show_padded_line(6, "B2:ENTER");
    oled_ui_show_padded_line(7, "HOLD:EXIT");
}

static void oled_ui_render_tune_page(void)
{
    char buf[24];
    uint8_t body_id = (uint8_t)(g_oled_ui.tune_shot_idx + 1U);

    snprintf(buf, sizeof(buf), "BODY %d %d/8",
             (int)body_id,
             (int)g_oled_ui.tune_saved_count);
    oled_ui_show_padded_line(0, buf);

    snprintf(buf, sizeof(buf), "%cYAW:%6.1f",
             (g_oled_ui.tune_item == (uint8_t)OLED_UI_TUNE_ITEM_YAW) ? '>' : ' ',
             g_oled_ui.edit_yaw);
    oled_ui_show_padded_line(1, buf);

    snprintf(buf, sizeof(buf), "%cCIRCLE:%4.1f",
             (g_oled_ui.tune_item == (uint8_t)OLED_UI_TUNE_ITEM_CIRCLE) ? '>' : ' ',
             g_oled_ui.edit_circle);
    oled_ui_show_padded_line(2, buf);

    snprintf(buf, sizeof(buf), "STEP:%5.1f", g_oled_ui_step_table[g_oled_ui.tune_step_idx]);
    oled_ui_show_padded_line(3, buf);

    oled_ui_show_padded_line(5, "B2:SAVE NEXT");
    oled_ui_show_padded_line(6, "A6:ITEM B6:STEP");
    oled_ui_show_padded_line(7, "B1/B3:SET H:B2 X");
}

static void oled_ui_render_current_page(void)
{
    if (g_oled_ui.page == OLED_UI_PAGE_LOAD)
    {
        oled_ui_render_load_page();
    }
    else if (g_oled_ui.page == OLED_UI_PAGE_MENU)
    {
        oled_ui_render_menu_page();
    }
    else if (g_oled_ui.page == OLED_UI_PAGE_SELECT)
    {
        oled_ui_render_select_page();
    }
    else
    {
        oled_ui_render_tune_page();
    }

    g_oled_ui.last_render_ms = HAL_GetTick();
}

static void oled_ui_open_page(oled_ui_page_e page)
{
    g_oled_ui.screen_on = true;
    g_oled_ui.page = page;
    oled_ui_clear_screen();
    oled_ui_render_current_page();
}

static bool oled_ui_is_flash_erased(const uint8_t *data, uint32_t len)
{
    uint32_t i = 0U;

    if (data == NULL)
    {
        return true;
    }

    for (i = 0U; i < len; i++)
    {
        if (data[i] != 0xFFU)
        {
            return false;
        }
    }

    return true;
}

static uint16_t oled_ui_flash_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFU;
    uint16_t i = 0U;

    if (data == NULL)
    {
        return 0U;
    }

    for (i = 0U; i < len; i++)
    {
        uint8_t bit = 0U;

        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc >>= 1U;
                crc ^= 0xA001U;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}

static bool oled_ui_flash_blob_valid(const oled_ui_flash_blob_t *blob)
{
    uint16_t crc = 0U;
    uint8_t i = 0U;

    if (blob == NULL)
    {
        return false;
    }

    if (oled_ui_is_flash_erased((const uint8_t *)blob, sizeof(*blob)))
    {
        return false;
    }

    if ((blob->magic != OLED_UI_FLASH_MAGIC) || (blob->version != OLED_UI_FLASH_VERSION))
    {
        return false;
    }

    for (i = 0U; i < OLED_UI_SHOT_COUNT; i++)
    {
        if ((blob->order[i] < 1U) || (blob->order[i] > OLED_UI_BODY_COUNT))
        {
            return false;
        }
    }

    if (oled_ui_clamp_control_mode(blob->control_mode) != blob->control_mode)
    {
        return false;
    }

    crc = oled_ui_flash_crc16((const uint8_t *)blob, (uint16_t)offsetof(oled_ui_flash_blob_t, crc16));
    return (crc == blob->crc16);
}

static void oled_ui_set_default_profile(oled_ui_profile_t *profile)
{
    static const uint8_t default_order[OLED_UI_SHOT_COUNT] = {1U, 2U, 3U, 4U};
    static const float default_yaw[OLED_UI_SHOT_COUNT] = {0.0f, 0.1f, -0.4f, 0.1f};
    uint8_t i = 0U;

    if (profile == NULL)
    {
        return;
    }

    memcpy(profile->order, default_order, sizeof(default_order));
    profile->control_mode = (uint8_t)OLED_UI_CONTROL_FORCE;

    for (i = 0U; i < OLED_UI_BODY_COUNT; i++)
    {
        if (i < OLED_UI_SHOT_COUNT)
        {
            profile->body_yaw[i] = default_yaw[i];
        }
        else
        {
            profile->body_yaw[i] = 0.0f;
        }
        profile->body_circle[i] = oled_ui_get_body_circle((uint8_t)(i + 1U), 0U);
    }

    oled_ui_apply_order_to_shots(profile);
}

static bool oled_ui_flash_write_profile(const oled_ui_profile_t *profile)
{
    oled_ui_flash_blob_t *blob = &g_oled_ui_flash_blob;
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0U;
    const uint16_t *data = NULL;
    uint32_t address = OLED_UI_FLASH_PAGE_ADDR;
    uint32_t words = 0U;
    uint32_t i = 0U;

    if (profile == NULL)
    {
        return false;
    }

    memset(blob, 0, sizeof(*blob));
    blob->magic = OLED_UI_FLASH_MAGIC;
    blob->version = OLED_UI_FLASH_VERSION;
    memcpy(blob->order, profile->order, sizeof(blob->order));
    blob->control_mode = oled_ui_clamp_control_mode(profile->control_mode);
    memcpy(blob->body_yaw, profile->body_yaw, sizeof(blob->body_yaw));
    memcpy(blob->body_circle, profile->body_circle, sizeof(blob->body_circle));
    blob->crc16 = oled_ui_flash_crc16((const uint8_t *)blob, (uint16_t)offsetof(oled_ui_flash_blob_t, crc16));

    HAL_FLASH_Unlock();

    memset(&erase_init, 0, sizeof(erase_init));
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = OLED_UI_FLASH_PAGE_ADDR;
    erase_init.NbPages = 1U;
    if (HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return false;
    }

    data = (const uint16_t *)blob;
    words = (uint32_t)(sizeof(*blob) / sizeof(uint16_t));
    for (i = 0U; i < words; i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return false;
        }
        address += 2U;
    }

    HAL_FLASH_Lock();
    return true;
}

static void oled_ui_load_profile(void)
{
    const oled_ui_flash_blob_t *blob = (const oled_ui_flash_blob_t *)OLED_UI_FLASH_PAGE_ADDR;

    if (oled_ui_flash_blob_valid(blob))
    {
        memcpy(g_oled_ui.profile.order, blob->order, sizeof(g_oled_ui.profile.order));
        memcpy(g_oled_ui.profile.body_yaw, blob->body_yaw, sizeof(g_oled_ui.profile.body_yaw));
        memcpy(g_oled_ui.profile.body_circle, blob->body_circle, sizeof(g_oled_ui.profile.body_circle));
        g_oled_ui.profile.control_mode = blob->control_mode;
        oled_ui_apply_order_to_shots(&g_oled_ui.profile);
        g_oled_ui.flash_valid = true;
        return;
    }

    oled_ui_set_default_profile(&g_oled_ui.profile);
    g_oled_ui.flash_valid = oled_ui_flash_write_profile(&g_oled_ui.profile);
}

static bool oled_ui_read_keys(uint8_t *a, uint8_t *b)
{
    if ((a == NULL) || (b == NULL))
    {
        return false;
    }

    if (HAL_I2C_Mem_Read(&hi2c1, OLED_UI_KEYPAD_ADDRESS, 0x12U, I2C_MEMADD_SIZE_8BIT,
                         a, 1U, OLED_UI_KEY_I2C_TIMEOUT_MS) != HAL_OK)
    {
        return false;
    }

    if (HAL_I2C_Mem_Read(&hi2c1, OLED_UI_KEYPAD_ADDRESS, 0x13U, I2C_MEMADD_SIZE_8BIT,
                         b, 1U, OLED_UI_KEY_I2C_TIMEOUT_MS) != HAL_OK)
    {
        return false;
    }

    return true;
}

static void oled_ui_prepare_staged_for_select(void)
{
    memcpy(&g_oled_ui.staged, &g_oled_ui.profile, sizeof(g_oled_ui.staged));
    g_oled_ui.select_item = 0U;
}

static void oled_ui_prepare_menu(void)
{
    g_oled_ui.menu_item = (uint8_t)OLED_UI_MENU_TUNE;
}

static void oled_ui_start_tune_for_body(uint8_t body_id)
{
    uint8_t clamped_body_id = oled_ui_clamp_body_id(body_id, 1U);
    uint8_t body_index = (uint8_t)(clamped_body_id - 1U);

    g_oled_ui.tune_shot_idx = body_index;
    g_oled_ui.tune_saved_count = 0U;
    g_oled_ui.tune_item = 0U;
    g_oled_ui.tune_step_idx = 0U;
    oled_ui_sync_tune_edit_from_body();
}

static void oled_ui_prepare_staged_for_tune(uint8_t initial_body_id)
{
    memcpy(g_oled_ui.staged.order, g_oled_ui.profile.order, sizeof(g_oled_ui.staged.order));
    memcpy(g_oled_ui.staged.body_yaw, g_oled_ui.profile.body_yaw, sizeof(g_oled_ui.staged.body_yaw));
    memcpy(g_oled_ui.staged.body_circle, g_oled_ui.profile.body_circle, sizeof(g_oled_ui.staged.body_circle));
    g_oled_ui.staged.control_mode = g_oled_ui.profile.control_mode;

    oled_ui_apply_order_to_shots(&g_oled_ui.staged);
    oled_ui_start_tune_for_body(initial_body_id);
}

static void oled_ui_finish_tune_cycle(void)
{
    memcpy(&g_oled_ui.profile, &g_oled_ui.staged, sizeof(g_oled_ui.profile));
    oled_ui_apply_order_to_shots(&g_oled_ui.profile);
    g_oled_ui.flash_valid = oled_ui_flash_write_profile(&g_oled_ui.profile);
    g_oled_ui.profile_ready = true;
    g_oled_ui.cycle_started = false;
    oled_ui_reset_all_key_locks();
    oled_ui_open_page(OLED_UI_PAGE_LOAD);
    g_oled_ui.wait_key_release = true;
}

static void oled_ui_handle_short_confirm(void)
{
    if (!g_oled_ui.screen_on)
    {
        oled_ui_reset_all_key_locks();
        oled_ui_open_page(OLED_UI_PAGE_LOAD);
        return;
    }

    if (g_oled_ui.page == OLED_UI_PAGE_LOAD)
    {
        if (g_oled_ui.profile_ready)
        {
            uint8_t shot_idx = g_oled_ui.host_shot_index;
            uint8_t body_id = 1U;
            if (shot_idx >= OLED_UI_SHOT_COUNT)
            {
                shot_idx = 0U;
            }
            body_id = g_oled_ui.profile.order[shot_idx];
            oled_ui_prepare_staged_for_tune(body_id);
            oled_ui_open_page(OLED_UI_PAGE_TUNE);
        }
        else
        {
            oled_ui_prepare_staged_for_select();
            oled_ui_open_page(OLED_UI_PAGE_SELECT);
        }
        return;
    }

    if (g_oled_ui.page == OLED_UI_PAGE_MENU)
    {
        if (g_oled_ui.menu_item == (uint8_t)OLED_UI_MENU_TUNE)
        {
            uint8_t shot_idx = g_oled_ui.host_shot_index;
            uint8_t body_id = 1U;
            if (shot_idx >= OLED_UI_SHOT_COUNT)
            {
                shot_idx = 0U;
            }
            body_id = g_oled_ui.profile.order[shot_idx];
            oled_ui_prepare_staged_for_tune(body_id);
            oled_ui_open_page(OLED_UI_PAGE_TUNE);
        }
        else if (g_oled_ui.menu_item == (uint8_t)OLED_UI_MENU_ORDER)
        {
            oled_ui_prepare_staged_for_select();
            oled_ui_open_page(OLED_UI_PAGE_SELECT);
        }
        else if (g_oled_ui.menu_item == (uint8_t)OLED_UI_MENU_MODE)
        {
            oled_ui_toggle_control_mode(&g_oled_ui.profile.control_mode);
            g_oled_ui.flash_valid = oled_ui_flash_write_profile(&g_oled_ui.profile);
            oled_ui_render_current_page();
        }
        else
        {
            oled_ui_open_page(OLED_UI_PAGE_LOAD);
        }
        return;
    }

    if (g_oled_ui.page == OLED_UI_PAGE_SELECT)
    {
        uint8_t body_id = g_oled_ui.staged.order[g_oled_ui.select_item];
        memcpy(g_oled_ui.profile.order, g_oled_ui.staged.order, sizeof(g_oled_ui.profile.order));
        oled_ui_apply_order_to_shots(&g_oled_ui.profile);
        g_oled_ui.flash_valid = oled_ui_flash_write_profile(&g_oled_ui.profile);
        oled_ui_prepare_staged_for_tune(body_id);
        oled_ui_open_page(OLED_UI_PAGE_TUNE);
        return;
    }

    g_oled_ui.staged.body_yaw[g_oled_ui.tune_shot_idx] = oled_ui_quantize_0p1(g_oled_ui.edit_yaw);
    g_oled_ui.staged.body_circle[g_oled_ui.tune_shot_idx] = oled_ui_quantize_0p1(g_oled_ui.edit_circle);
    g_oled_ui.tune_saved_count = (uint8_t)(g_oled_ui.tune_shot_idx + 1U);
    oled_ui_apply_order_to_shots(&g_oled_ui.staged);

    if ((g_oled_ui.tune_shot_idx + 1U) >= OLED_UI_BODY_COUNT)
    {
        oled_ui_finish_tune_cycle();
        return;
    }

    g_oled_ui.tune_shot_idx++;
    g_oled_ui.tune_item = 0U;
    g_oled_ui.tune_step_idx = 0U;
    oled_ui_sync_tune_edit_from_body();
    oled_ui_open_page(OLED_UI_PAGE_TUNE);
}

static void oled_ui_handle_keys(void)
{
    uint8_t a = 0xFFU;
    uint8_t b = 0xFFU;
    float step = g_oled_ui_step_table[g_oled_ui.tune_step_idx];
    bool state_changed = false;

    if (!oled_ui_read_keys(&a, &b))
    {
        return;
    }

    if (g_oled_ui.wait_key_release)
    {
        if (!oled_ui_any_key_pressed(a, b))
        {
            g_oled_ui.wait_key_release = false;
        }
        return;
    }

    if (oled_ui_key_pressed(b, OLED_UI_KEY_B2_MASK))
    {
        if (!g_oled_ui.b2_down)
        {
            g_oled_ui.b2_down = true;
            g_oled_ui.b2_long_fired = false;
            g_oled_ui.b2_down_ms = HAL_GetTick();
        }
        else if (!g_oled_ui.b2_long_fired &&
                 ((HAL_GetTick() - g_oled_ui.b2_down_ms) >= OLED_UI_LONG_PRESS_MS))
        {
            g_oled_ui.b2_long_fired = true;
            oled_ui_close_screen();
        }
    }
    else if (g_oled_ui.b2_down)
    {
        g_oled_ui.b2_down = false;
        if (!g_oled_ui.b2_long_fired)
        {
            oled_ui_handle_short_confirm();
            return;
        }
    }

    if (!g_oled_ui.screen_on)
    {
        return;
    }

    if (g_oled_ui.page == OLED_UI_PAGE_LOAD)
    {
        if (!g_oled_ui.lock_a6)
        {
            if (oled_ui_key_pressed(a, OLED_UI_KEY_A6_MASK))
            {
                g_oled_ui.lock_a6 = true;
                oled_ui_prepare_menu();
                oled_ui_open_page(OLED_UI_PAGE_MENU);
                return;
            }
        }
        else if (!oled_ui_key_pressed(a, OLED_UI_KEY_A6_MASK))
        {
            g_oled_ui.lock_a6 = false;
        }

        if (!g_oled_ui.lock_b6)
        {
            if (oled_ui_key_pressed(b, OLED_UI_KEY_B6_MASK))
            {
                g_oled_ui.lock_b6 = true;
                oled_ui_toggle_control_mode(&g_oled_ui.profile.control_mode);
                g_oled_ui.flash_valid = oled_ui_flash_write_profile(&g_oled_ui.profile);
                state_changed = true;
            }
        }
        else if (!oled_ui_key_pressed(b, OLED_UI_KEY_B6_MASK))
        {
            g_oled_ui.lock_b6 = false;
        }

        if (state_changed)
        {
            oled_ui_render_current_page();
        }
        return;
    }

    if (g_oled_ui.page == OLED_UI_PAGE_MENU)
    {
        if (!g_oled_ui.lock_a6)
        {
            if (oled_ui_key_pressed(a, OLED_UI_KEY_A6_MASK))
            {
                g_oled_ui.lock_a6 = true;
                g_oled_ui.menu_item = (uint8_t)((g_oled_ui.menu_item + 1U) % OLED_UI_MENU_ITEM_COUNT);
                state_changed = true;
            }
        }
        else if (!oled_ui_key_pressed(a, OLED_UI_KEY_A6_MASK))
        {
            g_oled_ui.lock_a6 = false;
        }

        if (!g_oled_ui.lock_b3)
        {
            if (oled_ui_key_pressed(b, OLED_UI_KEY_B3_MASK))
            {
                g_oled_ui.lock_b3 = true;
                g_oled_ui.menu_item = (uint8_t)((g_oled_ui.menu_item + 1U) % OLED_UI_MENU_ITEM_COUNT);
                state_changed = true;
            }
        }
        else if (!oled_ui_key_pressed(b, OLED_UI_KEY_B3_MASK))
        {
            g_oled_ui.lock_b3 = false;
        }

        if (!g_oled_ui.lock_b1)
        {
            if (oled_ui_key_pressed(b, OLED_UI_KEY_B1_MASK))
            {
                g_oled_ui.lock_b1 = true;
                if (g_oled_ui.menu_item == 0U)
                {
                    g_oled_ui.menu_item = (uint8_t)(OLED_UI_MENU_ITEM_COUNT - 1U);
                }
                else
                {
                    g_oled_ui.menu_item--;
                }
                state_changed = true;
            }
        }
        else if (!oled_ui_key_pressed(b, OLED_UI_KEY_B1_MASK))
        {
            g_oled_ui.lock_b1 = false;
        }

        if (!g_oled_ui.lock_b6)
        {
            if (oled_ui_key_pressed(b, OLED_UI_KEY_B6_MASK))
            {
                g_oled_ui.lock_b6 = true;
                if (g_oled_ui.menu_item == (uint8_t)OLED_UI_MENU_MODE)
                {
                    oled_ui_toggle_control_mode(&g_oled_ui.profile.control_mode);
                    g_oled_ui.flash_valid = oled_ui_flash_write_profile(&g_oled_ui.profile);
                    state_changed = true;
                }
            }
        }
        else if (!oled_ui_key_pressed(b, OLED_UI_KEY_B6_MASK))
        {
            g_oled_ui.lock_b6 = false;
        }

        if (state_changed)
        {
            oled_ui_render_current_page();
        }
        return;
    }

    if (g_oled_ui.page == OLED_UI_PAGE_SELECT)
    {
        if (!g_oled_ui.lock_a6)
        {
            if (oled_ui_key_pressed(a, OLED_UI_KEY_A6_MASK))
            {
                g_oled_ui.lock_a6 = true;
                g_oled_ui.select_item = (uint8_t)((g_oled_ui.select_item + 1U) % OLED_UI_SHOT_COUNT);
                state_changed = true;
            }
        }
        else if (!oled_ui_key_pressed(a, OLED_UI_KEY_A6_MASK))
        {
            g_oled_ui.lock_a6 = false;
        }

        if (!g_oled_ui.lock_b3)
        {
            if (oled_ui_key_pressed(b, OLED_UI_KEY_B3_MASK))
            {
                g_oled_ui.lock_b3 = true;
                g_oled_ui.staged.order[g_oled_ui.select_item]++;
                if (g_oled_ui.staged.order[g_oled_ui.select_item] > OLED_UI_BODY_COUNT)
                {
                    g_oled_ui.staged.order[g_oled_ui.select_item] = 1U;
                }
                state_changed = true;
            }
        }
        else if (!oled_ui_key_pressed(b, OLED_UI_KEY_B3_MASK))
        {
            g_oled_ui.lock_b3 = false;
        }

        if (!g_oled_ui.lock_b1)
        {
            if (oled_ui_key_pressed(b, OLED_UI_KEY_B1_MASK))
            {
                g_oled_ui.lock_b1 = true;
                if (g_oled_ui.staged.order[g_oled_ui.select_item] <= 1U)
                {
                    g_oled_ui.staged.order[g_oled_ui.select_item] = OLED_UI_BODY_COUNT;
                }
                else
                {
                    g_oled_ui.staged.order[g_oled_ui.select_item]--;
                }
                state_changed = true;
            }
        }
        else if (!oled_ui_key_pressed(b, OLED_UI_KEY_B1_MASK))
        {
            g_oled_ui.lock_b1 = false;
        }

        if (state_changed)
        {
            oled_ui_render_current_page();
        }
        return;
    }

    if (!g_oled_ui.lock_a6)
    {
        if (oled_ui_key_pressed(a, OLED_UI_KEY_A6_MASK))
        {
            g_oled_ui.lock_a6 = true;
            g_oled_ui.tune_item = (uint8_t)((g_oled_ui.tune_item + 1U) % OLED_UI_TUNE_ITEM_COUNT);
            state_changed = true;
        }
    }
    else if (!oled_ui_key_pressed(a, OLED_UI_KEY_A6_MASK))
    {
        g_oled_ui.lock_a6 = false;
    }

    if (!g_oled_ui.lock_b6)
    {
        if (oled_ui_key_pressed(b, OLED_UI_KEY_B6_MASK))
        {
            g_oled_ui.lock_b6 = true;
            g_oled_ui.tune_step_idx = (uint8_t)((g_oled_ui.tune_step_idx + 1U) % OLED_UI_STEP_COUNT);
            state_changed = true;
        }
    }
    else if (!oled_ui_key_pressed(b, OLED_UI_KEY_B6_MASK))
    {
        g_oled_ui.lock_b6 = false;
    }

    step = g_oled_ui_step_table[g_oled_ui.tune_step_idx];

    if (!g_oled_ui.lock_b3)
    {
        if (oled_ui_key_pressed(b, OLED_UI_KEY_B3_MASK))
        {
            g_oled_ui.lock_b3 = true;
            if (g_oled_ui.tune_item == (uint8_t)OLED_UI_TUNE_ITEM_YAW)
            {
                g_oled_ui.edit_yaw += step;
            }
            else if (g_oled_ui.tune_item == (uint8_t)OLED_UI_TUNE_ITEM_CIRCLE)
            {
                g_oled_ui.edit_circle += step;
            }
            state_changed = true;
        }
    }
    else if (!oled_ui_key_pressed(b, OLED_UI_KEY_B3_MASK))
    {
        g_oled_ui.lock_b3 = false;
    }

    if (!g_oled_ui.lock_b1)
    {
        if (oled_ui_key_pressed(b, OLED_UI_KEY_B1_MASK))
        {
            g_oled_ui.lock_b1 = true;
            if (g_oled_ui.tune_item == (uint8_t)OLED_UI_TUNE_ITEM_YAW)
            {
                g_oled_ui.edit_yaw -= step;
            }
            else if (g_oled_ui.tune_item == (uint8_t)OLED_UI_TUNE_ITEM_CIRCLE)
            {
                g_oled_ui.edit_circle -= step;
                if (g_oled_ui.edit_circle < 0.0f)
                {
                    g_oled_ui.edit_circle = 0.0f;
                }
            }
            state_changed = true;
        }
    }
    else if (!oled_ui_key_pressed(b, OLED_UI_KEY_B1_MASK))
    {
        g_oled_ui.lock_b1 = false;
    }

    if (state_changed)
    {
        oled_ui_render_current_page();
    }
}

static void oled_ui_apply_host_command(const dart_link_command_t *command)
{
    bool host_active = false;

    if (command == NULL)
    {
        g_oled_ui.host_online = false;
        g_oled_ui.host_cycle_active = false;
        g_oled_ui.host_shot_index = 0U;
        return;
    }

    g_oled_ui.host_command = *command;
    g_oled_ui.host_online = ((command->flags & DART_LINK_COMMAND_FLAG_HOST_ONLINE) != 0U);
    host_active = ((command->flags & DART_LINK_COMMAND_FLAG_HOST_ACTIVE) != 0U);
    g_oled_ui.host_cycle_active = host_active;
    g_oled_ui.host_shot_index = command->shot_index;
    if (g_oled_ui.host_shot_index >= OLED_UI_SHOT_COUNT)
    {
        g_oled_ui.host_shot_index = 0U;
    }

    g_oled_ui.host_update_ms = HAL_GetTick();

    if (g_oled_ui.profile_ready && host_active)
    {
        g_oled_ui.cycle_started = true;
    }

    if (g_oled_ui.cycle_started && !host_active)
    {
        g_oled_ui.profile_ready = false;
        g_oled_ui.cycle_started = false;
    }
}

static void oled_ui_refresh_host_state(void)
{
    if ((HAL_GetTick() - g_oled_ui.host_update_ms) > OLED_UI_HOST_TIMEOUT_MS)
    {
        g_oled_ui.host_online = false;
        g_oled_ui.host_cycle_active = false;
        if (g_oled_ui.cycle_started)
        {
            g_oled_ui.profile_ready = false;
            g_oled_ui.cycle_started = false;
        }
    }
}

static void oled_ui_get_live_tx_profile(oled_ui_profile_t *profile_out, bool *profile_ready_out)
{
    uint8_t edit_idx = 0U;

    if ((profile_out == NULL) || (profile_ready_out == NULL))
    {
        return;
    }

    memcpy(profile_out, &g_oled_ui.profile, sizeof(*profile_out));
    *profile_ready_out = g_oled_ui.profile_ready;

    if (g_oled_ui.page == OLED_UI_PAGE_TUNE)
    {
        memcpy(profile_out, &g_oled_ui.staged, sizeof(*profile_out));
        edit_idx = g_oled_ui.tune_shot_idx;
        if (edit_idx >= OLED_UI_BODY_COUNT)
        {
            edit_idx = 0U;
        }

        profile_out->body_yaw[edit_idx] = g_oled_ui.edit_yaw;
        profile_out->body_circle[edit_idx] = g_oled_ui.edit_circle;
        profile_out->control_mode = g_oled_ui.staged.control_mode;
        oled_ui_apply_order_to_shots(profile_out);
        *profile_ready_out = true;
    }
}

static void oled_ui_refresh_telemetry_regs(void)
{
    dart_link_telemetry_t telemetry;
    oled_ui_profile_t tx_profile;
    bool tx_profile_ready = false;
    uint8_t active_idx = 0U;

    g_oled_ui_refresh_count++;

    if (g_link_tx_pending_mask != 0U)
    {
        return;
    }

    memset(&telemetry, 0, sizeof(telemetry));
    memset(g_link_tx_frame, 0, sizeof(g_link_tx_frame));
    oled_ui_get_live_tx_profile(&tx_profile, &tx_profile_ready);

    active_idx = g_oled_ui.host_shot_index;
    if (active_idx >= OLED_UI_SHOT_COUNT)
    {
        active_idx = 0U;
    }

    telemetry.flags = 0U;
    if (tx_profile_ready)
    {
        telemetry.flags |= DART_LINK_TELEMETRY_FLAG_PROFILE_READY;
    }
    if (g_oled_ui.screen_on)
    {
        telemetry.flags |= DART_LINK_TELEMETRY_FLAG_UI_ACTIVE;
    }
    if (g_oled_ui.host_cycle_active)
    {
        telemetry.flags |= DART_LINK_TELEMETRY_FLAG_HOST_ACTIVE;
    }
    if (g_oled_ui.cycle_started)
    {
        telemetry.flags |= DART_LINK_TELEMETRY_FLAG_CYCLE_STARTED;
    }
    if (g_oled_ui.flash_valid)
    {
        telemetry.flags |= DART_LINK_TELEMETRY_FLAG_FLASH_VALID;
    }
    if (oled_ui_clamp_control_mode(tx_profile.control_mode) == (uint8_t)OLED_UI_CONTROL_CIRCLE)
    {
        telemetry.flags |= DART_LINK_TELEMETRY_FLAG_CONTROL_CIRCLE;
    }

    telemetry.seq = g_oled_ui.telemetry_seq++;
    telemetry.shot_index = active_idx;
    telemetry.body_id = tx_profile.order[active_idx];
    telemetry.control_mode = oled_ui_clamp_control_mode(tx_profile.control_mode);
    telemetry.yaw_ddeg = dart_link_float_to_ddeg(tx_profile.shot_yaw[active_idx]);
    telemetry.target_force_cN = dart_link_float_to_centi_n(
        oled_ui_get_active_target_value(&tx_profile, active_idx));
    telemetry.total_force_cN = dart_link_float_to_centi_n(total_loadcell_value / 100.0f);
    memcpy(telemetry.order, tx_profile.order, sizeof(telemetry.order));

    g_link_tx_frame[0][0] = DART_LINK_TELEMETRY_SOF;
    g_link_tx_frame[0][1] = DART_LINK_PROTOCOL_VERSION;
    g_link_tx_frame[0][2] = telemetry.flags;
    oled_ui_can_put_u16(&g_link_tx_frame[0][3], telemetry.seq);
    g_link_tx_frame[0][5] = telemetry.shot_index;
    g_link_tx_frame[0][6] = telemetry.body_id;
    g_link_tx_frame[0][7] = telemetry.control_mode;

    g_link_tx_frame[1][0] = DART_LINK_TELEMETRY_TARGET_SOF;
    oled_ui_can_put_u16(&g_link_tx_frame[1][1], telemetry.seq);
    oled_ui_can_put_u16(&g_link_tx_frame[1][3], (uint16_t)telemetry.yaw_ddeg);
    oled_ui_can_put_u16(&g_link_tx_frame[1][5], telemetry.target_force_cN);

    g_link_tx_frame[2][0] = DART_LINK_TELEMETRY_FORCE_SOF;
    oled_ui_can_put_u16(&g_link_tx_frame[2][1], telemetry.seq);
    oled_ui_can_put_u16(&g_link_tx_frame[2][3], telemetry.total_force_cN);

    g_link_tx_frame[3][0] = DART_LINK_TELEMETRY_ORDER_SOF;
    oled_ui_can_put_u16(&g_link_tx_frame[3][1], telemetry.seq);
    memcpy(&g_link_tx_frame[3][3], telemetry.order, sizeof(telemetry.order));

    g_link_tx_pending_mask = 0x0FU;
    g_link_tx_next_frame = 0U;
}

static void oled_ui_send_pending_telemetry(void)
{
    static const uint32_t frame_id[4] = {
        DART_LINK_CAN_TELEMETRY_STATUS_ID,
        DART_LINK_CAN_TELEMETRY_TARGET_ID,
        DART_LINK_CAN_TELEMETRY_FORCE_ID,
        DART_LINK_CAN_TELEMETRY_ORDER_ID,
    };
    uint8_t i = 0U;

    g_oled_ui_send_attempt_count++;

    for (i = 0U; i < 4U; i++)
    {
        uint8_t frame = (uint8_t)((g_link_tx_next_frame + i) & 0x03U);
        uint8_t mask = (uint8_t)(1U << frame);

        if ((g_link_tx_pending_mask & mask) == 0U)
        {
            continue;
        }

        if (oled_ui_can_send(frame_id[frame], g_link_tx_frame[frame]))
        {
            g_link_tx_pending_mask = (uint8_t)(g_link_tx_pending_mask & (uint8_t)~mask);
            g_link_tx_next_frame = (uint8_t)((frame + 1U) & 0x03U);
        }
        return;
    }
}

static void oled_ui_can_init(void)
{
    LoadcellCanStart();
}

static void oled_ui_handle_rx_frame(const CAN_RxHeaderTypeDef *rx_header, const uint8_t *rx_data)
{
    dart_link_command_t command;

    if ((rx_header == NULL) || (rx_data == NULL))
    {
        return;
    }

    g_loadcell_can_rx_msg_count++;
    g_loadcell_can_rx_last_id = rx_header->StdId;
    g_loadcell_can_rx_last_dlc = rx_header->DLC;
    g_loadcell_can_rx_last_tick = HAL_GetTick();

    if ((rx_header->IDE != CAN_ID_STD) ||
        (rx_header->RTR != CAN_RTR_DATA) ||
        (rx_header->StdId != DART_LINK_CAN_HOST_COMMAND_ID) ||
        (rx_header->DLC < 8U) ||
        (rx_data[0] != DART_LINK_COMMAND_SOF) ||
        (rx_data[1] != DART_LINK_PROTOCOL_VERSION))
    {
        return;
    }

    memset(&command, 0, sizeof(command));
    command.flags = rx_data[2];
    command.seq = oled_ui_can_get_u16(&rx_data[3]);
    command.shot_index = rx_data[5];
    command.distance_index = rx_data[6];
    command.reserved[0] = rx_data[7];

    loadcell_can_store_host_command(&command);
    if (g_oled_ui.initialized)
    {
        oled_ui_apply_host_command(&command);
    }
}

static void oled_ui_can_poll_rx(void)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    while (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0U)
    {
        if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
        {
            return;
        }

        g_loadcell_can_rx_poll_count++;
        oled_ui_handle_rx_frame(&rx_header, rx_data);
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *can_handle)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8] = {0};

    if (can_handle != &hcan)
    {
        return;
    }

    if (HAL_CAN_GetRxMessage(can_handle, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
    {
        return;
    }

    oled_ui_handle_rx_frame(&rx_header, rx_data);
}

void oled_ui_init(void)
{
    g_oled_ui_init_count++;
    memset(&g_oled_ui, 0, sizeof(g_oled_ui));

    OLED_Init();
    oled_ui_clear_screen();
    oled_ui_load_profile();
    g_oled_ui.screen_on = true;
    oled_ui_reset_all_key_locks();
    oled_ui_open_page(OLED_UI_PAGE_LOAD);
    g_oled_ui.initialized = true;
    oled_ui_can_init();
    oled_ui_refresh_telemetry_regs();
    oled_ui_send_pending_telemetry();
    g_link_last_tx_ms = HAL_GetTick();
}

void oled_ui_task(void)
{
    g_oled_ui_task_count++;
    if (!g_oled_ui.initialized)
    {
        return;
    }

    oled_ui_can_poll_rx();
    oled_ui_refresh_host_state();
    if ((g_link_tx_pending_mask == 0U) &&
        ((HAL_GetTick() - g_link_last_tx_ms) >= OLED_UI_LINK_PERIOD_MS))
    {
        oled_ui_refresh_telemetry_regs();
        g_link_last_tx_ms = HAL_GetTick();
    }
    oled_ui_send_pending_telemetry();
    oled_ui_handle_keys();

    if (g_oled_ui.screen_on &&
        (g_oled_ui.page == OLED_UI_PAGE_LOAD) &&
        ((HAL_GetTick() - g_oled_ui.last_render_ms) >= OLED_UI_LOAD_REFRESH_MS))
    {
        oled_ui_render_current_page();
    }
}
