/*
 * shell.c
 *
 *  Created on: Oct 1, 2023
 *      Author: nicolas
 */
#include "usart.h"
#include "mylibs/shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tim.h"
#define MAX_SPEED 1000
uint8_t prompt[]="user@Nucleo-STM32G474RET6>>";
uint8_t started[]=
		"\r\n*-----------------------------*"
		"\r\n| Welcome on Nucleo-STM32G474 |"
		"\r\n*-----------------------------*"
		"\r\n";
uint8_t newline[]="\r\n";
uint8_t backspace[]="\b \b";
uint8_t cmdNotFound[]="Command not found\r\n";
uint8_t brian[]="Brian is in the kitchen\r\n";
uint8_t uartRxReceived;
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];

char	 	cmdBuffer[CMD_BUFFER_SIZE];
int 		idx_cmd;
char* 		argv[MAX_ARGS];
int		 	argc = 0;
char*		token;
int 		newCmdReady = 0;
int currentSpeed = 0;  // Vitesse actuelle du moteur

void Shell_Init(void){
	memset(argv, 0, MAX_ARGS * sizeof(char*));
	memset(cmdBuffer, 0, CMD_BUFFER_SIZE * sizeof(char));
	memset(uartRxBuffer, 0, UART_RX_BUFFER_SIZE * sizeof(char));
	memset(uartTxBuffer, 0, UART_TX_BUFFER_SIZE * sizeof(char));

	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_UART_Transmit(&huart2, started, strlen((char *)started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
}
// Fonction pour définir la vitesse via PWM
void SpeedCommand(int speedValue) {
    // Limite la valeur de speed à la vitesse maximale autorisée
    if (speedValue > MAX_SPEED) {
    	//speedValue = 0;
    } else if (speedValue < 0) {

        speedValue = 0; // Assure que la valeur minimale est 0
    }

   /* // Définir le pas d'incrémentation et la durée de la montée progressive
        int step = 10; // Pas d'incrémentation de la vitesse
        int delayMs = 50; // Délai entre chaque incrément en millisecondes

        // Montée progressive vers la vitesse cible
        while (currentSpeed !=speedValue) {
            if (currentSpeed < speedValue) {
                currentSpeed += step;
                if (currentSpeed > speedValue) {
                    currentSpeed = speedValue;
                }
            } else {
                currentSpeed -= step;
                if (currentSpeed < speedValue) {
                    currentSpeed = speedValue;
                }
            }*/
    // Calcule la valeur de PWM correspondante
       uint32_t pulseValue1 = (uint32_t)((speedValue * __HAL_TIM_GET_AUTORELOAD(&htim1)) / MAX_SPEED);
       uint32_t pulseValue2= (uint32_t)(((MAX_SPEED-speedValue) * __HAL_TIM_GET_AUTORELOAD(&htim1)) / MAX_SPEED);
       __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulseValue1);
       __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulseValue2);
       if (speedValue > MAX_SPEED) {
    	   snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "La vitesse a depassé la limite", speedValue);
    	      	       HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
       }else {
       // Envoie une confirmation par UART
       snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "Speed set to %d\r\n", speedValue);
       HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
   }
}

//stop function
void StopMotor(void) {
    // Arrêt du PWM
    HAL_TIM_PWM_Stop_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop_IT(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop_IT(&htim1, TIM_CHANNEL_2);

    snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "Motor stopped\r\n");
    HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);

    // Relancer la réception UART
    HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}

//Start function
void StartMotor(void) {
    uint32_t midDutyCycle = __HAL_TIM_GET_AUTORELOAD(&htim1) / 2;

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, midDutyCycle);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, midDutyCycle);

    // Démarrage du PWM sans arrêter les interruptions
    HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start_IT(&htim1, TIM_CHANNEL_2);

    snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "Motor started with 50%% duty cycle\r\n");
    HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);

    // Relancer la réception UART
    HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}

void Shell_Loop(void){
	if(uartRxReceived){
		switch(uartRxBuffer[0]){
		case ASCII_CR: // Nouvelle ligne, instruction à traiter
			HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
			cmdBuffer[idx_cmd] = '\0';
			argc = 0;
			token = strtok(cmdBuffer, " ");
			while(token!=NULL){
				argv[argc++] = token;
				token = strtok(NULL, " ");
			}
			idx_cmd = 0;
			newCmdReady = 1;
			break;


		case ASCII_BACK: // Suppression du dernier caractère
			cmdBuffer[idx_cmd--] = '\0';
			HAL_UART_Transmit(&huart2, backspace, sizeof(backspace), HAL_MAX_DELAY);
			break;

		default: // Nouveau caractère
			cmdBuffer[idx_cmd++] = uartRxBuffer[0];
			HAL_UART_Transmit(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
		}
		uartRxReceived = 0;
	}


	if (newCmdReady) {
	    if (strcmp(argv[0], "WhereisBrian?") == 0) {
	        HAL_UART_Transmit(&huart2, brian, sizeof(brian), HAL_MAX_DELAY);
	    } else if (strcmp(argv[0], "help") == 0) {
	        int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE,
	                                          "Available commands:\r\n- WhereisBrian?\r\n- help\r\n- start\r\n- stop\r\n- speed <value>\r\n");
	        HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
	    } else if (strcmp(argv[0], "start") == 0) {
	        StartMotor();
	        HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE); // Relancer la réception UART

	    } else if (strcmp(argv[0], "stop") == 0) {
	        StopMotor();
	        HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE); // Relancer la réception UART

	    } else if (strcmp(argv[0], "speed") == 0) {
	        if (argc > 1) {
	            int speedValue = atoi(argv[1]);
	            SpeedCommand(speedValue);
	        } else {
	            snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Usage: speed <value>\r\n");
	            HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
	        }
	    } else {
	        HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
	    }
	    HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
	    newCmdReady = 0;
	}


}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart){
    uartRxReceived = 1;
    HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE); // Relancer la réception UART
}

