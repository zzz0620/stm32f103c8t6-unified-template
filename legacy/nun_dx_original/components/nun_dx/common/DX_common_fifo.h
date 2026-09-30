#ifndef __DX_COMMON_FIFO_H
#define __DX_COMMON_FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"

/* Exported types ------------------------------------------------------------*/

typedef enum
{
    FIFO_SUCCESS,

    FIFO_RESET_UNDO,
    FIFO_CLEAR_UNDO,
    FIFO_BUFFER_NULL,
    FIFO_WRITE_UNDO,
    FIFO_SPACE_NO_ENOUGH,
    FIFO_READ_UNDO,
    FIFO_DATA_NO_ENOUGH,
} fifo_state_enum;

typedef enum
{
    FIFO_IDLE       = 0x00,
    FIFO_RESET      = 0x01,
    FIFO_CLEAR      = 0x02,
    FIFO_WRITE      = 0x04,
    FIFO_READ       = 0x08,
} fifo_execution_enum;

typedef enum
{
    FIFO_READ_AND_CLEAN,
    FIFO_READ_ONLY,
} fifo_operation_enum;

typedef enum
{
    FIFO_DATA_8BIT,
    FIFO_DATA_16BIT,
    FIFO_DATA_32BIT,
} fifo_data_type_enum;

typedef struct __attribute__((packed))
{
    uint8               execution;
    fifo_data_type_enum type;
    void                *buffer;
    uint32              head;
    uint32              end;
    uint32              size;
    uint32              max;
} fifo_struct;

/* Exported functions prototypes ---------------------------------------------*/

fifo_state_enum fifo_clear              (fifo_struct *fifo);
uint32          fifo_used               (fifo_struct *fifo);

fifo_state_enum fifo_write_element      (fifo_struct *fifo, uint32 dat);
fifo_state_enum fifo_write_buffer       (fifo_struct *fifo, void *dat, uint32 length);
fifo_state_enum fifo_read_element       (fifo_struct *fifo, void *dat, fifo_operation_enum flag);
fifo_state_enum fifo_read_buffer        (fifo_struct *fifo, void *dat, uint32 *length, fifo_operation_enum flag);
fifo_state_enum fifo_read_tail_buffer   (fifo_struct *fifo, void *dat, uint32 *length, fifo_operation_enum flag);

fifo_state_enum fifo_init               (fifo_struct *fifo, fifo_data_type_enum type, void *buffer_addr, uint32 size);

#ifdef __cplusplus
}
#endif

#endif /* __DX_COMMON_FIFO_H */
