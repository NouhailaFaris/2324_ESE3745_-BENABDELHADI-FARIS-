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
#include "adc.h"




#define MAX_SPEED 1000 // Vitesse maximale du moteur
#define SENSITIVITY 0.05 // Sensibilité (50 mV/A)
#define VREF 3.3         // Référence de tension ADC
#define OFFSET 2.5       // Offset en volts
#define ADC_MAX_VALUE 4096.0 // Résolution ADC 12 bits

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
uint16_t pData[ADC_BUFF_SIZE];
uint16_t adc_value = 0;
char	 	cmdBuffer[CMD_BUFFER_SIZE];
int 		idx_cmd;
char* 		argv[MAX_ARGS];
int		 	argc = 0;
char*		token;
int 		newCmdReady = 0;  // Flag pour indiquer si une nouvelle commande est prête
int currentSpeed = 0;  // Vitesse actuelle du moteur
float current=0; // Valeur du courant

/*
 * Initialisation du shell et du périphérique UART
 */

void Shell_Init(void){
	// Initialisation des buffers
	memset(argv, 0, MAX_ARGS * sizeof(char*));
	memset(cmdBuffer, 0, CMD_BUFFER_SIZE * sizeof(char));
	memset(uartRxBuffer, 0, UART_RX_BUFFER_SIZE * sizeof(char));
	memset(uartTxBuffer, 0, UART_TX_BUFFER_SIZE * sizeof(char));
        // Démarre la réception UART en mode interruption
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_UART_Transmit(&huart2, started, strlen((char *)started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
}
// Fonction pour définir la vitesse via PWM
void SpeedCommand(int speedValue) {
	// Limite la valeur de speed à la vitesse maximale autorisée
	if (speedValue > MAX_SPEED) {
	// Si la vitesse est trop élevée, on ne fait rien ou on peut renvoyer un message d'erreur
	} else if (speedValue < 0) {

		speedValue = 0; // Assure que la valeur minimale est 0
	}
	// Calcul des valeurs de comparaison pour le PWM
	uint32_t pulseValue1 = (uint32_t)((speedValue * __HAL_TIM_GET_AUTORELOAD(&htim1)) / MAX_SPEED);
	uint32_t pulseValue2= (uint32_t)(((MAX_SPEED-speedValue) * __HAL_TIM_GET_AUTORELOAD(&htim1)) / MAX_SPEED);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulseValue1);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulseValue2);
	// Envoi d'un message de confirmation via UART
	if (speedValue > MAX_SPEED) {
		snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "La vitesse a depassé la limite", speedValue);
		HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
	}else {
		// Envoie une confirmation par UART
		snprintf((char *)uartTxBuffer, sizeof(uartTxBuffer), "Speed set to %d\r\n", speedValue);
		HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
	}
}
/*
 * Fonction de lecture du courant en utilisant l'ADC
 */
float read_current_polling()
{
	// Démarre la calibration de l'ADC
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); // Calibration
	    HAL_ADC_Start(&hadc1);
	// Attente de la conversion et lecture de la valeur ADC
	    if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK)
	    {
	        adc_value = HAL_ADC_GetValue(&hadc1); // Lire la valeur brute
	        double voltage = (adc_value * VREF) / ADC_MAX_VALUE; // Conversion en tension
	        double current = (voltage - 1.65) / SENSITIVITY;   // Conversion en courant
	// Affichage de la valeur brute et du courant
	        printf("\r\nRAW ADC value: %d\r\n", adc_value);

	        printf("Current: %.3f A\r\n", current);

	        HAL_ADC_Stop(&hadc1);// Arrêt de l'ADC
	        return (float)current;
	    }

	    HAL_ADC_Stop(&hadc1); // Arrêt de l'ADC si la conversion échoue
	    return 0.0f;// Retourne 0 si la lecture échoue

}
/*
 * Callback de conversion ADC complète pour DMA
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        adc_value = pData[0];
        float voltage = (adc_value * VREF) / ADC_MAX_VALUE;
        current = (voltage - 1.65) / SENSITIVITY;
 	// Démarrage de l'ADC en mode DMA
        
        HAL_ADC_Start_DMA(hadc, pData, ADC_BUFF_SIZE);
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

//Start function avec un cycle  de 50%
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
/*
 * Boucle principale pour le shell (traitement des commandes reçues)
 */

void Shell_Loop(void){
	if(uartRxReceived){
		switch(uartRxBuffer[0]){
		case ASCII_CR: // Nouvelle ligne, instruction à traiter
			HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
			cmdBuffer[idx_cmd] = '\0'; // Terminer la commande
			argc = 0;
			token = strtok(cmdBuffer, " ");// Séparer la commande et les arguments
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

	// Traitement des commandes
	if (newCmdReady) {
		if (strcmp(argv[0], "current") == 0) {
			// Afficher la valeur du courant
			snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Current: %.2f A\r\n", current);
			HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
		}
		else if (strcmp(argv[0], "WhereisBrian?") == 0) {
			HAL_UART_Transmit(&huart2, brian, sizeof(brian), HAL_MAX_DELAY);
		} else if (strcmp(argv[0], "help") == 0) {
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE,
					"Available commands:\r\n- WhereisBrian?\r\n- help\r\n- start\r\n- stop\r\n- speed <value>\r\n");
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
		} else if (strcmp(argv[0], "start") == 0) {
			// Si la commande est "start", on démarre le moteur
			StartMotor();
			// Relance la réception UART pour attendre de nouvelles commandes
			HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE); // Relancer la réception UART

		} else if (strcmp(argv[0], "stop") == 0) {
			// Si la commande est "stop", on arrête le moteur
			StopMotor();
			// Relance la réception UART pour attendre de nouvelles commandes
			HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE); // Relancer la réception UART

		} else if (strcmp(argv[0], "speed") == 0) {
			// Si la commande est "speed", on ajuste la vitesse du moteur
			if (argc > 1) {
			// Si un argument est passé (la valeur de la vitesse), on le convertit en entier et on applique la commande de vitesse
				int speedValue = atoi(argv[1]);// Convertir l'argument en entier
				SpeedCommand(speedValue); // Appliquer la commande de vitesse
			} else {
				snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Usage: speed <value>\r\n");
				HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer), HAL_MAX_DELAY);
			}
		} else {
			
			HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
		}
		HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
		// Réinitialisation du flag indiquant qu'une nouvelle commande est prête
		newCmdReady = 0;
	}


}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE); // Relancer la réception UART
}

