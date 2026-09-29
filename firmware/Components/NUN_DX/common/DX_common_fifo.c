#include "DX_debug.h"
#include "DX_common_fifo.h"

static void fifo_head_offset (fifo_struct *fifo, uint32 offset)
{
    fifo->head += offset;
    while (fifo->max <= fifo->head)
    {
        fifo->head -= fifo->max;
    }
}

static void fifo_end_offset (fifo_struct *fifo, uint32 offset)
{
    fifo->end += offset;
    while (fifo->max <= fifo->end)
    {
        fifo->end -= fifo->max;
    }
}

fifo_state_enum fifo_clear (fifo_struct *fifo)
{
    DX_assert(NULL != fifo);
    fifo_state_enum return_state = FIFO_SUCCESS;
    do
    {
        fifo->execution |= FIFO_RESET;
        fifo->head      = 0;
        fifo->end       = 0;
        fifo->size      = fifo->max;
        switch (fifo->type)
        {
            case FIFO_DATA_8BIT:    memset(fifo->buffer, 0, fifo->max);     break;
            case FIFO_DATA_16BIT:   memset(fifo->buffer, 0, fifo->max * 2); break;
            case FIFO_DATA_32BIT:   memset(fifo->buffer, 0, fifo->max * 4); break;
        }
        fifo->execution = FIFO_IDLE;
    } while (0);
    return return_state;
}

uint32 fifo_used (fifo_struct *fifo)
{
    DX_assert(fifo != NULL);
    return (fifo->max - fifo->size);
}

fifo_state_enum fifo_write_element (fifo_struct *fifo, uint32 dat)
{
    DX_assert(NULL != fifo);
    fifo_state_enum return_state = FIFO_SUCCESS;
    do
    {
        if ((FIFO_RESET | FIFO_WRITE) & fifo->execution)
        {
            return_state = FIFO_WRITE_UNDO;
            break;
        }
        fifo->execution |= FIFO_WRITE;

        if (1 <= fifo->size)
        {
            switch (fifo->type)
            {
                case FIFO_DATA_8BIT:    ((uint8 *)fifo->buffer)[fifo->head]  = dat;  break;
                case FIFO_DATA_16BIT:   ((uint16 *)fifo->buffer)[fifo->head] = dat; break;
                case FIFO_DATA_32BIT:   ((uint32 *)fifo->buffer)[fifo->head] = dat; break;
            }
            fifo_head_offset(fifo, 1);
            fifo->size -= 1;
        }
        else
        {
            return_state = FIFO_SPACE_NO_ENOUGH;
        }
        fifo->execution &= ~FIFO_WRITE;
    } while (0);
    return return_state;
}

fifo_state_enum fifo_write_buffer (fifo_struct *fifo, void *dat, uint32 length)
{
    DX_assert(NULL != fifo);
    fifo_state_enum return_state = FIFO_SUCCESS;
    uint32 temp_length = 0;

    do
    {
        if (NULL == dat)
        {
            return_state = FIFO_BUFFER_NULL;
            break;
        }
        if ((FIFO_RESET | FIFO_WRITE) & fifo->execution)
        {
            return_state = FIFO_WRITE_UNDO;
            break;
        }
        fifo->execution |= FIFO_WRITE;

        if (length <= fifo->size)
        {
            temp_length = fifo->max - fifo->head;

            if (length > temp_length)
            {
                switch (fifo->type)
                {
                    case FIFO_DATA_8BIT:
                    {
                        memcpy(&(((uint8 *)fifo->buffer)[fifo->head]), dat, temp_length);
                        fifo_head_offset(fifo, temp_length);
                        memcpy(&(((uint8 *)fifo->buffer)[fifo->head]), &(((uint8 *)dat)[temp_length]), length - temp_length);
                        fifo_head_offset(fifo, length - temp_length);
                    } break;
                    case FIFO_DATA_16BIT:
                    {
                        memcpy(&(((uint16 *)fifo->buffer)[fifo->head]), dat, temp_length * 2);
                        fifo_head_offset(fifo, temp_length);
                        memcpy(&(((uint16 *)fifo->buffer)[fifo->head]), &(((uint16 *)dat)[temp_length]), (length - temp_length) * 2);
                        fifo_head_offset(fifo, length - temp_length);
                    } break;
                    case FIFO_DATA_32BIT:
                    {
                        memcpy(&(((uint32 *)fifo->buffer)[fifo->head]), dat, temp_length * 4);
                        fifo_head_offset(fifo, temp_length);
                        memcpy(&(((uint32 *)fifo->buffer)[fifo->head]), &(((uint32 *)dat)[temp_length]), (length - temp_length) * 4);
                        fifo_head_offset(fifo, length - temp_length);
                    } break;
                }
            }
            else
            {
                switch (fifo->type)
                {
                    case FIFO_DATA_8BIT:    memcpy(&(((uint8 *)fifo->buffer)[fifo->head]), dat, length);        fifo_head_offset(fifo, length); break;
                    case FIFO_DATA_16BIT:   memcpy(&(((uint16 *)fifo->buffer)[fifo->head]), dat, length * 2);   fifo_head_offset(fifo, length); break;
                    case FIFO_DATA_32BIT:   memcpy(&(((uint32 *)fifo->buffer)[fifo->head]), dat, length * 4);   fifo_head_offset(fifo, length); break;
                }
            }
            fifo->size -= length;
        }
        else
        {
            return_state = FIFO_SPACE_NO_ENOUGH;
        }
        fifo->execution &= ~FIFO_WRITE;
    } while (0);
    return return_state;
}

fifo_state_enum fifo_read_element (fifo_struct *fifo, void *dat, fifo_operation_enum flag)
{
    DX_assert(NULL != fifo);
    fifo_state_enum return_state = FIFO_SUCCESS;

    do
    {
        if (NULL == dat)
        {
            return_state = FIFO_BUFFER_NULL;
        }
        else
        {
            if ((FIFO_RESET | FIFO_CLEAR) & fifo->execution)
            {
                return_state = FIFO_READ_UNDO;
                break;
            }

            if (1 > fifo_used(fifo))
            {
                return_state = FIFO_DATA_NO_ENOUGH;
                break;
            }

            fifo->execution |= FIFO_READ;
            switch (fifo->type)
            {
                case FIFO_DATA_8BIT:    *((uint8 *)dat)  = ((uint8 *)fifo->buffer)[fifo->end];  break;
                case FIFO_DATA_16BIT:   *((uint16 *)dat) = ((uint16 *)fifo->buffer)[fifo->end]; break;
                case FIFO_DATA_32BIT:   *((uint32 *)dat) = ((uint32 *)fifo->buffer)[fifo->end]; break;
            }
            fifo->execution &= ~FIFO_READ;
        }

        if (FIFO_READ_AND_CLEAN == flag)
        {
            if ((FIFO_RESET | FIFO_CLEAR | FIFO_READ) == fifo->execution)
            {
                return_state = FIFO_CLEAR_UNDO;
                break;
            }
            fifo->execution |= FIFO_CLEAR;
            fifo_end_offset(fifo, 1);
            fifo->size += 1;
            fifo->execution &= ~FIFO_CLEAR;
        }
    } while (0);
    return return_state;
}

fifo_state_enum fifo_read_buffer (fifo_struct *fifo, void *dat, uint32 *length, fifo_operation_enum flag)
{
    DX_assert(NULL != fifo);
    DX_assert(NULL != length);
    fifo_state_enum return_state = FIFO_SUCCESS;
    uint32 temp_length = 0;
    uint32 fifo_data_length = 0;

    do
    {
        if (NULL == dat)
        {
            return_state = FIFO_BUFFER_NULL;
        }
        else
        {
            if ((FIFO_RESET | FIFO_CLEAR) & fifo->execution)
            {
                *length = fifo_data_length;
                return_state = FIFO_READ_UNDO;
                break;
            }

            fifo_data_length = fifo_used(fifo);
            if (*length > fifo_data_length)
            {
                *length = fifo_data_length;
                return_state = FIFO_DATA_NO_ENOUGH;
                if (0 == fifo_data_length)
                {
                    fifo->execution &= ~FIFO_READ;
                    break;
                }
            }

            fifo->execution |= FIFO_READ;
            temp_length = fifo->max - fifo->end;
            if (*length <= temp_length)
            {
                switch (fifo->type)
                {
                    case FIFO_DATA_8BIT:    memcpy(dat, &(((uint8 *)fifo->buffer)[fifo->end]), *length);        break;
                    case FIFO_DATA_16BIT:   memcpy(dat, &(((uint16 *)fifo->buffer)[fifo->end]), *length * 2);   break;
                    case FIFO_DATA_32BIT:   memcpy(dat, &(((uint32 *)fifo->buffer)[fifo->end]), *length * 4);   break;
                }
            }
            else
            {
                switch (fifo->type)
                {
                    case FIFO_DATA_8BIT:
                    {
                        memcpy(dat, &(((uint8 *)fifo->buffer)[fifo->end]), temp_length);
                        memcpy(&(((uint8 *)dat)[temp_length]), fifo->buffer, *length - temp_length);
                    } break;
                    case FIFO_DATA_16BIT:
                    {
                        memcpy(dat, &(((uint16 *)fifo->buffer)[fifo->end]), temp_length * 2);
                        memcpy(&(((uint16 *)dat)[temp_length]), fifo->buffer, (*length - temp_length) * 2);
                    } break;
                    case FIFO_DATA_32BIT:
                    {
                        memcpy(dat, &(((uint32 *)fifo->buffer)[fifo->end]), temp_length * 4);
                        memcpy(&(((uint32 *)dat)[temp_length]), fifo->buffer, (*length - temp_length) * 4);
                    } break;
                }
            }
            fifo->execution &= ~FIFO_READ;
        }

        if (FIFO_READ_AND_CLEAN == flag)
        {
            if ((FIFO_RESET | FIFO_CLEAR | FIFO_READ) == fifo->execution)
            {
                return_state = FIFO_CLEAR_UNDO;
                break;
            }
            fifo->execution |= FIFO_CLEAR;
            fifo_end_offset(fifo, *length);
            fifo->size += *length;
            fifo->execution &= ~FIFO_CLEAR;
        }
    } while (0);
    return return_state;
}

fifo_state_enum fifo_read_tail_buffer (fifo_struct *fifo, void *dat, uint32 *length, fifo_operation_enum flag)
{
    DX_assert(NULL != fifo);
    DX_assert(NULL != length);
    fifo_state_enum return_state = FIFO_SUCCESS;
    uint32 temp_length = 0;
    uint32 fifo_data_length = 0;

    do
    {
        if (NULL == dat)
        {
            return_state = FIFO_BUFFER_NULL;
        }
        else
        {
            if ((FIFO_RESET | FIFO_CLEAR | FIFO_WRITE) & fifo->execution)
            {
                *length = fifo_data_length;
                return_state = FIFO_READ_UNDO;
                break;
            }

            fifo_data_length = fifo_used(fifo);
            if (*length > fifo_data_length)
            {
                *length = fifo_data_length;
                return_state = FIFO_DATA_NO_ENOUGH;
                if (0 == fifo_data_length)
                {
                    fifo->execution &= ~FIFO_READ;
                    break;
                }
            }

            fifo->execution |= FIFO_READ;
            if ((fifo->head > fifo->end) || (fifo->head >= *length))
            {
                switch (fifo->type)
                {
                    case FIFO_DATA_8BIT:    memcpy(dat, &(((uint8 *)fifo->buffer)[fifo->head - *length]), *length);      break;
                    case FIFO_DATA_16BIT:   memcpy(dat, &(((uint16 *)fifo->buffer)[fifo->head - *length]), *length * 2); break;
                    case FIFO_DATA_32BIT:   memcpy(dat, &(((uint32 *)fifo->buffer)[fifo->head - *length]), *length * 4); break;
                }
            }
            else
            {
                temp_length = *length - fifo->head;
                switch (fifo->type)
                {
                    case FIFO_DATA_8BIT:
                    {
                        memcpy(dat, &(((uint8 *)fifo->buffer)[fifo->max - temp_length]), temp_length);
                        memcpy(&(((uint8 *)dat)[temp_length]), &(((uint8 *)fifo->buffer)[fifo->head - *length]), (*length - temp_length));
                    } break;
                    case FIFO_DATA_16BIT:
                    {
                        memcpy(dat, &(((uint16 *)fifo->buffer)[fifo->max - temp_length]), temp_length * 2);
                        memcpy(&(((uint16 *)dat)[temp_length]), &(((uint16 *)fifo->buffer)[fifo->head - *length]), (*length - temp_length) * 2);
                    } break;
                    case FIFO_DATA_32BIT:
                    {
                        memcpy(dat, &(((uint32 *)fifo->buffer)[fifo->max - temp_length]), temp_length * 4);
                        memcpy(&(((uint32 *)dat)[temp_length]), &(((uint32 *)fifo->buffer)[fifo->head - *length]), (*length - temp_length) * 4);
                    } break;
                }
            }
            fifo->execution &= ~FIFO_READ;
        }

        if (FIFO_READ_AND_CLEAN == flag)
        {
            if ((FIFO_RESET | FIFO_CLEAR | FIFO_READ) == fifo->execution)
            {
                return_state = FIFO_CLEAR_UNDO;
                break;
            }
            fifo_clear(fifo);
        }
    } while (0);
    return return_state;
}

fifo_state_enum fifo_init (fifo_struct *fifo, fifo_data_type_enum type, void *buffer_addr, uint32 size)
{
    DX_assert(NULL != fifo);
    fifo_state_enum return_state = FIFO_SUCCESS;
    do
    {
        fifo->buffer    = buffer_addr;
        fifo->execution = FIFO_IDLE;
        fifo->type      = type;
        fifo->head      = 0;
        fifo->end       = 0;
        fifo->size      = size;
        fifo->max       = size;
    } while (0);
    return return_state;
}
