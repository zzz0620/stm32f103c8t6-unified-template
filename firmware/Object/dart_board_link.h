#ifndef DART_BOARD_LINK_H
#define DART_BOARD_LINK_H

#include <stdint.h>

#define DART_LINK_PROTOCOL_VERSION 1U
#define DART_LINK_CAN_ORDINAL 2U

#define DART_LINK_CAN_HOST_COMMAND_ID      0x320U
#define DART_LINK_CAN_TELEMETRY_STATUS_ID  0x321U
#define DART_LINK_CAN_TELEMETRY_TARGET_ID  0x322U
#define DART_LINK_CAN_TELEMETRY_FORCE_ID   0x323U
#define DART_LINK_CAN_TELEMETRY_ORDER_ID   0x324U

#define DART_LINK_TELEMETRY_SOF 0xA5U
#define DART_LINK_TELEMETRY_TARGET_SOF 0xA6U
#define DART_LINK_TELEMETRY_FORCE_SOF 0xA7U
#define DART_LINK_TELEMETRY_ORDER_SOF 0xA8U
#define DART_LINK_COMMAND_SOF   0x5AU

#define DART_LINK_COMMAND_FLAG_HOST_ACTIVE (1U << 0)
#define DART_LINK_COMMAND_FLAG_HOST_ONLINE (1U << 1)

#define DART_LINK_TELEMETRY_FLAG_PROFILE_READY (1U << 0)
#define DART_LINK_TELEMETRY_FLAG_UI_ACTIVE     (1U << 1)
#define DART_LINK_TELEMETRY_FLAG_HOST_ACTIVE   (1U << 2)
#define DART_LINK_TELEMETRY_FLAG_CYCLE_STARTED (1U << 3)
#define DART_LINK_TELEMETRY_FLAG_FLASH_VALID   (1U << 4)
#define DART_LINK_TELEMETRY_FLAG_CONTROL_CIRCLE (1U << 5)

#define DART_LINK_ORDER_COUNT 4U

#define DART_LINK_CONTROL_MODE_FORCE  0U
#define DART_LINK_CONTROL_MODE_CIRCLE 1U

typedef struct
{
    uint8_t flags;
    uint16_t seq;
    uint8_t shot_index;
    uint8_t body_id;
    uint8_t control_mode;
    uint8_t reserved0;
    int16_t yaw_ddeg;
    uint16_t target_force_cN;
    uint16_t total_force_cN;
    uint8_t order[DART_LINK_ORDER_COUNT];
} dart_link_telemetry_t;

typedef struct
{
    uint8_t flags;
    uint16_t seq;
    uint8_t shot_index;
    uint8_t distance_index;
    uint8_t reserved[4];
} dart_link_command_t;

static inline uint16_t dart_link_float_to_centi_n(float value)
{
    int32_t scaled = 0;

    if (value <= 0.0f)
    {
        return 0U;
    }

    scaled = (int32_t)(value * 100.0f + 0.5f);
    if (scaled < 0)
    {
        scaled = 0;
    }
    if (scaled > 65535)
    {
        scaled = 65535;
    }

    return (uint16_t)scaled;
}

static inline float dart_link_centi_n_to_float(uint16_t value)
{
    return ((float)value) / 100.0f;
}

static inline int16_t dart_link_float_to_ddeg(float value)
{
    int32_t scaled = 0;

    if (value >= 0.0f)
    {
        scaled = (int32_t)(value * 10.0f + 0.5f);
    }
    else
    {
        scaled = (int32_t)(value * 10.0f - 0.5f);
    }

    if (scaled > 32767)
    {
        scaled = 32767;
    }
    if (scaled < -32768)
    {
        scaled = -32768;
    }

    return (int16_t)scaled;
}

static inline float dart_link_ddeg_to_float(int16_t value)
{
    return ((float)value) / 10.0f;
}

#endif /* DART_BOARD_LINK_H */
