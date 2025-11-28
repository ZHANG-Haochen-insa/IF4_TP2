/**
 * 5GE-IF4 & 5GEA-IF5
 * 速度测量程序 (使用中断)
 * 版本: 2024.1
 * 日期: 2024年9月17日
 * 作者: A. Lelevé (修改版)
 *
 * **程序目标:**
 * 本程序实现移动机器人的速度测量功能:
 *   1. 使用中断方法读取编码器脉冲
 *   2. 每100毫秒计算一次左右轮的转速
 *   3. 通过串口发送速度数据到PC
 *   4. 持续运行10秒
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"

// ==============================================================================
// 用户可修改参数
// ==============================================================================

#define PWM_FREQ 1000
#define PWM_RESOLUTION 15

// --- 编码器参数 ---
// 编码器每转脉冲数 (根据实际编码器规格调整)
#define ENCODER_PPR 11  // 编码器每转脉冲数 (Pulses Per Revolution)
#define GEAR_RATIO 30   // 减速比
// 每转的总脉冲数 = PPR * 减速比 * 4 (四倍频)
#define PULSES_PER_REV (ENCODER_PPR * GEAR_RATIO * 4)

// 速度测量周期 (毫秒)
#define SPEED_MEASURE_PERIOD_MS 100
// 总运行时间 (秒)
#define TOTAL_RUN_TIME_S 10

// --- 编码器引脚定义 ---
// 右侧编码器
const uint8_t SRA = 35; // 右侧编码器A相引脚 (蓝色线)
const uint8_t SRB = 34; // 右侧编码器B相引脚 (紫色线)
// 左侧编码器
const uint8_t SLA = 14; // 左侧编码器A相引脚 (蓝色或紫色线)
const uint8_t SLB = 27; // 左侧编码器B相引脚 (紫色或蓝色线)

// --- 电机驱动器引脚定义 ---
// 左侧电机
const uint8_t MLF = 26; // 左侧电机前进
const uint8_t MLB = 25; // 左侧电机后退
// 右侧电机
const uint8_t MRF = 33; // 右侧电机前进
const uint8_t MRB = 32; // 右侧电机后退

// ==============================================================================
// 全局变量
// ==============================================================================
// PWM最大值
const uint32_t PWM_MAX = (1 << PWM_RESOLUTION) - 1;
// 电机速度设置 (占空比百分比，0-100)
const uint8_t MOTOR_SPEED = 50;

// --- 编码器计数器 (使用volatile因为会在中断中修改) ---
volatile int32_t leftEncoderCount = 0;   // 左轮编码器计数
volatile int32_t rightEncoderCount = 0;  // 右轮编码器计数

// --- 编码器上一次状态 (用于四倍频解码) ---
volatile uint8_t lastLeftState = 0;
volatile uint8_t lastRightState = 0;

// --- 互斥锁 (用于保护共享资源) ---
SemaphoreHandle_t encoderMutex;

// 临界区保护用的自旋锁
portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

// ==============================================================================
// 中断服务程序
// ==============================================================================

/**
 * @brief 左编码器A相中断处理函数
 * 使用四倍频解码方式
 */
void IRAM_ATTR leftEncoderISR_A() {
  uint8_t A = digitalRead(SLA);
  uint8_t B = digitalRead(SLB);
  uint8_t currentState = (A << 1) | B;
  
  // 根据状态转换判断方向
  // 正转: 00->01->11->10->00
  // 反转: 00->10->11->01->00
  int8_t direction = 0;
  switch (lastLeftState) {
    case 0b00:
      if (currentState == 0b01) direction = 1;
      else if (currentState == 0b10) direction = -1;
      break;
    case 0b01:
      if (currentState == 0b11) direction = 1;
      else if (currentState == 0b00) direction = -1;
      break;
    case 0b11:
      if (currentState == 0b10) direction = 1;
      else if (currentState == 0b01) direction = -1;
      break;
    case 0b10:
      if (currentState == 0b00) direction = 1;
      else if (currentState == 0b11) direction = -1;
      break;
  }
  
  leftEncoderCount += direction;
  lastLeftState = currentState;
}

/**
 * @brief 左编码器B相中断处理函数
 */
void IRAM_ATTR leftEncoderISR_B() {
  uint8_t A = digitalRead(SLA);
  uint8_t B = digitalRead(SLB);
  uint8_t currentState = (A << 1) | B;
  
  int8_t direction = 0;
  switch (lastLeftState) {
    case 0b00:
      if (currentState == 0b01) direction = 1;
      else if (currentState == 0b10) direction = -1;
      break;
    case 0b01:
      if (currentState == 0b11) direction = 1;
      else if (currentState == 0b00) direction = -1;
      break;
    case 0b11:
      if (currentState == 0b10) direction = 1;
      else if (currentState == 0b01) direction = -1;
      break;
    case 0b10:
      if (currentState == 0b00) direction = 1;
      else if (currentState == 0b11) direction = -1;
      break;
  }
  
  leftEncoderCount += direction;
  lastLeftState = currentState;
}

/**
 * @brief 右编码器A相中断处理函数
 */
void IRAM_ATTR rightEncoderISR_A() {
  uint8_t A = digitalRead(SRA);
  uint8_t B = digitalRead(SRB);
  uint8_t currentState = (A << 1) | B;
  
  int8_t direction = 0;
  switch (lastRightState) {
    case 0b00:
      if (currentState == 0b01) direction = 1;
      else if (currentState == 0b10) direction = -1;
      break;
    case 0b01:
      if (currentState == 0b11) direction = 1;
      else if (currentState == 0b00) direction = -1;
      break;
    case 0b11:
      if (currentState == 0b10) direction = 1;
      else if (currentState == 0b01) direction = -1;
      break;
    case 0b10:
      if (currentState == 0b00) direction = 1;
      else if (currentState == 0b11) direction = -1;
      break;
  }
  
  rightEncoderCount += direction;
  lastRightState = currentState;
}

/**
 * @brief 右编码器B相中断处理函数
 */
void IRAM_ATTR rightEncoderISR_B() {
  uint8_t A = digitalRead(SRA);
  uint8_t B = digitalRead(SRB);
  uint8_t currentState = (A << 1) | B;
  
  int8_t direction = 0;
  switch (lastRightState) {
    case 0b00:
      if (currentState == 0b01) direction = 1;
      else if (currentState == 0b10) direction = -1;
      break;
    case 0b01:
      if (currentState == 0b11) direction = 1;
      else if (currentState == 0b00) direction = -1;
      break;
    case 0b11:
      if (currentState == 0b10) direction = 1;
      else if (currentState == 0b01) direction = -1;
      break;
    case 0b10:
      if (currentState == 0b00) direction = 1;
      else if (currentState == 0b11) direction = -1;
      break;
  }
  
  rightEncoderCount += direction;
  lastRightState = currentState;
}

// ==============================================================================
// 电机控制函数
// ==============================================================================

void stopMotors() {
  analogWrite(MLF, 0);
  analogWrite(MLB, 0);
  analogWrite(MRF, 0);
  analogWrite(MRB, 0);
}

void moveForward(uint8_t speed) {
  uint32_t pwm_value = (PWM_MAX * speed) / 100;
  analogWrite(MLF, pwm_value);
  analogWrite(MLB, 0);
  analogWrite(MRF, pwm_value);
  analogWrite(MRB, 0);
}

void moveBackward(uint8_t speed) {
  uint32_t pwm_value = (PWM_MAX * speed) / 100;
  analogWrite(MLF, 0);
  analogWrite(MLB, pwm_value);
  analogWrite(MRF, 0);
  analogWrite(MRB, pwm_value);
}

// ==============================================================================
// 编码器辅助函数
// ==============================================================================

/**
 * @brief 获取并重置左编码器计数
 * @return 从上次调用以来的脉冲数
 */
int32_t getAndResetLeftEncoder() {
  portENTER_CRITICAL(&spinlock);
  int32_t count = leftEncoderCount;
  leftEncoderCount = 0;
  portEXIT_CRITICAL(&spinlock);
  return count;
}

/**
 * @brief 获取并重置右编码器计数
 * @return 从上次调用以来的脉冲数
 */
int32_t getAndResetRightEncoder() {
  portENTER_CRITICAL(&spinlock);
  int32_t count = rightEncoderCount;
  rightEncoderCount = 0;
  portEXIT_CRITICAL(&spinlock);
  return count;
}

/**
 * @brief 计算角速度 (rad/s)
 * @param pulseCount 脉冲数
 * @param periodMs 测量周期 (毫秒)
 * @return 角速度 (rad/s)
 */
float calculateAngularVelocity(int32_t pulseCount, uint32_t periodMs) {
  // 角速度 = (脉冲数 / 每转脉冲数) * 2π / 时间(秒)
  float revolutions = (float)pulseCount / PULSES_PER_REV;
  float timeSeconds = (float)periodMs / 1000.0;
  return (revolutions * 2.0 * PI) / timeSeconds;
}

// ==============================================================================
// 任务函数
// ==============================================================================

/**
 * @brief 速度测量任务
 * 每100ms测量一次左右轮速度，并通过串口发送
 */
void speedMeasureTask(void *pvParameters) {
  Serial.println("Speed measurement task started");
  Serial.println("Time(ms)\tLeft(rad/s)\tRight(rad/s)");
  
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint32_t elapsedTime = 0;
  const uint32_t totalTimeMs = TOTAL_RUN_TIME_S * 1000;
  
  while (elapsedTime < totalTimeMs) {
    // 等待下一个周期
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SPEED_MEASURE_PERIOD_MS));
    
    // 获取编码器计数并重置
    int32_t leftCount = getAndResetLeftEncoder();
    int32_t rightCount = getAndResetRightEncoder();
    
    // 计算角速度 (rad/s)
    float leftSpeed = calculateAngularVelocity(leftCount, SPEED_MEASURE_PERIOD_MS);
    float rightSpeed = calculateAngularVelocity(rightCount, SPEED_MEASURE_PERIOD_MS);
    
    elapsedTime += SPEED_MEASURE_PERIOD_MS;
    
    // 通过串口发送数据 (格式: 时间 左轮速度 右轮速度)
    Serial.print(elapsedTime);
    Serial.print("\t");
    Serial.print(leftSpeed, 3);
    Serial.print("\t");
    Serial.println(rightSpeed, 3);
  }
  
  Serial.println("Speed measurement completed");
  
  // 任务完成后删除自己
  vTaskDelete(NULL);
}

/**
 * @brief 电机控制任务
 * 运行电机以产生可测量的速度
 */
void motorControlTask(void *pvParameters) {
  Serial.println("Motor control task started");
  
  // 等待1秒让系统稳定
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  // 前进5秒
  Serial.println("Moving forward...");
  moveForward(MOTOR_SPEED);
  vTaskDelay(pdMS_TO_TICKS(5000));
  
  // 后退5秒
  Serial.println("Moving backward...");
  moveBackward(MOTOR_SPEED);
  vTaskDelay(pdMS_TO_TICKS(5000));
  
  // 停止
  Serial.println("Stopping motors");
  stopMotors();
  
  // 任务完成后删除自己
  vTaskDelete(NULL);
}

// ==============================================================================
// 支持函数
// ==============================================================================

void init_motor_pwm(uint8_t pin) {
  ledcAttach(pin, PWM_FREQ, PWM_RESOLUTION);
  analogWrite(pin, 0);
}

/**
 * @brief 初始化编码器引脚并配置中断
 */
void init_encoder_with_interrupt(uint8_t pinA, uint8_t pinB, 
                                  void (*isrA)(), void (*isrB)(),
                                  volatile uint8_t *lastState) {
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  
  // 读取初始状态
  *lastState = (digitalRead(pinA) << 1) | digitalRead(pinB);
  
  // 配置中断 (双边沿触发)
  attachInterrupt(digitalPinToInterrupt(pinA), isrA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB), isrB, CHANGE);
}

// ==============================================================================
// Arduino核心函数
// ==============================================================================

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Setup start: Speed Measurement with Interrupts");

  // 初始化电机PWM
  init_motor_pwm(MLF);
  init_motor_pwm(MLB);
  init_motor_pwm(MRF);
  init_motor_pwm(MRB);

  // 初始化编码器并配置中断
  init_encoder_with_interrupt(SLA, SLB, leftEncoderISR_A, leftEncoderISR_B, &lastLeftState);
  init_encoder_with_interrupt(SRA, SRB, rightEncoderISR_A, rightEncoderISR_B, &lastRightState);

  // 创建速度测量任务 (较高优先级)
  xTaskCreate(
    speedMeasureTask,
    "SpeedMeasure",
    4096,
    NULL,
    2,  // 较高优先级
    NULL
  );

  // 创建电机控制任务
  xTaskCreate(
    motorControlTask,
    "MotorControl",
    2048,
    NULL,
    1,
    NULL
  );

  Serial.println("Setup complete, tasks created");
  
  // 挂起setup/loop任务
  TaskHandle_t setup_task = xTaskGetCurrentTaskHandle();
  vTaskSuspend(setup_task);
}

void loop() {
  Serial.println("loop function is running !!?? :-( ");
}
