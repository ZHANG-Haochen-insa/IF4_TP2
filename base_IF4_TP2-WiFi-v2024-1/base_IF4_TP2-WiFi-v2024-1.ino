/**
 * 5GE-IF4 & 5GEA-IF5
 * 带WiFi通信的闭环控制(BF)基础程序
 * 版本: 2024.1
 * 日期: 2024年9月17日
 * 作者: A. Lelevé
 *
 * **中文注释:**
 * 此程序是用于ESP32机器人的一个框架，旨在实现一个通过WiFi远程遥控的闭环速度控制系统。
 * 它包含了硬件初始化、WiFi热点启动以及控制参数的定义，但核心的控制逻辑和WiFi通信处理
 * 任务需要用户在此基础上自行实现。
 */

// ==============================================================================
// 包含的头文件
// ==============================================================================
#include <stdio.h>               // 标准输入输出库
#include "freertos/FreeRTOS.h"     // FreeRTOS实时操作系统核心库
#include "freertos/task.h"         // FreeRTOS任务管理库
#include "sdkconfig.h"             // ESP-IDF SDK配置文件

#include "ESP32Encoder.h"          // ESP32编码器读取库
#include "http_server.hpp"         // 自定义的HTTP服务器，用于WiFi通信

// ==============================================================================
// 用户可修改参数
// ==============================================================================
// >>>>>>>>>>>>>>>>>>>>>>>>>> 需要修改的参数 <<<<<<<<<<<<<<<<<<<<<<<<<<<<
#define PWM_FREQ 1000              // PWM信号的频率 (赫兹)
#define PWM_RESOLUTION 15          // PWM信号的占空比分辨率 (位)
#define WIFI_NOM_ROBOT "PouetPouet" // WiFi热点的SSID (网络名称)
#define WIFI_MOT_DE_PASSE "123456789" // WiFi热点的密码 (仅支持数字)
#define CONSIGNE_VITESSE (10.0f)   // 期望速度的常量 (单位: rad/s), 可能用于测试

// --- 机器人模型辨识参数 ---
// 这些参数通常通过系统辨识实验获得，用于设计控制器
#define TAU (30.20f)               // 系统时间常数
#define G (50.01f)                 // 系统静态增益

// --- 控制器参数 ---
#define PERIODE_MS 20000           // 速度控制(asservissement)任务的周期 (毫秒) - 注意: 这个值(20秒)看起来非常大, 可能是一个占位符或需要修正为更小的值(如20ms)

// <<<<<<<<<<<<<<<<<<<<<<<<<<<< 结束修改区域 <<<<<<<<<<<<<<<<<<<<<<<<<<<<

// ==============================================================================
// 机器人与控制器固定参数
// ==============================================================================

// --- 机器人物理参数 ---
#define REDUCTION_RATIO (53.0f)    // 电机减速比
#define NBR_FRONT (12.0f)          // 编码器每转产生的脉冲数 (或边沿数)

// --- 连续PI控制器参数计算 ---
// 基于辨识出的模型参数来计算连续域的PI控制器参数
#define TAU_DEZ (TAU)              //
#define KP  (TAU/G/TAU_DEZ)        // 比例增益 (Proportional Gain)
#define TI (TAU)                   // 积分时间 (Integral Time)

// --- 离散PI控制器参数计算 ---
// 将连续控制器参数通过离散化方法(如后向欧拉法)转换为离散域的控制器参数
#define TE (PERIODE_MS*0.001f)     // 控制周期 (秒)
#define R0 (KP*(1+TE/TI))          // 离散PI控制器参数 R0
#define R1 (-KP)                   // 离散PI控制器参数 R1

// --- 编码器计算参数 ---
// 用于将编码器读数转换为物理单位(如 rad/s)
#define DELTA_POS ( 2.0f * PI / (NBR_FRONT * REDUCTION_RATIO) / TE) // 每个控制周期内,编码器每跳动一格代表的角速度变化量

// ==============================================================================
// 常量定义
// ==============================================================================
const uint8_t PWM_frequence  = PWM_FREQ;       // PWM频率
const uint8_t PWM_resolution = PWM_RESOLUTION; // PWM分辨率

const char* ssid     = WIFI_NOM_ROBOT;     // WiFi SSID
const char* password =  WIFI_MOT_DE_PASSE; // WiFi 密码

// --- 硬件引脚定义 ---
// 右侧编码器
uint8_t SRA = 35; // A相 (蓝色)
uint8_t SRB = 34; // B相 (紫色)
// 左侧编码器
uint8_t SLA = 14; // A相
uint8_t SLB = 27; // B相

// 右侧电机 (引脚已反转以确保正指令对应前进方向)
uint8_t MRF = 33; // IN2 Right
uint8_t MRB = 32; // IN1 Right

// 左侧电机
uint8_t MLF = 26; // IN3 Left Forward
uint8_t MLB = 25; // IN4 Left Backward

// ==============================================================================
// 全局变量
// ==============================================================================
ESP32Encoder encodeur_gauche; // 左轮编码器对象
ESP32Encoder encodeur_droit;  // 右轮编码器对象

// ==============================================================================
// 支持函数与中断
// ==============================================================================
// INTERRUPTIONS (中断服务程序区, 待添加)

/**
 * @brief 初始化一个PWM输出引脚。
 *
 * @param pin 要配置为PWM输出的GPIO引脚编号。
 */
void init_motor_pwm(uint8_t pin) {
  ledcAttach(pin, PWM_FREQ, PWM_RESOLUTION); // 绑定引脚到PWM通道，并设置频率和分辨率
  ledcWrite(pin, 0); // 初始占空比设为0，电机停止
}

// ==============================================================================
// Arduino核心函数
// ==============================================================================

/**
 * @brief 程序初始化函数。
 */
void setup() {

  /*
     参考资料:
     - 串行通信与WiFi: https://techtutorialsx.com/2017/11/13/esp32-arduino-setting-a-socket-server/
  */

  // 启动串行通信
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Setup start");

  // 配置并启动Wi-Fi热点
  wifi_start(ssid, password); // 调用http_server.cpp中定义的函数
  Serial.print("[INFO] WiFi started with ssid " + String(ssid) + " mdp: "+ String(password));

  // 初始化两个电机的PWM输出
  init_motor_pwm(MLF);
  init_motor_pwm(MLB);
  init_motor_pwm(MRF);
  init_motor_pwm(MRB);

  // 配置编码器
  // 使用正交解码模式(FullQuad)来捕获两个相的上升沿和下降沿，提供最高分辨率
  encodeur_gauche.attachFullQuad(SLA, SLB);
  encodeur_droit.attachFullQuad( SRA, SRB);
  // 清除编码器的初始计数值
  encodeur_gauche.clearCount();
  encodeur_droit.clearCount();

   // Setup()是作为一个任务运行的。当所有初始化完成后，这个任务就不再需要了。
   // 因此，它会删除自身以释放资源。
  Serial.print("[INFO] Startup finished\n");
  TaskHandle_t setup_task_t = xTaskGetCurrentTaskHandle(); // 获取当前任务的句柄
  vTaskDelete(setup_task_t); // 删除当前任务
}

/**
 * @brief 程序主循环函数。
 *
 * 由于`setup()`任务在完成时删除了自身，`loop()`函数实际上永远不会被执行。
 * 所有的程序逻辑都应该在由`setup()`或其他任务创建的独立FreeRTOS任务中运行。
 */
 void loop() {
    // 此处不应有任何代码
}
