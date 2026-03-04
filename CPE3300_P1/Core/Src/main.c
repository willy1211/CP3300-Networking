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
#include "rng.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "transmitter.h"
#include "receiver.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
bool transmitting = false;
const char *manchester_start = "1001100110011001";
volatile bool print_rx_message = false;
volatile bool recieving_message = false;
int message_index = 0;
char *combined_message;
volatile bool start_of_transmission = true;
char *manchester_message;
char *binary_message;
char message_length_manchester[17];

typedef enum {
	IDLE, BUSY, COLLISION,
} state_t;

volatile state_t current_state;

void setState(state_t new_state) {
	current_state = new_state;

	switch (new_state) {
	case IDLE:
		HAL_GPIO_WritePin(IDLE_GPIO_Port, IDLE_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BUSY_GPIO_Port, BUSY_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(COLLISION_GPIO_Port, COLLISION_Pin, GPIO_PIN_RESET);
		break;

	case BUSY:
		HAL_GPIO_WritePin(IDLE_GPIO_Port, IDLE_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BUSY_GPIO_Port, BUSY_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(COLLISION_GPIO_Port, COLLISION_Pin, GPIO_PIN_RESET);
		break;

	case COLLISION:
		HAL_GPIO_WritePin(IDLE_GPIO_Port, IDLE_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BUSY_GPIO_Port, BUSY_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(COLLISION_GPIO_Port, COLLISION_Pin, GPIO_PIN_SET);
		break;
	}
}

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)

void random_wait(){
	uint32_t random32bit  = 0;
	HAL_RNG_GenerateRandomNumber(&hrng, &random32bit);
	float random_value = (float)random32bit / 0xFFFFFFFF;
	int random_wait_ms = (int)(random_value * 1000);
	HAL_Delay(random_wait_ms);
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
PUTCHAR_PROTOTYPE {
	HAL_UART_Transmit(&huart1, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	return ch;
}

GETCHAR_PROTOTYPE {
	uint8_t ch = 0;
	__HAL_UART_CLEAR_OREFLAG(&huart1);
	HAL_UART_Receive(&huart1, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	return ch;
}

void sendBit(char bit) {
	if (bit == '0') {
		HAL_GPIO_WritePin(Tx_GPIO_Port, Tx_Pin, RESET);
	} else if (bit == '1') {
		HAL_GPIO_WritePin(Tx_GPIO_Port, Tx_Pin, SET);
	}
}

volatile uint16_t rx_index = 0;
volatile char rx_message[2500] = { '0' };
// Function to convert string to binary representation

void startTransmission() {
	message_index = 0;
	transmitting = true;
	start_of_transmission = true;
	HAL_TIM_Base_Start_IT(&htim10);

}

void stopTransmission() {
	HAL_TIM_Base_Stop_IT(&htim10);
	HAL_GPIO_WritePin(Tx_GPIO_Port, Tx_Pin, SET);
	transmitting = false;
}

// TIM10 has a 0.5ms period. This will allow us to toggle
// TIM11 is for busy
// TIM9 is for collision
// the output pin properly for the manchester encoding.
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM10) { // Tx timer

		if (message_index < strlen(combined_message)) {
			sendBit(combined_message[message_index++]);
			setState(BUSY); //Busy state while transmitting
		} else {
			// message has been sent
			stopTransmission();
			setState(IDLE);
		}
	} else if (htim->Instance == TIM9) { // Collison timer

		// if Rx is low, after 1.1 ms, enter collsion state
		if (HAL_GPIO_ReadPin(Rx_GPIO_Port, Rx_Pin)
				== GPIO_PIN_RESET) {
			setState(COLLISION);
			rx_index = 0;

		} else {
			//HAL_GPIO_WritePin(COLLISION_GPIO_Port, COLLISION_Pin, GPIO_PIN_RESET);
			setState(BUSY);
		}
		HAL_TIM_Base_Stop_IT(&htim9);

	} else if (htim->Instance == TIM11) { // idle timer
		// TODO: ADD IDLE LOGIC
		// if after 1.1ms rx is high, enter idle state
		if (HAL_GPIO_ReadPin(Rx_GPIO_Port, Rx_Pin) == GPIO_PIN_SET) {
			if (recieving_message == true) {
				print_rx_message = true;
				start_of_transmission = true;
			}
			setState(IDLE);
		} else {
			//clear idle state
			//HAL_GPIO_WritePin(IDLE_GPIO_Port, IDLE_Pin,GPIO_PIN_RESET);
			setState(BUSY);
		}
		HAL_TIM_Base_Stop_IT(&htim11);
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	setvbuf(stdin, NULL, _IONBF, 0);
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
  MX_USART1_UART_Init();
  MX_TIM10_Init();
  MX_TIM9_Init();
  MX_TIM11_Init();
  MX_TIM8_Init();
  MX_RNG_Init();
  /* USER CODE BEGIN 2 */

	HAL_GPIO_WritePin(Tx_GPIO_Port, Tx_Pin, SET);
	printf("Listening for a message... \n");
	char message_to_print[255] = { 'F' };
	char message_to_send_buffer[255] = { '0' };
	int message_to_send_index = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		char entered_char = 0;
		__HAL_UART_CLEAR_OREFLAG(&huart1);
		// to keep this method non-blocking the character
		// has to be read from the UART signal one by one.
		// The HAL_UART_Receive function will return a status of HAL_OK if and only if a
		// character is retrieved wihtin the timput.
		// if the character entered is \n or \r the message is sent
		if (HAL_UART_Receive(&huart1, (uint8_t*) &entered_char, 1, 20)
				== HAL_OK) {
			// TODO: only send messages that aren't empty
			if (entered_char == '\n' || entered_char == '\r') {
				message_to_send_buffer[message_to_send_index] = '\0';
				printf("Sending message %s\n", message_to_send_buffer);
				message_to_send_index = 0;

				// convert the message length into manchester form.
				lengthToString(strlen(message_to_send_buffer), message_length_manchester);

				binary_message = stringToBinary(message_to_send_buffer);
				manchester_message = binaryToManchester(binary_message);

				int combined_length = strlen(manchester_start)
						+ strlen(message_length_manchester)
						+ strlen(manchester_message);
				combined_message = malloc((combined_length + 1) * sizeof(char));
				snprintf(combined_message, (combined_length + 1) * sizeof(char),
						"%s%s%s", manchester_start, message_length_manchester,
						manchester_message);
				startTransmission();

			} else {
				message_to_send_buffer[message_to_send_index++] = entered_char;
			}


		} else if (print_rx_message == true && rx_index > 8) {

			rx_message[rx_index + 1] = '\0';
			decodeBinary(rx_message, message_to_print);
			printf("Received Message: %s\n", message_to_print);
			print_rx_message = false;
			recieving_message = false;
			rx_index = 0;
			printf("Enter message to send: \n");
		} else if (current_state == COLLISION){
			// generate random number
			// set timer to wait radnom amount of time
			// use timer IT callback to restart transmission
			rx_index = 0;
			stopTransmission();
			message_index = 0;
			random_wait();
			startTransmission();

		}
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 10;
  RCC_OscInitStruct.PLL.PLLN = 210;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 10;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
volatile bool skip_edge = true;

// 0 for falling
// 1 for rising edge
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	// check to see if the interupt was caused by the Rx Pin edge
	// TODO: Impliment BUSY CODE
	// TODO: Implement IDLE CODE
	if (GPIO_Pin == Rx_Pin) {

		GPIO_PinState rx_value = HAL_GPIO_ReadPin(Rx_GPIO_Port, Rx_Pin);

		HAL_TIM_Base_Stop_IT(&htim11); //idle
		HAL_TIM_Base_Stop_IT(&htim9);

		//Rising edge --> idle
		if (rx_value == GPIO_PIN_SET) {

			__HAL_TIM_SET_COUNTER(&htim11, 0);
			HAL_TIM_Base_Start_IT(&htim11);
		} else { //Falling edge ->collision
			__HAL_TIM_SET_COUNTER(&htim9, 0);
			HAL_TIM_Base_Start_IT(&htim9);
		}


		setState(BUSY);
		recieving_message = true;
		if (start_of_transmission == true) {
			__HAL_TIM_SET_COUNTER(&htim8, 0);
			HAL_TIM_Base_Start(&htim8);
			start_of_transmission = false;
			return;
		}

		uint16_t time_elapsed = __HAL_TIM_GET_COUNTER(&htim8);

		if (time_elapsed < 750) {
			if (skip_edge == true) {
				skip_edge = false;
			} else {
				if (rx_value == GPIO_PIN_SET) {
					rx_message[++rx_index] = '1';
				} else {
					rx_message[++rx_index] = '0';
				}
				skip_edge = true;
			}
		} else {
			if (rx_value == GPIO_PIN_SET) {
				rx_message[++rx_index] = '1';
			} else {
				rx_message[++rx_index] = '0';

			}
			skip_edge = true;
			//rx_message[rx_index++] = HAL_GPIO_ReadPin(Rx_GPIO_Port, Rx_Pin);

		}
		__HAL_TIM_SET_COUNTER(&htim8, 0);
		HAL_TIM_Base_Start(&htim8);
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
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
