/**
 * @file loadcell_service.h
 * @brief Bare-metal polling service for the dual-channel Modbus load cell.
 */

#ifndef LOADCELL_SERVICE_H
#define LOADCELL_SERVICE_H

#include <stdint.h>

#include "loadcell.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    loadcell_value_t channel1;
    loadcell_value_t channel2;
    float total;
    loadcell_status_t channel1_status;
    loadcell_status_t channel2_status;
} loadcell_service_sample_t;

void loadcell_service_init(UART_HandleTypeDef *uart, uint8_t slave_address);
uint8_t loadcell_service_read(loadcell_service_sample_t *sample);
const loadcell_service_sample_t *loadcell_service_latest(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADCELL_SERVICE_H */
