/**
 * 5GE-IF4 & 5GEA-IF5
 * 左轮速度闭环控制程序 (P控制器)
 * 版本: 2024.1
 * 日期: 2024年9月17日
 * 
 * **程序目标:**
 * 本程序实现左轮的速度闭环控制:
 *   1. 使用P控制器进行速度控制
 *   2. 阶跃设定点测试: 0 -> 2.5 rad/s -> -2.5 rad/s -> 0 (每500ms)
 *   3. 通过串口发送数据: 设定点、测量速度、控制信号 (TSV格式)
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
#define ENCODER_PPR 11  // 编码器每转脉冲数 (Pulses Per Revolution)
#define GEAR_RATIO 30   // 减速比
#define PULSES_PER_REV (ENCODER_PPR * GEAR_RATIO * 4)  // 四倍频

// --- 控制参数 ---
#define CONTROL_PERIOD_MS 100   // 控制周期 (毫秒)
#define STEP_PERIOD_MS 500      // 阶跃变化周期 (毫秒)

// --- P控制器参数 ---
// Kp需要根据实际情况调整
// 粗略估计: PWM_MAX对应最大速度约10 rad/s, 所以 Kp ≈ PWM_MAX / 10
#define KP 2000.0f  // 比例增益 (可调整)

// --- 编码器引脚定义 ---
// 左侧编码器
const uint8_t SLA = 14; // 左侧编码器A相引脚
const uint8_t SLB = 27; // 左侧编码器B相引脚
// 右侧编码器 (保留但本程序暂不使用)
const uint8_t SRA = 35;
const uint8_t SRB = 34;

// --- 电机驱动器引脚定义 ---
// 左侧电机
const uint8_t MLF = 26; // 左侧电机前进
const uint8_t MLB = 25; // 左侧电机后退
// 右侧电机 (保留但本程序暂不使用)
const uint8_t MRF = 33;
const uint8_t MRB = 32;

// ==============================================================================
// 全局变量
// ==============================================================================

// PWM最大值
const uint32_t PWM_MAX = (1 << PWM_RESOLUTION) - 1;

// --- 编码器计数器 (使用volatile因为会在中断中修改) ---
volatile int32_t leftEncoderCount = 0;

// --- 编码器上一次状态 (用于四倍频解码) ---
volatile uint8_t lastLeftState = 0;

// --- 控制相关变量 ---
float desiredSpeedLeft = 0.0f;  // 左轮期望速度 (rad/s)
float measuredSpeedLeft = 0.0f; // 左轮测量速度 (rad/s)
float controlSignalLeft = 0.0f; // 左轮控制信号

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

// ==============================================================================
// 电机控制函数
// ==============================================================================

/**
 * @brief 停止左电机
 */
void stopLeftMotor() {
  analogWrite(MLF, 0);
  analogWrite(MLB, 0);
}

/**
 * @brief 设置左电机PWM值
 * @param pwmValue PWM值，正值前进，负值后退
 */
void setLeftMotorPWM(int32_t pwmValue) {
  // 限幅
  if (pwmValue > (int32_t)PWM_MAX) pwmValue = PWM_MAX;
  if (pwmValue < -(int32_t)PWM_MAX) pwmValue = -PWM_MAX;
  
  if (pwmValue >= 0) {
    // 前进
    analogWrite(MLF, pwmValue);
    analogWrite(MLB, 0);
  } else {
    // 后退
    analogWrite(MLF, 0);
    analogWrite(MLB, -pwmValue);
  }
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
 * @brief 计算角速度 (rad/s)
 * @param pulseCount 脉冲数
 * @param periodMs 测量周期 (毫秒)
 * @return 角速度 (rad/s)
 */
float calculateAngularVelocity(int32_t pulseCount, uint32_t periodMs) {
  float revolutions = (float)pulseCount / PULSES_PER_REV;
  float timeSeconds = (float)periodMs / 1000.0f;
  return (revolutions * 2.0f * PI) / timeSeconds;
}

// ==============================================================================
// 控制器函数
// ==============================================================================

/**
 * @brief P控制器
 * @param setpoint 设定值
 * @param measured 测量值
 * @return 控制信号
 */
float pController(float setpoint, float measured) {
  float error = setpoint - measured;
  return KP * error;
}

// ==============================================================================
// 任务函数
// ==============================================================================

/**
 * @brief 速度控制任务 (左轮)
 * 实现P控制器的闭环速度控制
 */
void speedControlTask(void *pvParameters) {
  Serial.println("Speed control task started (Left wheel only, P controller)");
  Serial.println("Time(ms)\tSetpoint(rad/s)\tMeasured(rad/s)\tControl");
  
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint32_t elapsedTime = 0;
  
  // 阶跃设定点序列: 0 -> 2.5 -> -2.5 -> 0 (每500ms变化)
  const float setpoints[] = {0.0f, 2.5f, -2.5f, 0.0f};
  const int numSetpoints = 4;
  int currentSetpointIndex = 0;
  uint32_t setpointChangeTime = 0;
  
  // 总运行时间 = 4个阶跃 * 500ms = 2000ms + 额外时间观察最后状态
  const uint32_t totalTimeMs = 3000;  // 3秒总时间
  
  while (elapsedTime < totalTimeMs) {
    // 等待下一个控制周期
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    elapsedTime += CONTROL_PERIOD_MS;
    
    // 检查是否需要更新设定点
    if (elapsedTime - setpointChangeTime >= STEP_PERIOD_MS && 
        currentSetpointIndex < numSetpoints - 1) {
      currentSetpointIndex++;
      setpointChangeTime = elapsedTime;
    }
    desiredSpeedLeft = setpoints[currentSetpointIndex];
    
    // 获取编码器计数并计算速度
    int32_t leftCount = getAndResetLeftEncoder();
    measuredSpeedLeft = calculateAngularVelocity(leftCount, CONTROL_PERIOD_MS);
    
    // P控制器计算控制信号
    controlSignalLeft = pController(desiredSpeedLeft, measuredSpeedLeft);
    
    // 应用控制信号到电机
    setLeftMotorPWM((int32_t)controlSignalLeft);
    
    // 通过串口发送数据 (TSV格式: 时间 设定点 测量值 控制信号)
    Serial.print(elapsedTime);
    Serial.print("\t");
    Serial.print(desiredSpeedLeft, 3);
    Serial.print("\t");
    Serial.print(measuredSpeedLeft, 3);
    Serial.print("\t");
    Serial.println(controlSignalLeft, 1);
  }
  
  // 停止电机
  stopLeftMotor();
  Serial.println("Speed control completed");
  
  // 任务完成后删除自己
  vTaskDelete(NULL);
}

// ==============================================================================
// 支持函数
// ==============================================================================

/**
 * @brief 初始化PWM
 */
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
  Serial.println("Setup start: Left Wheel Speed Control (P Controller)");

  // 初始化左电机PWM
  init_motor_pwm(MLF);
  init_motor_pwm(MLB);
  
  // 初始化右电机PWM (保持停止)
  init_motor_pwm(MRF);
  init_motor_pwm(MRB);

  // 初始化左编码器并配置中断
  init_encoder_with_interrupt(SLA, SLB, leftEncoderISR_A, leftEncoderISR_B, &lastLeftState);

  // 等待1秒让系统稳定
  delay(1000);

  // 创建速度控制任务
  xTaskCreate(
    speedControlTask,
    "SpeedControl",
    4096,
    NULL,
    2,  // 较高优先级
    NULL
  );

  Serial.println("Setup complete, control task created");
  
  // 挂起setup/loop任务
  TaskHandle_t setup_task = xTaskGetCurrentTaskHandle();
  vTaskSuspend(setup_task);
}

void loop() {
  // 不应该执行到这里
  Serial.println("loop function is running !!?? :-( ");
}
