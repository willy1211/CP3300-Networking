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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
bool transmitting = false;
const char *manchester_start = "1001100110011001";

int message_index = 0;
char *combined_message;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)

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
PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

GETCHAR_PROTOTYPE
{
	uint8_t ch = 0;
	__HAL_UART_CLEAR_OREFLAG(&huart1);
	HAL_UART_Receive(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

void sendBit(char bit) {
	if (bit == '0'){
		HAL_GPIO_WritePin(Tx_GPIO_Port, Tx_Pin, RESET);
	} else if (bit=='1') {
		HAL_GPIO_WritePin(Tx_GPIO_Port, Tx_Pin, SET);
	}
}

// Function to convert string to binary representation

void startTransmission(){
	message_index = 0;
	transmitting = true;
	HAL_TIM_Base_Start_IT(&htim10);

}

void stopTransmission(){
	HAL_TIM_Base_Stop_IT(&htim10);
	HAL_GPIO_WritePin(Tx_GPIO_Port, Tx_Pin, SET);
	transmitting = false;
}

// TIM10 has a 0.5ms period. This will allow us to toggle
// TIM11 is for busy
// TIM9 is for collision
// the output pin properly for the manchester encoding.
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM10){ // Tx timer
  
		if(message_index < strlen(combined_message)){
			sendBit(combined_message[message_index++]);
		} else {
      // message has been sent
			stopTransmission();
		}

	} else if (htim->Instance == TIM9){ // Collison timer

		// if Rx is low, after 1.1 ms, enter collsion state
		if(HAL_GPIO_ReadPin(COLLISION_GPIO_Port, COLLISION_Pin) == GPIO_PIN_RESET){
			HAL_GPIO_WritePin(COLLISION_GPIO_Port, COLLISION_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(BUSY_GPIO_Port, BUSY_Pin, GPIO_PIN_RESET);
		} else {
			HAL_GPIO_WritePin(COLLISION_GPIO_Port, COLLISION_Pin, GPIO_PIN_RESET);
		}
		HAL_TIM_Base_Stop_IT(&htim9);

	} else if (htim->Instance == TIM11){ // idle timer
		// TODO: ADD IDLE LOGIC
		// if after 1.1ms rx is high, enter idle state
		if(HAL_GPIO_ReadPin(Rx_GPIO_Port, Rx_Pin) == GPIO_PIN_SET){
			HAL_GPIO_WritePin(IDLE_GPIO_Port, IDLE_Pin, GPIO_PIN_SET);

			//clear other states @FIXME
			HAL_GPIO_WritePin(BUSY_GPIO_Port, BUSY_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(COLLISION_GPIO_Port, COLLISION_Pin,GPIO_PIN_RESET);

		} else{
			//clear idle state
			HAL_GPIO_WritePin(IDLE_GPIO_Port, IDLE_Pin,GPIO_PIN_RESET);

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
  MX_TIM2_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
  char *manchester_message;
  char *binary_message;
  char message_length_manchester[17];
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  HAL_GPIO_WritePin(Tx_GPIO_Port, Tx_Pin, SET);
	  printf("Please eneter message to send: ");
	  char message[256] = {0};
	  //scanf("%s255", message);
	  HAL_TIM_Base_Start_IT(&htim9);
	  HAL_TIM_Base_Start(&htim8);

	  while (1){};
	  // convert the message length into manchester form.
	  lengthToString(strlen(message), message_length_manchester);

	  binary_message = stringToBinary(message);
	  manchester_message = binaryToManchester(binary_message);

	  int combined_length = strlen(manchester_start) + strlen(message_length_manchester) + strlen(manchester_message);
	  combined_message = malloc((combined_length + 1) * sizeof(char));
	  snprintf(combined_message, (combined_length + 1) * sizeof(char), "%s%s%s", manchester_start, message_length_manchester, manchester_message);

	  startTransmission();
	  HAL_Delay(100);
	  while(transmitting){};
	  free(binary_message);
	  free(manchester_message);

	  printf("Message sent. \n");
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
  RCC_OscInitStruct.PLL.PLLQ = 2;
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
uint32_t last_time = 0;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  // check to see if the interupt was caused by the Rx Pin edge
	// TODO: Impliment BUSY CODE
	// TODO: Implement IDLE CODE
	if(GPIO_Pin == Rx_Pin){
		//@FIXME
		//RX changed -> busy unless proven otherwise
		HAL_GPIO_WritePin(BUSY_GPIO_Port, BUSY_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(IDLE_GPIO_Port, IDLE_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(COLLISION_GPIO_Port, COLLISION_Pin, GPIO_PIN_RESET);

		//Stop both timers since Rx toggled early
		HAL_TIM_Base_Stop_IT(&htim9); //collision
		HAL_TIM_Base_Stop_IT(&htim11); //idle
		__HAL_TIM_SET_COUNTER(&htim9, 0); //reset collision timer count
		__HAL_TIM_SET_COUNTER(&htim11, 0); //reset idle timer count

		//Rising edge --> idle
		if(HAL_GPIO_ReadPin(Rx_GPIO_Port, Rx_Pin)==GPIO_PIN_SET){
			HAL_TIM_Base_Start_IT(&htim11); //start idle timer
		} else { //Falling edge ->collision
			HAL_TIM_Base_Start_IT(&htim9); //start collision timer
		}
		uint32_t current_time = __HAL_TIM_GET_COUNTER(&htim8);
		// 2. Calculate the difference (handling the 16-bit wrap-around)
		uint32_t diff;
		if (current_time >= last_time) {
			diff = current_time - last_time;
		} else {
			// Timer rolled over (reached ARR and started back at 0)
			diff = (htim8.Instance->ARR - last_time) + current_time + 1;
		}
		printf("%d \n", (int)diff);



//		// RISING EDGE
//		if(HAL_GPIO_ReadPin(Rx_GPIO_Port, Rx_Pin) == GPIO_PIN_SET){
//			// start idle timer
//			HAL_TIM_Base_Stop_IT(&htim9); // stop collision timer
//			__HAL_TIM_SET_COUNTER(&htim9, 0); // reset collision timer count register
//			HAL_GPIO_WritePin(COLLISION_GPIO_Port, COLLISION_Pin, GPIO_PIN_RESET);// set collision to low
//
//		} else {
//
//			HAL_TIM_Base_Start_IT(&htim9); // start collision timer
//		}

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
  while (1)
  {
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
