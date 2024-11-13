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

![image](https://github.com/user-attachments/assets/925e4530-36b6-4f70-bfd5-cbdde80f60b4)


![image](https://github.com/user-attachments/assets/910905ab-f8c8-4dd9-a80b-ec3a33059a13)


![image](https://github.com/user-attachments/assets/d9921aea-ffcd-4d8b-a18b-f21fb6a8bed7)


![image](https://github.com/user-attachments/assets/d7c1c816-c5dd-4e8c-b8aa-2943b5575536)

Calcul temp_mort:

> 1/170MHZ = 5.88ns = 1TIC

> 100ns/5.88 ns = 17 TIC 


### Affichage sur l'oscilloscope;

![tek00005](https://github.com/user-attachments/assets/7f763cc3-21b2-4325-bcd3-d457672c579a)


**temp mort**

![WhatsApp Image 2024-11-10 at 23 30 16 (4)](https://github.com/user-attachments/assets/9d382d9c-046c-437c-bd73-f88c91812ace)


**On peut Remarquer que notre PWM à 20KHZ et de taille de 12 bits puisque notre ARR est de 4249** 


**Implement Speed Control Function:**

![image](https://github.com/user-attachments/assets/c81dcf9d-0823-4eab-9fe1-2cb77fabdc8f)

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
