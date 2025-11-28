/**
 * 5GE-IF4 & 5GEA-IF5 
 * Programme de base pour BO 
 * Ver 2024.1
 * 17/09/2024
 * A. Lelevé
 * 
 * Ce programme doit 
    • attendre 1 seconde ;
    • faire avancer le robot pendant 500 ms ;
    • attendre 500 ms ;
    • faire reculer le robot pendant 500 ms ;
    • l’arrêter définitivement.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"


// A MODIFIER >>>>>>>>>>>>>>>>>>>>>>>>>>
#define PWM_FREQ 36
#define PWM_RESOLUTION 15
 

// CONSTANTES ------------------------------------------------------------------------
const uint8_t PWM_frequence = PWM_FREQ;
const uint8_t PWM_resolution = PWM_RESOLUTION;

//Encod dr
const uint8_t SRA = 35; //bleu
const uint8_t SRB = 34; //violet
//Encod gche
const uint8_t SLA = 14; //bleu ou violet...
const uint8_t SLB = 27; //violet ou bleu...

//Moteur gauche
//Sorties de commande  
const uint8_t MLF = 26; //IN3 Left Forward
const uint8_t MLB = 25; //IN4 Left Backward
 
//Moteur droite
//Sorties de commande 
const uint8_t MRF = 33; //IN2 Right Backward
const uint8_t MRB = 32; //IN1 Right Forward
 

// VARIABLES GLOBALES EVENTUELLES ------------------------------------------------------



// INTERRUPTIONS EVENTUELLES ------------------------------------------------------


   
// FONCTIONS SUPPORT A APPELER DANS VOTRE PROGRAMME  ------------------------------------------------------

/**
 * Initialise une PWM 
 * cf https://github.com/espressif/arduino-esp32/blob/master/docs/en/migration_guides/2.x_to_3.0.rst#ledc
 */
void init_motor_pwm(uint8_t pin) {
  ledcAttach(pin, PWM_FREQ, PWM_RESOLUTION);  
  analogWrite(pin, 0);
}

/**
  * Initialise un codeur roue
 */
 void init_encoder(uint8_t pinA, uint8_t pinB ) {
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);  
 }   


/*
 * initialise le programme
 */
void setup() {

  /*
     Communication Serie (https://techtutorialsx.com/2017/11/13/esp32-arduino-setting-a-socket-server/)  
     Free RTOS (http://tvaira.free.fr/esp32/esp32-freertos.html)
  */

  Serial.begin(115200);
  while (!Serial);
  Serial.println("Setup start : openloop");
 
  // init sorties drivers 2 moteurs
  init_motor_pwm(MLF); // gauche avant
  init_motor_pwm(MLB); // gauche arrière
  init_motor_pwm(MRF); // droite avant
  init_motor_pwm(MRB); // droite arrière
  
  // init encoders
  init_encoder(SLA, SLB); // gauche
  init_encoder(SRA, SRB); // droite

  // Setup() fait partie d'une tâche dont on n'a plus besoin, maintenant que tout est lancé => elle s'endort elle-même à jamais.
  TaskHandle_t setup_task_t = xTaskGetCurrentTaskHandle();
  vTaskSuspend(setup_task_t);
}

/*
 * indispensable pour pouvoir compiler dans l'environnement Arduino même si inutile car setup ne la lance jamais.
 */
// 
void loop() {
    Serial.println("loop function is running !!?? :-(");

}

 
