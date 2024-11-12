# 2324_ESE3745_-BENABDELHADI-FARIS

# ENSEA - Projet Onduleur Triphasé didactique, 60V, 10A

**Rédaction :Mohamed Benabdelhadi ,Faris Nouhaila**
## Séance 1 - Commande MCC basique
### Objectifs :

- Générer 4 PWM en complémentaire décalée pour contrôler en boucle ouverte le moteur en respectant le cahier des charges.
- Inclure le temps mort,
- Vérifier les signaux de commande à l'oscilloscope,
- Prendre en main le hacheur,
- Faire un premier essai de commande moteur.

### Configuration: 

![WhatsApp Image 2024-11-10 at 23 30 16 (2)](https://github.com/user-attachments/assets/b514a10b-1d94-467c-b1ad-75295994c149)

![WhatsApp Image 2024-11-10 at 23 30 16 (1)](https://github.com/user-attachments/assets/359662c2-6f5b-4d97-bfbe-dd428f59164c)

![WhatsApp Image 2024-11-10 at 23 30 16](https://github.com/user-attachments/assets/aa8efdfd-d0e5-4a8b-87b0-7d24e0c69718)

### Code :
```c
/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mylibs/shell.h"
extern UART_HandleTypeDef huart2;
uint8_t uartRxReceived = 0;
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
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
	MX_ADC2_Init();
	MX_ADC1_Init();
	MX_TIM1_Init();
	MX_TIM3_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	/* USER CODE BEGIN 2 */
	Shell_Init();
	uint32_t pulseValue1 = (htim1.Instance->CCR1);
	uint32_t pulseValue2 = (htim1.Instance->CCR2);

	/* Configuration des PWM sur TIM1 Channel 1 et 2 */
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulseValue1);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulseValue2);

	/* Démarrage des PWM */
	if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		Shell_Loop();
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
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
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
	RCC_OscInitStruct.PLL.PLLN = 85;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

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
```

### Affichage sur l'oscilloscope;
![WhatsApp Image 2024-11-10 at 23 30 16 (4)](https://github.com/user-attachments/assets/9d382d9c-046c-437c-bd73-f88c91812ace)

![WhatsApp Image 2024-11-10 at 23 30 16 (3)](https://github.com/user-attachments/assets/7bf31acd-527b-4dc9-bfb2-1a637a3d927e)


**Define Max Speed and UART Command Handling:**

```c
#define MAX_SPEED 1000  // Define the maximum allowed speed value
#include <stdlib.h>     // For atoi function to convert strings to integers
```

**Implement Speed Control Function:**

```c
void ProcessSpeedCommand(char *cmd) {
    // Check if command starts with "speed "
    if (strncmp(cmd, "speed ", 6) == 0) {
        // Convert the speed value after "speed " to an integer
        int speedValue = atoi(&cmd[6]);

        // Limit the speed value to the maximum allowed speed
        if (speedValue > MAX_SPEED) {
            speedValue = MAX_SPEED;
        }

        // Set the PWM duty cycle according to speedValue
        uint32_t pulseValue = (uint32_t)((speedValue * __HAL_TIM_GET_AUTORELOAD(&htim1)) / MAX_SPEED);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulseValue);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulseValue);

        // Send acknowledgment over UART
        snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "Speed set to %d\r\n", speedValue);
        HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
    } else {
        // Send error message if command is incorrect
        snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "Invalid command\r\n");
        HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
    }
}

while (1) {
    Shell_Loop();

    if (uartRxReceived) {
        uartRxReceived = 0;  // Reset the flag after processing
        ProcessSpeedCommand((char *)uartRxBuffer);  // Process the received command
        HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);  // Re-enable UART receive interrupt
    }
}
```
**Add UART Receive Callback Function**

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {  // Assuming we are using USART2
        uartRxReceived = 1;  // Set flag to indicate data was received
    }
}
```
### Premiers tests

**Rapport cyclique de 50%:**
**Rapport cyclique de 70%:**

## TP2

1-Commande start : permet de fixer le rapport cyclique à 50% (vitesse nulle) et d'activer la génération des pwm (HAL_TIM_PWM_Start et HAL_TIMEx_PWMN_Start),
2-Commande stop : permet de désactiver la génération des PWM.
3-Commande speed XXXX : permet de définir le rapport cyclique à XXXX/PWM_MAX, mais afin de réduire l'appel à courant, vous devez établir une montée progressive à cette vitesse en quelques secondes. Vous pouvez effectuer une rampe entre la valeur actuelle et la valeur 4-4-cible avec un incrément bien réfléchi de la PWM à un intervalle de temps régulier. Par la suite votre asservissement fera cela tout seul.

```c
void ProcessCommand(char *cmd) {
    if (strncmp(cmd, "start", 5) == 0) {
        StartMotor();
    } else if (strncmp(cmd, "stop", 4) == 0) {
        StopMotor();
    } else if (strncmp(cmd, "speed ", 6) == 0) {
        int speedValue = atoi(&cmd[6]);
        SetMotorSpeed(speedValue);
    } else {
        snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "Invalid command\r\n");
        HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
    }
}
```
**Define the StartMotor, StopMotor, and SetMotorSpeed Functions**

```c
void StartMotor(void) {
    uint32_t midDutyCycle = __HAL_TIM_GET_AUTORELOAD(&htim1) / 2;  // 50% duty cycle

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, midDutyCycle);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, midDutyCycle);

    // Start PWM generation on both channels
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK || HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK || HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler();
    }

    snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "Motor started with 50%% duty cycle\r\n");
    HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
}
```
```c
void StopMotor(void) {
    // Stop PWM generation on both channels
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

    snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "Motor stopped\r\n");
    HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
}
```
### ProcessCommand Function: Decides which command to execute based on the input string.
```c
void SetMotorSpeed(int speedValue) {
    // Limit the speed value to the maximum allowed speed
    if (speedValue > MAX_SPEED) {
        speedValue = MAX_SPEED;
    }

    // Calculate target duty cycle based on the speed value
    uint32_t targetDutyCycle = (uint32_t)((speedValue * __HAL_TIM_GET_AUTORELOAD(&htim1)) / MAX_SPEED);

    // Gradually ramp up/down to the target duty cycle
    while (currentDutyCycle != targetDutyCycle) {
        if (currentDutyCycle < targetDutyCycle) {
            currentDutyCycle += rampStepSize;
            if (currentDutyCycle > targetDutyCycle) currentDutyCycle = targetDutyCycle;  // Prevent overshooting
        } else {
            currentDutyCycle -= rampStepSize;
            if (currentDutyCycle < targetDutyCycle) currentDutyCycle = targetDutyCycle;  // Prevent undershooting
        }

        // Apply the current duty cycle
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, currentDutyCycle);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, currentDutyCycle);

        // Delay to allow gradual increase
        HAL_Delay(rampDelay);
    }

    snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "Speed set to %d\r\n", speedValue);
    HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
}
```
