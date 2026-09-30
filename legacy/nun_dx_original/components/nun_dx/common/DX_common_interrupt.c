#include "DX_common_interrupt.h"

void interrupt_global_enable (void)
{
    __enable_irq();
}

void interrupt_global_disable (void)
{
    __disable_irq();
}

void interrupt_set_priority_group (uint32 group)
{
    HAL_NVIC_SetPriorityGrouping(group);
}

void interrupt_set_priority (IRQn_Type irq, uint32 preempt, uint32 sub)
{
    uint32 priority = NVIC_EncodePriority(NVIC_GetPriorityGrouping(), preempt, sub);
    HAL_NVIC_SetPriority(irq, priority, 0);
}

void interrupt_enable (IRQn_Type irq)
{
    HAL_NVIC_EnableIRQ(irq);
}

void interrupt_disable (IRQn_Type irq)
{
    HAL_NVIC_DisableIRQ(irq);
}

void interrupt_set_pending (IRQn_Type irq)
{
    HAL_NVIC_SetPendingIRQ(irq);
}

void interrupt_clear_pending (IRQn_Type irq)
{
    HAL_NVIC_ClearPendingIRQ(irq);
}
