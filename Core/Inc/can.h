/*
 * can.h
 *
 *  Created on: Mar 15, 2026
 *      Author: buiqu
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "stm32f1xx_hal.h"

void can_init(CAN_HandleTypeDef *_can);

void can_update();

void can_send(uint32_t can_id, const uint8_t* data);

void can_tx_callback();

void can_error_callback();
#endif /* INC_CAN_H_ */
