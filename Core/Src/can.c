/*
 * can.c
 *
 *  Created on: Mar 15, 2026
 *      Author: buiqu
 */

#include "can.h"
#include "utilities.h"

#ifndef TX_LED_Pin
#define TX_LED_Pin GPIO_PIN_15
#endif

#ifndef TX_LED_GPIO_Port
#define TX_LED_GPIO_Port GPIOB
#endif

#ifndef RX_LED_Pin
#define RX_LED_Pin GPIO_PIN_8
#endif

#ifndef RX_LED_GPIO_Port
#define RX_LED_GPIO_Port GPIOA
#endif

#ifndef CAN_ERR_LED_Pin
#define CAN_ERR_LED_Pin GPIO_PIN_10
#endif

#ifndef CAN_ERR_LED_GPIO_Port
#define CAN_ERR_LED_GPIO_Port GPIOA
#endif

static CAN_HandleTypeDef *can = NULL;

static CAN_TxHeaderTypeDef tx_header;
static uint8_t tx_data[8];
static uint32_t tx_mailbox;
static volatile uint32_t can_error_code = 0;

static void can_filter_accept_all(CAN_HandleTypeDef *_can);

static uint8_t can_get_blink_count(uint32_t err) {
	if (err & HAL_CAN_ERROR_ACK)
		return 1;
	if (err & HAL_CAN_ERROR_STF)
		return 2;
	if (err & HAL_CAN_ERROR_FOR)
		return 3;
	if (err & HAL_CAN_ERROR_BD)
		return 4;
	if (err & HAL_CAN_ERROR_CRC)
		return 5;

	return 0;
}

static void can_error_led_task(void) {
	static uint32_t last_tick = 0;
	static uint8_t blink_count = 0;
	static uint8_t current_blink = 0;
	static uint8_t led_state = 0;

	uint32_t now = HAL_GetTick();

	if (can_error_code & HAL_CAN_ERROR_BOF) {
		// Fast blink for Bus Off
		if (now - last_tick > 100) {
			last_tick = now;
			HAL_GPIO_TogglePin(CAN_ERR_LED_GPIO_Port, CAN_ERR_LED_Pin);
		}
		return;
	}

	if (blink_count == 0) {
		blink_count = can_get_blink_count(can_error_code);
		current_blink = 0;
	}

	if (blink_count == 0) {
		HAL_GPIO_WritePin(CAN_ERR_LED_GPIO_Port, CAN_ERR_LED_Pin,
				GPIO_PIN_RESET);
		return;
	}

	if (now - last_tick > 500) {
		last_tick = now;

		if (led_state == 0) {
			HAL_GPIO_WritePin(CAN_ERR_LED_GPIO_Port, CAN_ERR_LED_Pin,
					GPIO_PIN_SET);
			led_state = 1;
		} else {
			HAL_GPIO_WritePin(CAN_ERR_LED_GPIO_Port, CAN_ERR_LED_Pin,
					GPIO_PIN_RESET);
			led_state = 0;
			current_blink++;

			if (current_blink >= blink_count) {
				current_blink = 0;
				blink_count = 0;
				last_tick = now + 800; // pause between sequences
			}
		}
	}
}

void can_init(CAN_HandleTypeDef *_can) {
	can = _can;

	can_filter_accept_all(can);

	if (HAL_CAN_Start(can) != HAL_OK) {
		uart_print("[CAN] Failed to start CAN\r\n");
		return;
	}

	if (HAL_CAN_ActivateNotification(can,
	CAN_IT_RX_FIFO0_MSG_PENDING |
	CAN_IT_TX_MAILBOX_EMPTY |
	CAN_IT_ERROR | CAN_IT_BUSOFF |
	CAN_IT_LAST_ERROR_CODE) != HAL_OK) {
		uart_print("[CAN] Failed to activate notification\r\n");
		return;
	}

	// Set up tx heade
	tx_header.ExtId = 0;
	tx_header.IDE = CAN_ID_STD;
	tx_header.RTR = CAN_RTR_DATA;
	tx_header.DLC = 8;
	tx_header.TransmitGlobalTime = DISABLE;
}

void can_update() {
	can_error_led_task();
}

void can_send(uint32_t can_id, const uint8_t *data) {
	tx_header.StdId = can_id;

	for (uint8_t i = 0; i < 8; ++i)
		tx_data[i] = data[i];

	if (HAL_CAN_AddTxMessage(can, &tx_header, tx_data, &tx_mailbox) != HAL_OK) {
		uart_print("[CAN] Failed to send can message\r\n");
		HAL_GPIO_WritePin(TX_LED_GPIO_Port, TX_LED_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(TX_LED_GPIO_Port, TX_LED_Pin, GPIO_PIN_SET);
	}
}

void can_tx_callback() {
	can_error_code = HAL_CAN_ERROR_NONE;
	HAL_GPIO_WritePin(TX_LED_GPIO_Port, TX_LED_Pin, GPIO_PIN_RESET);
}

void can_error_callback() {
	can_error_code = HAL_CAN_GetError(can);
}

static void can_filter_accept_all(CAN_HandleTypeDef *_can) {
	CAN_FilterTypeDef filter = { 0 };

	// Accept all IDs into FIFO0
	filter.FilterBank = 0;
	filter.FilterMode = CAN_FILTERMODE_IDMASK;
	filter.FilterScale = CAN_FILTERSCALE_32BIT;
	filter.FilterIdHigh = 0x0000;
	filter.FilterIdLow = 0x0000;
	filter.FilterMaskIdHigh = 0x0000;
	filter.FilterMaskIdLow = 0x0000;
	filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter.FilterActivation = ENABLE;

	// If your MCU has CAN2, set this appropriately; otherwise keep 14
	filter.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(can, &filter) != HAL_OK) {
		uart_print("[CAN] Failed to config filter\r\n");
	}
}
