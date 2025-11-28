/**
 * 5GE-IF4 & 5GEA-IF5
 * WiFi远程遥控机器人 - 闭环速度控制程序
 * 版本: 2024.1
 * 
 * **程序目标:**
 * 本程序实现了通过WiFi远程控制机器人的功能:
 *   1. 使用PI控制器对两个轮子进行闭环速度控制
 *   2. 通过WiFi接收手机浏览器的控制指令 (前进/后退/左转/右转/停止)
 *   3. 根据接收的指令更新两个轮子的期望速度
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"

#include "ESP32Encoder.h"
#include "http_server.hpp"

// ==============================================================================
// 用户可修改参数
// ==============================================================================

// >>>>>>>>>>>>>>>>>>>>>>>>>> 需要修改的WiFi参数 <<<<<<<<<<<<<<<<<<<<<<<<<<<<
#define WIFI_NOM_ROBOT "MonRobot"     // WiFi热点的SSID (网络名称)
#define WIFI_MOT_DE_PASSE "123456789" // WiFi热点的密码 (仅支持数字)

// --- PWM参数 ---
#define PWM_FREQ 1000              // PWM频率
#define PWM_RESOLUTION 15          // PWM分辨率 (位)

// --- 编码器参数 ---
#define ENCODER_PPR 11             // 编码器每转脉冲数 (Pulses Per Revolution)
#define GEAR_RATIO 30              // 减速比
#define PULSES_PER_REV (ENCODER_PPR * GEAR_RATIO * 4)  // 四倍频

// --- 控制参数 ---
#define CONTROL_PERIOD_MS 50       // 控制周期 (毫秒)

// --- PI控制器参数 ---
#define KP 6000.0f                 // 比例增益
#define KI 8000.0f                 // 积分增益
#define INTEGRAL_MAX 15000.0f      // 积分限幅

// --- 运动速度参数 ---
#define FORWARD_SPEED 2.5f         // 前进/后退速度 (rad/s)
#define TURN_SPEED 1.5f            // 转向速度 (rad/s)

// <<<<<<<<<<<<<<<<<<<<<<<<<<<< 结束修改区域 <<<<<<<<<<<<<<<<<<<<<<<<<<<<

// ==============================================================================
// 硬件引脚定义
// ==============================================================================

// --- 编码器引脚 ---
const uint8_t SRA = 35;   // 右侧编码器A相
const uint8_t SRB = 34;   // 右侧编码器B相
const uint8_t SLA = 14;   // 左侧编码器A相
const uint8_t SLB = 27;   // 左侧编码器B相

// --- 电机驱动引脚 ---
const uint8_t MRF = 33;   // 右侧电机前进
const uint8_t MRB = 32;   // 右侧电机后退
const uint8_t MLF = 26;   // 左侧电机前进
const uint8_t MLB = 25;   // 左侧电机后退

// ==============================================================================
// 全局常量
// ==============================================================================
const char* ssid     = WIFI_NOM_ROBOT;
const char* password = WIFI_MOT_DE_PASSE;
const uint32_t PWM_MAX = (1 << PWM_RESOLUTION) - 1;

// ==============================================================================
// 全局变量
// ==============================================================================

// --- 编码器对象 ---
ESP32Encoder encodeur_gauche;
ESP32Encoder encodeur_droit;

// --- 速度控制变量 ---
volatile float desiredSpeedLeft = 0.0f;    // 左轮期望速度 (rad/s)
volatile float desiredSpeedRight = 0.0f;   // 右轮期望速度 (rad/s)
float measuredSpeedLeft = 0.0f;            // 左轮测量速度
float measuredSpeedRight = 0.0f;           // 右轮测量速度
float controlSignalLeft = 0.0f;            // 左轮控制信号
float controlSignalRight = 0.0f;           // 右轮控制信号
float integralLeft = 0.0f;                 // 左轮积分项
float integralRight = 0.0f;                // 右轮积分项

// --- 当前指令 ---
volatile int currentOrder = ORDER_ROBOT_STOP;

// --- 互斥锁 ---
portMUX_TYPE speedMutex = portMUX_INITIALIZER_UNLOCKED;

// ==============================================================================
// 电机控制函数
// ==============================================================================

/**
 * @brief 初始化一个PWM输出引脚
 */
void init_motor_pwm(uint8_t pin) {
  ledcAttach(pin, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(pin, 0);
}

/**
 * @brief 设置左电机PWM值
 * @param pwmValue PWM值，正值前进，负值后退
 */
void setLeftMotorPWM(int32_t pwmValue) {
  if (pwmValue > (int32_t)PWM_MAX) pwmValue = PWM_MAX;
  if (pwmValue < -(int32_t)PWM_MAX) pwmValue = -PWM_MAX;
  
  if (pwmValue >= 0) {
    analogWrite(MLF, pwmValue);
    analogWrite(MLB, 0);
  } else {
    analogWrite(MLF, 0);
    analogWrite(MLB, -pwmValue);
  }
}

/**
 * @brief 设置右电机PWM值
 * @param pwmValue PWM值，正值前进，负值后退
 */
void setRightMotorPWM(int32_t pwmValue) {
  if (pwmValue > (int32_t)PWM_MAX) pwmValue = PWM_MAX;
  if (pwmValue < -(int32_t)PWM_MAX) pwmValue = -PWM_MAX;
  
  if (pwmValue >= 0) {
    analogWrite(MRF, pwmValue);
    analogWrite(MRB, 0);
  } else {
    analogWrite(MRF, 0);
    analogWrite(MRB, -pwmValue);
  }
}

/**
 * @brief 停止所有电机
 */
void stopAllMotors() {
  analogWrite(MLF, 0);
  analogWrite(MLB, 0);
  analogWrite(MRF, 0);
  analogWrite(MRB, 0);
}

// ==============================================================================
// 速度计算函数
// ==============================================================================

/**
 * @brief 计算角速度 (rad/s)
 * @param pulseCount 脉冲数
 * @param periodMs 测量周期 (毫秒)
 * @return 角速度 (rad/s)
 */
float calculateAngularVelocity(int64_t pulseCount, uint32_t periodMs) {
  float revolutions = (float)pulseCount / PULSES_PER_REV;
  float timeSeconds = (float)periodMs / 1000.0f;
  return (revolutions * 2.0f * PI) / timeSeconds;
}

// ==============================================================================
// 控制器函数
// ==============================================================================

/**
 * @brief PI控制器
 * @param setpoint 设定值
 * @param measured 测量值
 * @param integral 积分项指针
 * @param dt 控制周期 (秒)
 * @return 控制信号
 */
float piController(float setpoint, float measured, float *integral, float dt) {
  float error = setpoint - measured;
  
  // 更新积分项
  *integral += error * dt;
  
  // 积分限幅
  if (*integral > INTEGRAL_MAX) *integral = INTEGRAL_MAX;
  if (*integral < -INTEGRAL_MAX) *integral = -INTEGRAL_MAX;
  
  // 如果设定点为0，重置积分项
  if (setpoint == 0.0f) {
    *integral = 0.0f;
  }
  
  return KP * error + KI * (*integral);
}

// ==============================================================================
// WiFi指令处理函数
// ==============================================================================

/**
 * @brief 根据WiFi接收的指令更新两个轮子的期望速度
 * @param order 从communicate_with_phone()接收到的指令
 * 
 * 指令与动作对应关系:
 *   - ORDER_ROBOT_FORWARD:  两轮同向正转 -> 前进
 *   - ORDER_ROBOT_BACKWARD: 两轮同向反转 -> 后退
 *   - ORDER_ROBOT_LEFT:     左轮减速/反转，右轮正转 -> 左转
 *   - ORDER_ROBOT_RIGHT:    左轮正转，右轮减速/反转 -> 右转
 *   - ORDER_ROBOT_STOP:     两轮停止
 */
void update_desired_speeds(int order) {
  float leftSpeed = 0.0f;
  float rightSpeed = 0.0f;
  
  switch (order) {
    case ORDER_ROBOT_FORWARD:
      leftSpeed = FORWARD_SPEED;
      rightSpeed = FORWARD_SPEED;
      Serial.println("[CMD] Forward");
      break;
      
    case ORDER_ROBOT_BACKWARD:
      leftSpeed = -FORWARD_SPEED;
      rightSpeed = -FORWARD_SPEED;
      Serial.println("[CMD] Backward");
      break;
      
    case ORDER_ROBOT_LEFT:
      leftSpeed = -TURN_SPEED;
      rightSpeed = TURN_SPEED;
      Serial.println("[CMD] Turn Left");
      break;
      
    case ORDER_ROBOT_RIGHT:
      leftSpeed = TURN_SPEED;
      rightSpeed = -TURN_SPEED;
      Serial.println("[CMD] Turn Right");
      break;
      
    case ORDER_ROBOT_STOP:
    default:
      leftSpeed = 0.0f;
      rightSpeed = 0.0f;
      Serial.println("[CMD] Stop");
      break;
  }
  
  // 使用临界区保护共享变量
  portENTER_CRITICAL(&speedMutex);
  desiredSpeedLeft = leftSpeed;
  desiredSpeedRight = rightSpeed;
  portEXIT_CRITICAL(&speedMutex);
}

// ==============================================================================
// FreeRTOS任务
// ==============================================================================

/**
 * @brief WiFi通信任务
 * 永久调用communicate_with_phone()函数以响应手机浏览器的请求
 */
void wifiCommunicationTask(void *pvParameters) {
  Serial.println("[TASK] WiFi communication task started");
  
  int lastOrder = ORDER_ROBOT_STOP;
  
  while (true) {
    // 调用WiFi通信函数
    int order = communicate_with_phone();
    
    // 如果接收到有效指令且与上次不同，则更新速度
    if (order != 0 && order != 1) {  // 0表示无客户端，1表示有活动但无有效指令
      if (order != lastOrder) {
        update_desired_speeds(order);
        lastOrder = order;
        currentOrder = order;
      }
    }
    
    // 短暂延时，避免占用过多CPU
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/**
 * @brief 速度控制任务 (两个轮子)
 * 实现PI控制器的闭环速度控制
 */
void speedControlTask(void *pvParameters) {
  Serial.println("[TASK] Speed control task started");
  
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const float dt = CONTROL_PERIOD_MS / 1000.0f;
  
  // 清除编码器计数
  encodeur_gauche.clearCount();
  encodeur_droit.clearCount();
  
  int64_t lastLeftCount = 0;
  int64_t lastRightCount = 0;
  
  while (true) {
    // 等待下一个控制周期
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    
    // 读取编码器当前值
    int64_t currentLeftCount = encodeur_gauche.getCount();
    int64_t currentRightCount = encodeur_droit.getCount();
    
    // 计算增量
    int64_t deltaLeft = currentLeftCount - lastLeftCount;
    int64_t deltaRight = currentRightCount - lastRightCount;
    
    lastLeftCount = currentLeftCount;
    lastRightCount = currentRightCount;
    
    // 计算测量速度
    measuredSpeedLeft = calculateAngularVelocity(deltaLeft, CONTROL_PERIOD_MS);
    measuredSpeedRight = calculateAngularVelocity(deltaRight, CONTROL_PERIOD_MS);
    
    // 获取期望速度 (临界区保护)
    float targetLeft, targetRight;
    portENTER_CRITICAL(&speedMutex);
    targetLeft = desiredSpeedLeft;
    targetRight = desiredSpeedRight;
    portEXIT_CRITICAL(&speedMutex);
    
    // PI控制器计算控制信号
    controlSignalLeft = piController(targetLeft, measuredSpeedLeft, &integralLeft, dt);
    controlSignalRight = piController(targetRight, measuredSpeedRight, &integralRight, dt);
    
    // 应用控制信号到电机
    setLeftMotorPWM((int32_t)controlSignalLeft);
    setRightMotorPWM((int32_t)controlSignalRight);
    
    // 调试输出 (可选，注释掉以减少串口输出)
    // Serial.printf("L: %.2f/%.2f, R: %.2f/%.2f\n", 
    //               targetLeft, measuredSpeedLeft, 
    //               targetRight, measuredSpeedRight);
  }
}

// ==============================================================================
// Arduino核心函数
// ==============================================================================

void setup() {
  // 启动串行通信
  Serial.begin(115200);
  while (!Serial);
  Serial.println("=================================");
  Serial.println("WiFi Remote Control Robot");
  Serial.println("=================================");
  Serial.println("[INFO] Setup start");

  // 配置并启动Wi-Fi热点
  wifi_start(ssid, password);
  Serial.println("[INFO] WiFi started");
  Serial.println("[INFO] SSID: " + String(ssid));
  Serial.println("[INFO] Password: " + String(password));
  Serial.println("[INFO] Connect to WiFi and open http://192.168.4.1 in browser");

  // 初始化所有电机PWM
  init_motor_pwm(MLF);
  init_motor_pwm(MLB);
  init_motor_pwm(MRF);
  init_motor_pwm(MRB);
  Serial.println("[INFO] Motors initialized");

  // 配置编码器
  encodeur_gauche.attachFullQuad(SLA, SLB);
  encodeur_droit.attachFullQuad(SRA, SRB);
  encodeur_gauche.clearCount();
  encodeur_droit.clearCount();
  Serial.println("[INFO] Encoders initialized");

  // 等待1秒让系统稳定
  delay(1000);

  // 创建WiFi通信任务
  xTaskCreate(
    wifiCommunicationTask,
    "WiFiComm",
    4096,
    NULL,
    1,  // 较低优先级
    NULL
  );

  // 创建速度控制任务
  xTaskCreate(
    speedControlTask,
    "SpeedCtrl",
    4096,
    NULL,
    2,  // 较高优先级 (控制任务需要实时性)
    NULL
  );

  Serial.println("[INFO] All tasks created");
  Serial.println("[INFO] Setup complete");
  Serial.println("=================================");

  // 删除setup任务
  TaskHandle_t setup_task_t = xTaskGetCurrentTaskHandle();
  vTaskDelete(setup_task_t);
}

void loop() {
  // 此处不应有任何代码，所有逻辑在FreeRTOS任务中运行
}
