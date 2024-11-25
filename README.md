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

Nous avons utilisé le mode **Center Aligned Mode 1** pour générer des signaux PWM déphasés de \( T/2 \). Ce mode permet de synchroniser les signaux PWM en ajustant la position des fronts montants et descendants de manière symétrique par rapport au centre de la période \( T \), facilitant ainsi la génération des déphasages souhaités.


### Affichage sur l'oscilloscope;

![tek00005](https://github.com/user-attachments/assets/7f763cc3-21b2-4325-bcd3-d457672c579a)


**temp mort**

![WhatsApp Image 2024-11-10 at 23 30 16 (4)](https://github.com/user-attachments/assets/9d382d9c-046c-437c-bd73-f88c91812ace)

On a 100 ns de temp mort comme indiqué dans la photo

**On peut Remarquer que notre PWM à 20KHZ et de taille de 12 bits puisque notre ARR est de 4249** 


## Implement Speed Control Function:

### Remarque

Tous les fonctions coder sont dans le shell.c

![image](https://github.com/user-attachments/assets/c81dcf9d-0823-4eab-9fe1-2cb77fabdc8f)

#### SPEED 600:

![tek00006](https://github.com/user-attachments/assets/32a44729-d139-4b01-9780-dbadf6a7b399)

#### SPEED 200:

![tek00005](https://github.com/user-attachments/assets/7f763cc3-21b2-4325-bcd3-d457672c579a)


### Premiers tests

**Rapport cyclique de 50%:**

![image](https://github.com/user-attachments/assets/47902732-debd-49c1-93e5-7f705805d88d)

**Rapport cyclique de 70%:**

![image](https://github.com/user-attachments/assets/6d21f4cc-4933-4f6f-938e-cdb3d77f81e8)

#### Remarque:

Le problème observé se manifeste lors du changement du rapport cyclique de 50 % à 70 %, provoquant une montée brusque qui pourrait endommager le moteur ou la carte d'acquisition du projet.

## TP2

# Objectif

- Commande start : permet de fixer le rapport cyclique à 50% (vitesse nulle) et d'activer la génération des pwm (HAL_TIM_PWM_Start et HAL_TIMEx_PWMN_Start),
- Commande stop : permet de désactiver la génération des PWM.
- Commande speed XXXX : permet de définir le rapport cyclique à XXXX/PWM_MAX, mais afin de réduire l'appel à courant, vous devez établir une montée progressive à cette vitesse en quelques secondes. Vous pouvez effectuer une rampe entre la valeur actuelle et la valeur 4-4-cible avec un incrément bien réfléchi de la PWM à un intervalle de temps régulier. Par la suite votre asservissement fera cela tout seul.

![image](https://github.com/user-attachments/assets/a806a30b-4816-44bc-984b-0dcc3df06941)

On peut remarquer que la détection des commandes ainsi que la gestion des erreurs ont été implémentées, permettant de traiter les commandes incorrectes ou les dépassements de vitesse en dehors de la plage autorisée de 0 % à 100 %.


## 1. Définir les courants à mesurer

Courants de phase du moteur : Courants traversant les phases du moteur. Ces courants sont essentiels pour surveiller la performance et l'état du moteur.

Courant du bus (VBus) : Total consommé par l'ensemble du système (utile pour diagnostiquer les surcharges).

## 2. Définir les fonctions de transfert des capteurs de courant


$$
I_{\text{mesuré}} = \text{Sensibilité} \times (V_{\text{sortie}} - V_{\text{offset}})
$$


Voffset = 1.65V

Sensibilité = 50mV/A

$$
I_{\text{mesuré}} = 0.05 \times (V_{\text{sortie}} - 1.65)
$$


## Pins STM32 utilisés

PA0 : Connecté à ADC2_IN1.

PA1 : Connecté à ADC1_IN2.

PB0 : Connecté à ADC1_IN15.

## Parametrage de L'ADC pour le Polling

![image](https://github.com/user-attachments/assets/d8b288ca-c65c-4a1b-b4f3-1effe1e39e44)

## Première mesure avec ADC en Polling

```c

/* Mesure de courant avec ADC en mode Polling */
HAL_ADC_Start(&hadc1); // Démarrer l'ADC
if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK)
{
    uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
    float current = (adc_value * 3.3 / 4096 - 1.65) / 0.05; 
    printf("Current: %.2f A\r\n", current);
}
HAL_ADC_Stop(&hadc1);

```
Equation Float Curent:

-adc_value est la valeur capter de l'adc entre 0 et 4096

-4096 sont les 12 bits du ADC 

-Offset de 1.65V 

-50mV de sensibilité

-3.3V alimentation du STM32

### Polling:

![image](https://github.com/user-attachments/assets/b226b534-7d70-46fb-a99e-06d89a64d02e)

## Parametrage de L'ADC pour le DMA
![image](https://github.com/user-attachments/assets/4133d95f-e7d6-4671-b329-3b51f33bad84)

![image](https://github.com/user-attachments/assets/1b504c01-01af-49e5-94a6-141830313525)

Nous avons conservé le même paramétrage en mode Polling, tout en ajoutant le **Timer 1 Trigger Output Event** pour générer des événements de synchronisation, ainsi que le paramétrage du **DMA** pour automatiser le transfert des données.

### DMA
![image](https://github.com/user-attachments/assets/48eeba79-0ef0-4935-b0c6-686ea444cd86)

Notre **DMA** est synchronisé avec les PWM, ce qui permet de démarrer le moteur et d'effectuer les mesures de courant. 

Il est également possible de constater une variation significative des valeurs en modifiant la vitesse du moteur. Les tests ont été réalisés avec le moteur alimenté et contrôlé via le shell.


