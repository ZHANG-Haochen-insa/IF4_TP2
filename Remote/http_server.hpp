/*
 * http_server.hpp - HTTP服务器接口头文件
 *
 * 创建于: 2022年12月13日
 * 最后修改: 2022年12月13日
 * 作者: Florian Bianco (florian.bianco@univ-lyon1.fr)
 *         Romain Delpoux (romain.delpoux@insa-lyon.fr)
 *         Arnaud Lelevé (arnaud.leleve@insa-lyon.fr)
 *
 * **中文注释:**
 * 这个头文件定义了与WiFi和HTTP服务器通信相关的功能接口。
 * 它声明了用于启动WiFi热点和处理手机指令的函数，并定义了一组机器人控制指令的枚举。
 */

#ifndef HTTP_SERVER_HPP_ // 防止头文件被重复包含
#define HTTP_SERVER_HPP_

#include <WiFi.h> // 包含ESP32的WiFi库

//- 全局类型定义 ----------------------------
/**
 * @enum _ORDER
 * @brief 定义了从手机接收到的机器人控制指令。
 *
 * 这些十六进制值是Web界面发送的特定指令代码，用于控制机器人的基本动作。
 */
enum _ORDER {
  ORDER_ROBOT_FORWARD  = 0x11, // 指令：机器人前进
  ORDER_ROBOT_BACKWARD = 0x22, // 指令：机器人后退
  ORDER_ROBOT_LEFT     = 0x21, // 指令：机器人向左
  ORDER_ROBOT_RIGHT    = 0x12, // 指令：机器人向右
  ORDER_ROBOT_STOP     = 0x33, // 指令：机器人停止
};


//- 函数原型 -----------------------

/**
 * @brief 启动WiFi功能。
 *
 * 此函数负责初始化ESP32的WiFi，通常是设置为一个接入点(Access Point, AP)模式，
 * 以便手机可以连接到机器人创建的WiFi网络。
 *
 * @param ssid 要创建的WiFi网络的名称 (SSID)。
 * @param password 要创建的WiFi网络的密码。
 */
void wifi_start(const char * ssid, const char * password);

/**
 * @brief 与手机进行通信。
 *
 * 此函数处理来自已连接手机的HTTP请求。它会监听、解析收到的指令，
 * 并返回一个`_ORDER`枚举中定义的命令代码。
 *
 * @return 返回一个代表机器人指令的整数值(对应`_ORDER`枚举)。
 *         如果没有新的指令，可能会返回一个特定的值(例如0或-1)。
 */
int communicate_with_phone();

#endif /* HTTP_SERVER_HPP_ */
