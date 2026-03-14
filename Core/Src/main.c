/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ds18b20.h"
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
//static CAN_TxHeaderTypeDef txHeader;
//static uint8_t txData[8];
//static uint32_t txMailbox;
//
//static CAN_RxHeaderTypeDef rxHeader;
//static uint8_t rxData[8];
//
//volatile uint8_t g_rx_flag = 0;
//volatile uint32_t g_rx_id = 0;
//volatile uint8_t g_rx_len = 0;
//volatile uint8_t g_rx_buf[8];
//
//volatile uint32_t can_error_code = 0;
uint8_t TxBuffer[] =
		"Hello World! From STM32 USB CDC Device To Virtual COM Port\r\n";
uint8_t TxBufferLen = sizeof(TxBuffer);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//static void CAN_FilterAcceptAll(CAN_HandleTypeDef *hcan) {
//	CAN_FilterTypeDef filter = { 0 };
//
//	// Accept all IDs into FIFO0
//	filter.FilterBank = 0;
//	filter.FilterMode = CAN_FILTERMODE_IDMASK;
//	filter.FilterScale = CAN_FILTERSCALE_32BIT;
//	filter.FilterIdHigh = 0x0000;
//	filter.FilterIdLow = 0x0000;
//	filter.FilterMaskIdHigh = 0x0000;
//	filter.FilterMaskIdLow = 0x0000;
//	filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
//	filter.FilterActivation = ENABLE;
//
//	// If your MCU has CAN2, set this appropriately; otherwise keep 14
//	filter.SlaveStartFilterBank = 14;
//
//	if (HAL_CAN_ConfigFilter(hcan, &filter) != HAL_OK) {
//		Error_Handler();
//	}
//}
//
//uint8_t CAN_GetBlinkCount(uint32_t err) {
//	if (err & HAL_CAN_ERROR_ACK)
//		return 1;
//	if (err & HAL_CAN_ERROR_STF)
//		return 2;
//	if (err & HAL_CAN_ERROR_FOR)
//		return 3;
//	if (err & HAL_CAN_ERROR_BD)
//		return 4;
//	if (err & HAL_CAN_ERROR_CRC)
//		return 5;
//
//	return 0;
//}
//
//void CAN_Error_LED_Task(void) {
//	static uint32_t last_tick = 0;
//	static uint8_t blink_count = 0;
//	static uint8_t current_blink = 0;
//	static uint8_t led_state = 0;
//
//	uint32_t now = HAL_GetTick();
//
//	if (can_error_code & HAL_CAN_ERROR_BOF) {
//		// Fast blink for Bus Off
//		if (now - last_tick > 100) {
//			last_tick = now;
//			HAL_GPIO_TogglePin(CAN_ERR_LED_GPIO_Port, CAN_ERR_LED_Pin);
//		}
//		return;
//	}
//
//	if (blink_count == 0) {
//		blink_count = CAN_GetBlinkCount(can_error_code);
//		current_blink = 0;
//	}
//
//	if (blink_count == 0) {
//		HAL_GPIO_WritePin(CAN_ERR_LED_GPIO_Port,
//		CAN_ERR_LED_Pin, GPIO_PIN_RESET);
//		return;
//	}
//
//	if (now - last_tick > 500) {
//		last_tick = now;
//
//		if (led_state == 0) {
//			HAL_GPIO_WritePin(CAN_ERR_LED_GPIO_Port,
//			CAN_ERR_LED_Pin, GPIO_PIN_SET);
//			led_state = 1;
//		} else {
//			HAL_GPIO_WritePin(CAN_ERR_LED_GPIO_Port,
//			CAN_ERR_LED_Pin, GPIO_PIN_RESET);
//			led_state = 0;
//			current_blink++;
//
//			if (current_blink >= blink_count) {
//				current_blink = 0;
//				blink_count = 0;
//				last_tick = now + 800; // pause between sequences
//			}
//		}
//	}
//}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_TIM2_Init();
	MX_USB_DEVICE_Init();
	/* USER CODE BEGIN 2 */
	HAL_Delay(1000);

	HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_SET);

//	CAN_FilterAcceptAll(&hcan);
//
//	if (HAL_CAN_Start(&hcan) != HAL_OK) {
//		Error_Handler();
//	}
//
//	if (HAL_CAN_ActivateNotification(&hcan,
//	CAN_IT_RX_FIFO0_MSG_PENDING |
//	CAN_IT_TX_MAILBOX_EMPTY |
//	CAN_IT_ERROR | CAN_IT_BUSOFF |
//	CAN_IT_LAST_ERROR_CODE) != HAL_OK) {
//		Error_Handler();
//	}

	ds18b20_init();

//    // Setup a test TX frame
//    txHeader.StdId = 0x123;
//    txHeader.ExtId = 0;
//    txHeader.IDE   = CAN_ID_STD;
//    txHeader.RTR   = CAN_RTR_DATA;
//    txHeader.DLC   = 8;
//    txHeader.TransmitGlobalTime = DISABLE;
//
//    // Payload "TEST0001" style
//    for (uint32_t i = 0; i < sizeof(txData); i++)
//    {
//        txData[i] = 0;
//    }
//    txData[0] = 'T';
//    txData[1] = 'E';
//    txData[2] = 'S';
//    txData[3] = 'T';
//    txData[4] = 0x00;
//    txData[5] = 0x00;
//    txData[6] = 0x00;
//    txData[7] = 0x01;
//
//    uint32_t counter = 1;
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		ds18b20_update();

		HAL_Delay(500);

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		// send every 500 ms
//	 	      txData[7] = (uint8_t)(counter & 0xFF);
//
//	 	      if (HAL_CAN_AddTxMessage(&hcan, &txHeader, txData, &txMailbox) == HAL_OK) {
//	 	    	  HAL_GPIO_WritePin(TX_LED_GPIO_Port, TX_LED_Pin, GPIO_PIN_SET);
//
//	 	          counter++;
//	 	      } else {
//	 	    	  HAL_GPIO_WritePin(TX_LED_GPIO_Port, TX_LED_Pin, GPIO_PIN_RESET);
//	 	      }
//
//	 	      // if we received something, copy is already in g_rx_buf
//	 	      uint32_t fill = HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0);
//	 	      if (fill > 0 || g_rx_flag == 1) {
//	 	          g_rx_flag = 0;
//
//	 	          // optional: toggle LED on receive
//	 	          HAL_GPIO_TogglePin(RX_LED_GPIO_Port, RX_LED_Pin);
//	 	      }
//
//		CAN_Error_LED_Task();
//
//	 	      HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
//
//	 	      HAL_Delay(500);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_IC_InitTypeDef sConfigIC = { 0 };

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 15;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 65535;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_IC_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, USER_LED_Pin | RX_LED_Pin | CAN_ERR_LED_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, DS18B20_DAT_Pin | TX_LED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : USER_LED_Pin RX_LED_Pin CAN_ERR_LED_Pin */
	GPIO_InitStruct.Pin = USER_LED_Pin | RX_LED_Pin | CAN_ERR_LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : DS18B20_DAT_Pin */
	GPIO_InitStruct.Pin = DS18B20_DAT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DS18B20_DAT_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : TX_LED_Pin */
	GPIO_InitStruct.Pin = TX_LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TX_LED_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : PB8 */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : PB9 */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure peripheral I/O remapping */
	__HAL_AFIO_REMAP_CAN1_2();

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
//	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK) {
//		return;
//	}
//
//	g_rx_id = (rxHeader.IDE == CAN_ID_STD) ? rxHeader.StdId : rxHeader.ExtId;
//	g_rx_len = rxHeader.DLC;
//	for (uint32_t i = 0; i < 8; i++) {
//		g_rx_buf[i] = rxData[i];
//	}
//	g_rx_flag = 1;
//}
//
//void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
//	can_error_code = HAL_CAN_GetError(hcan);
//}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
