/*
 * utilities.c
 *
 *  Created on: Mar 13, 2026
 *      Author: buiqu
 */
#include <string.h>

#include "stm32f1xx_hal.h"
#include "utilities.h"

#define UTILITY_UART (&huart2)

extern UART_HandleTypeDef huart2;

void uart_print(const char* s) {
	HAL_UART_Transmit(UTILITY_UART, (uint8_t*)s, strlen(s), HAL_MAX_DELAY);
}
