/**
 * 5GE-IF4 & 5GEA-IF5 
 * Programme de base pour BF avec comm wifi 
 * Ver 2024.1
 * 17/09/2024
 * A. Lelevé
 */


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "ESP32Encoder.h"
#include "http_server.hpp"

// A MODIFIER >>>>>>>>>>>>>>>>>>>>>>>>>>
#define PWM_FREQ 36
#define PWM_RESOLUTION 15
#define WIFI_NOM_ROBOT "PouetPouet"
#define WIFI_MOT_DE_PASSE "123456789" // choose only numeric password
#define CONSIGNE_VITESSE (10.0f) 

// identification robot
#define TAU (30.20f)
#define G (50.01f)

// Période asservissement en ms
#define PERIODE_MS 20000

// A MODIFIER <<<<<<<<<<<<<<<<<<<<<<<<<<<<


// PARAMETRES ROBOT ------------------------------
#define REDUCTION_RATIO (53.0f)
#define NBR_FRONT (12.0f)

// params correcteur continu:
#define TAU_DEZ (TAU)
#define KP  (TAU/G/TAU_DEZ)
#define TI (TAU)

// params correcteur discret:
#define TE (PERIODE_MS*0.001f)
#define R0 (KP*(1+TE/TI))
#define R1 (-KP)

// params encodeurs
#define REDUCTION_RATIO (53.0f)
#define NBR_FRONT (12.0f)
#define DELTA_POS ( 2.0f * PI / (NBR_FRONT * REDUCTION_RATIO) / TE)


// CONSTANTES ------------------------------------------------------------------------
const uint8_t PWM_frequence  = PWM_FREQ;
const uint8_t PWM_resolution = PWM_RESOLUTION;

const char* ssid     = WIFI_NOM_ROBOT;
const char* password =  WIFI_MOT_DE_PASSE; // choose only numeric password

//Encod droit
uint8_t SRA = 35; //bleu
uint8_t SRB = 34; //violet
//Encod gauche
uint8_t SLA = 14; //bleu ou violet...
uint8_t SLB = 27; //violet ou bleu...

//Moteur droite
// inversion pour que le sens soit >0 en marche avant 
uint8_t MRF = 33; //IN2 Right Backward
uint8_t MRB = 32; //IN1 Right Forward

//Moteur gauche
uint8_t MLF = 26; //IN3 Left Forward
uint8_t MLB = 25; //IN4 Left Backward

// VARIABLES GLOBALES ------------------

ESP32Encoder encodeur_gauche;
ESP32Encoder encodeur_droit;


// FONCTIONS SUPPORT ------------------------------------------------------



// INTERRUPTIONS ------------------------------------------------------



/**
 * Initialise une PWM 
 *  cf https://github.com/espressif/arduino-esp32/blob/master/docs/en/migration_guides/2.x_to_3.0.rst#ledc
 */
void init_motor_pwm(uint8_t pin) { //, uint8_t channel
  ledcAttach(pin, PWM_FREQ, PWM_RESOLUTION);  
  ledcWrite(pin, 0);
}

/**
 * Initialise le programme
 */
void setup() {

  /*
     Communication Serie et wifi (https://techtutorialsx.com/2017/11/13/esp32-arduino-setting-a-socket-server/)
  */

  // Start serial comm
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Setup start");

  // config Wi-Fi 
  wifi_start(ssid, password);
  Serial.print("[INFO] WiFi started with ssid " + String(ssid) + " mdp: "+ String(password));
      
  // init 2 motors
  init_motor_pwm(MLF);//, motor_left_channel_forward);
  init_motor_pwm(MLB);//, motor_left_channel_backward);  
  init_motor_pwm(MRF);//, motor_right_channel_forward);
  init_motor_pwm(MRB);//, motor_right_channel_backward); 


  // Encoders Config
  encodeur_gauche.attachFullQuad(SLA, SLB);  
  encodeur_droit.attachFullQuad( SRA, SRB);  
  encodeur_gauche.clearCount();
  encodeur_droit.clearCount();
   
   // Setup() fait partie d'une tâche dont on n'a plus besoin, maintenant que tout est lancé => elle s'autodétruit. 
  Serial.print("[INFO] Startup finished\n");
  TaskHandle_t setup_task_t = xTaskGetCurrentTaskHandle();
  vTaskDelete(setup_task_t); 
}

/*
 * indispensable pour pouvoir compiler même si inutile car setup ne se termine jamais.
 */
 void loop() {    
}
 

 
