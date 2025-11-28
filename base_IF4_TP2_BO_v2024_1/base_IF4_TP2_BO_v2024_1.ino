/**
 * 5GE-IF4 & 5GEA-IF5
 * 开放回路(BO)基础程序
 * 版本: 2024.1
 * 日期: 2024年9月17日
 * 作者: A. Lelevé
 *
 * **程序目标:**
 * 本程序旨在作为移动机器人开环控制的基础框架,其预期行为如下:
 *   1. 等待 1 秒。
 *   2. 使机器人前进 500 毫秒。
 *   3. 等待 500 毫秒。
 *   4. 使机器人后退 500 毫秒。
 *   5. 最终停止机器人。
 * (注意: 上述核心逻辑需由用户在后续实现中添加,此文件仅为初始化框架)
 */

#include <stdio.h> // 标准输入输出库,用于打印调试信息等
#include "freertos/FreeRTOS.h" // FreeRTOS实时操作系统库
#include "freertos/task.h"     // FreeRTOS任务管理库
#include "sdkconfig.h"         // ESP-IDF SDK配置头文件

// ==============================================================================
// 用户可修改参数
// ==============================================================================
// >>>>>>>>>>>>>>>>>>>>>>>>>> 需要修改的参数 <<<<<<<<<<<<<<<<<<<<<<<<<<<<
#define PWM_FREQ 36        // PWM信号的频率 (赫兹)
#define PWM_RESOLUTION 15  // PWM信号的占空比分辨率 (位), 15位表示2^15个等级

// A MODIFIER >>>>>>>>>>>>>>>>>>>>>>>>>>
#define PWM_FREQ 1000
#define PWM_RESOLUTION 15
 

// --- 编码器引脚定义 ---
// 右侧编码器
const uint8_t SRA = 35; // 右侧编码器A相引脚 (蓝色线)
const uint8_t SRB = 34; // 右侧编码器B相引脚 (紫色线)
// 左侧编码器
const uint8_t SLA = 14; // 左侧编码器A相引脚 (蓝色或紫色线)
const uint8_t SLB = 27; // 左侧编码器B相引脚 (紫色或蓝色线)

// --- 电机驱动器引脚定义 (L298N或其他双H桥驱动器) ---
// 左侧电机
const uint8_t MLF = 26; // 左侧电机前进 (IN3 - Left Forward)
const uint8_t MLB = 25; // 左侧电机后退 (IN4 - Left Backward)

// 右侧电机
const uint8_t MRF = 33; // 右侧电机前进 (IN2 - Right Forward)
const uint8_t MRB = 32; // 右侧电机后退 (IN1 - Right Backward)

// ==============================================================================
// 全局变量 (待扩展)
// ==============================================================================
// 如果程序需要共享的全局变量,可以在这里声明。

// ==============================================================================
// 中断服务程序 (待扩展)
// ==============================================================================
// 如果需要使用中断来处理编码器或其他传感器数据,可以在这里定义中断服务函数。

// ==============================================================================
// 支持函数
// ==============================================================================

/**
 * @brief 初始化一个PWM输出引脚。
 *
 * 此函数配置指定的GPIO引脚以生成PWM信号。它设置了PWM的频率和分辨率,
 * 并将初始占空比设置为0,即电机停止。
 *
 * @param pin 要配置为PWM输出的GPIO引脚编号。
 * @see https://github.com/espressif/arduino-esp32/blob/master/docs/en/migration_guides/2.x_to_3.0.rst#ledc
 */
void init_motor_pwm(uint8_t pin) {
  ledcAttach(pin, PWM_FREQ, PWM_RESOLUTION); // 将PWM通道绑定到引脚,并设置频率和分辨率
  analogWrite(pin, 0); // 将PWM占空比设置为0,确保电机初始停止
}

/**
 * @brief 初始化一个编码器输入引脚。
 *
 * 此函数将编码器的两个相(A相和B相)的引脚配置为输入,并启用内部上拉电阻。
 * 内部上拉电阻确保了当没有外部信号时,引脚处于高电平状态,防止浮空。
 *
 * @param pinA 编码器A相的GPIO引脚编号。
 * @param pinB 编码器B相的GPIO引脚编号。
 */
void init_encoder(uint8_t pinA, uint8_t pinB ) {
  pinMode(pinA, INPUT_PULLUP); // 配置A相引脚为输入并启用上拉
  pinMode(pinB, INPUT_PULLUP); // 配置B相引脚为输入并启用上拉
 }

// ==============================================================================
// Arduino核心函数
// ==============================================================================

/**
 * @brief 程序初始化函数。
 *
 * 这是Arduino程序的入口点,只在程序启动时执行一次。它负责初始化所有
 * 硬件组件和通信接口。
 */
void setup() {
  /*
     串行通信参考: https://techtutorialsx.com/2017/11/13/esp32-arduino-setting-a-socket-server/
     FreeRTOS参考: http://tvaira.free.fr/esp32/esp32-freertos.html
  */

  Serial.begin(115200); // 初始化串行通信,波特率为115200
  while (!Serial);      // 等待串行端口连接 (仅在某些板卡和IDE设置下需要)
  Serial.println("Setup start : openloop"); // 打印启动信息,指示进入开环设置

  // 初始化两个电机的驱动器输出 (PWM引脚)
  init_motor_pwm(MLF); // 初始化左侧电机前进引脚
  init_motor_pwm(MLB); // 初始化左侧电机后退引脚
  init_motor_pwm(MRF); // 初始化右侧电机前进引脚
  init_motor_pwm(MRB); // 初始化右侧电机后退引脚

  // 初始化编码器输入引脚
  init_encoder(SLA, SLB); // 初始化左侧编码器引脚
  init_encoder(SRA, SRB); // 初始化右侧编码器引脚

  // setup()函数是作为FreeRTOS任务的一部分运行的。
  // 一旦所有初始化完成,这个任务就不再需要了,因此它将自身挂起。
  // 这意味着主要的程序逻辑将由其他FreeRTOS任务处理,而不是由setup()或loop()直接执行。
  TaskHandle_t setup_task_t = xTaskGetCurrentTaskHandle(); // 获取当前setup任务的句柄
  vTaskSuspend(setup_task_t); // 挂起当前setup任务,使其永远不再运行
}

/**
 * @brief 程序主循环函数。
 *
 * 在Arduino环境中,此函数通常在`setup()`之后重复执行。但在FreeRTOS
 * 驱动的ESP32应用程序中,如果`setup()`任务被挂起,并且没有其他任务
 * 被显式创建和启动,`loop()`可能不会被执行,或者其执行行为可能不确定。
 *
 * 在本程序中,由于`setup()`任务已将自身挂起,`loop()`函数理论上不应该被调用。
 * 如果它被调用,则表示程序流程可能不符合预期。
 */
void loop() {
    Serial.println("loop function is running !!?? :-( "); // 打印警告信息,表明loop函数意外运行
    // 通常,在一个基于FreeRTOS的应用程序中,一旦所有FreeRTOS任务启动,
    // loop()函数要么被留空,要么被移除,因为任务将接管执行流程。
    // 如果此消息被打印,可能表示FreeRTOS任务未能正确启动或存在其他问题。
}
