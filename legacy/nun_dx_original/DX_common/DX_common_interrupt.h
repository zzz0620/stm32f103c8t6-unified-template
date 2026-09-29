#ifndef __DX_COMMON_INTERRUPT_H
#define __DX_COMMON_INTERRUPT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"

/* Exported functions prototypes ---------------------------------------------*/

void interrupt_global_enable       (void);
void interrupt_global_disable      (void);
void interrupt_set_priority_group  (uint32 group);
void interrupt_set_priority        (IRQn_Type irq, uint32 preempt, uint32 sub);
void interrupt_enable              (IRQn_Type irq);
void interrupt_disable             (IRQn_Type irq);
void interrupt_set_pending         (IRQn_Type irq);
void interrupt_clear_pending       (IRQn_Type irq);

#ifdef __cplusplus
}
#endif

#endif /* __DX_COMMON_INTERRUPT_H */
