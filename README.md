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


### Affichage sur l'oscilloscope;
![tek00004](https://github.com/user-attachments/assets/9f549d6f-244b-4358-a0dd-10497bd9d37a)

![WhatsApp Image 2024-11-10 at 23 30 16 (4)](https://github.com/user-attachments/assets/9d382d9c-046c-437c-bd73-f88c91812ace)

![WhatsApp Image 2024-11-10 at 23 30 16 (3)](https://github.com/user-attachments/assets/7bf31acd-527b-4dc9-bfb2-1a637a3d927e)


On peut Remarquer que notre PWM à 20KHZ et de taille de 12 bits puisque notre ARR est de 4249 


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
