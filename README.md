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

![image](https://github.com/user-attachments/assets/47902732-debd-49c1-93e5-7f705805d88d)
![image](https://github.com/user-attachments/assets/6d21f4cc-4933-4f6f-938e-cdb3d77f81e8)


## TP2

# Objectif

- Commande start : permet de fixer le rapport cyclique à 50% (vitesse nulle) et d'activer la génération des pwm (HAL_TIM_PWM_Start et HAL_TIMEx_PWMN_Start),
- Commande stop : permet de désactiver la génération des PWM.
- Commande speed XXXX : permet de définir le rapport cyclique à XXXX/PWM_MAX, mais afin de réduire l'appel à courant, vous devez établir une montée progressive à cette vitesse en quelques secondes. Vous pouvez effectuer une rampe entre la valeur actuelle et la valeur 4-4-cible avec un incrément bien réfléchi de la PWM à un intervalle de temps régulier. Par la suite votre asservissement fera cela tout seul.

![image](https://github.com/user-attachments/assets/a806a30b-4816-44bc-984b-0dcc3df06941)

sensibilité = 50mV/A
convertisseur sur 12 bits

**Implémenter la lecture ADC en Polling**
```c
uint32_t Read_Current(void) {
    uint32_t adcValue = 0;
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
        adcValue = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    return adcValue;
}
float Convert_To_Current(uint32_t adcValue) {
    float voltage = (adcValue * 3.3f) / 4095; // Conversion en tension
    float current = (voltage - 1.65f) / 0.066f; // Exemple pour un capteur à effet Hall (sensibilité : 66 mV/A)
    return current;
}
```

1. Définir les courants à mesurer

Courants de phase du moteur : Courants traversant les phases du moteur. Ces courants sont essentiels pour surveiller la performance et l'état du moteur.

Courant du bus (VBus) : Total consommé par l'ensemble du système (utile pour diagnostiquer les surcharges).

Définir les fonctions de transfert des capteurs de courant
Les capteurs de courant (par exemple, effet Hall) possèdent des caractéristiques définies dans leur datasheet. Voici une approche générique :

Plage de mesure : Exemple ±20 A.
Sensibilité : Exemple 100 mV/A.
Offset : Exemple 2,5 V pour 0 A.
Fonction de transfert pour un capteur standard :
𝐼
mesur
e
ˊ
=
𝑉
sortie
−
𝑉
offset
Sensibilit
e
ˊ
I 
mesur 
e
ˊ
 
​
 = 
Sensibilit 
e
ˊ
 
V 
sortie
​
 −V 
offset
​
 
​
 
Exemple pour un ACS712-20A :

𝑉
offset
=
2.5
 
V
V 
offset
​
 =2.5V
Sensibilité = 
100
 
mV/A
100mV/A
𝐼
=
𝑉
sortie
−
2.5
0.1
I= 
0.1
V 
sortie
​
 −2.5
​
 




