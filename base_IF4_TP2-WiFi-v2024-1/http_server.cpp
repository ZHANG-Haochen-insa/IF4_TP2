
#include "http_server.hpp"
#include <WiFi.h>

/**
 * **中文注释:**
 * 这个文件实现了HTTP服务器的功能，用于通过WiFi接收来自手机浏览器的控制指令。
 */

// ==============================================================================
// 全局变量 - HTTP服务器/WiFi相关
// ==============================================================================

// 创建一个WiFi服务器对象，监听80端口 (HTTP默认端口)
WiFiServer wifiServer(80);

// 创建一个WiFi客户端对象，用于处理连接
WiFiClient client;

// --- 全局变量 ---
// 用于存储从WiFi接收到的指令
unsigned short reponse = ORDER_ROBOT_STOP;

// 标记是否有客户端已连接
bool clientConnected = false;

// 用于存储从客户端接收到的完整HTTP请求头
String header;


/**
 * @brief 启动WiFi功能并设置为接入点(AP)模式。
 *
 * 此函数将ESP32配置为一个WiFi热点，手机等设备可以连接到这个网络。
 *
 * @param ssid WiFi网络的名称 (SSID)。
 * @param password WiFi网络的密码。
 */
void wifi_start(const char * ssid, const char * password) {
  Serial.println("\n[INFO] Configuring access point");
  WiFi.mode(WIFI_AP); // 设置WiFi为接入点模式
  WiFi.softAP(ssid, password); // 启动AP，并设置SSID和密码

  // 打印AP的IP地址，手机浏览器需要访问这个地址
  Serial.print("[INFO] Started access point at IP ");
  Serial.println(WiFi.softAPIP());

  // 启动服务器，开始监听客户端连接
  wifiServer.begin();
}

/**
 * @brief 处理与手机的通信。
 *
 * 此函数应被循环调用。它会检查是否有新的客户端连接，
 * 如果有，则读取HTTP请求，解析指令，并返回一个包含控制按钮的HTML页面。
 *
 * @return 返回从HTTP请求中解析出的指令代码 (来自`_ORDER`枚举)。
 *         如果没有新的有效指令，则可能返回上一次的指令或默认值。
 */
int communicate_with_phone() {

  int reponse = 0; // 本次调用的响应，默认为0 (无指令)
  
  // 检查是否有新的客户端（例如，手机浏览器）尝试连接
  WiFiClient client = wifiServer.available();

  if (client) { // 如果有新客户端连接...
    reponse = 1; // 标记有活动
    Serial.println("New Client."); // 在串口打印新连接信息
    String currentLine = ""; // 用于逐行读取HTTP请求
    while (client.connected()) { // 当客户端保持连接时循环
      
      if (client.available()) { // 如果客户端有数据可读
        char c = client.read(); // 读取一个字节
        Serial.write(c); // 在串口打印出来，用于调试
        header += c; // 将字节附加到完整的请求头字符串
        if (c == '\n') { // 如果读到换行符，表示一行结束
          
          // 如果当前行是空行，说明收到了两个连续的换行符，
          // 这标志着HTTP请求头的结束，此时可以发送响应了。
          if (currentLine.length() == 0) {
            // --- 发送HTTP响应头 ---
            client.println("HTTP/1.1 200 OK"); // 状态码: 200 OK
            client.println("Content-type:text/html"); // 内容类型: HTML
            client.println("Connection: close"); // 完成后关闭连接
            client.println(); // 响应头和内容之间的空行

            // --- 解析HTTP GET请求，确定收到的指令 ---
            // 通过查找请求头字符串中是否包含特定的URL来判断用户按了哪个按钮
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("Received Robot Forward");
              reponse = ORDER_ROBOT_FORWARD;
            }
            if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("Received Robot Left");
              reponse = ORDER_ROBOT_LEFT;
            }
            if (header.indexOf("GET /28/on") >= 0) {
              Serial.println("Received Robot Right");
              reponse = ORDER_ROBOT_RIGHT;
            }
            if (header.indexOf("GET /29/on") >= 0) {
              Serial.println("Received Robot Backward");
              reponse = ORDER_ROBOT_BACKWARD;
            }
            // 任何"off"指令都视为停止
            if (header.indexOf("GET /26/off") >= 0 || header.indexOf("GET /27/off") >= 0 || header.indexOf("GET /28/off") >= 0 || header.indexOf("GET /29/off") >= 0) {
              Serial.println("Received Robot Stop");
              reponse = ORDER_ROBOT_STOP;
            }

            // --- 发送HTML网页内容 ---
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS样式，用于美化按钮
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}"); // "OFF"按钮的样式
            client.println(".marge {margin-left: 10em;}");
            client.println(".marge2 {margin-left: 2em;}");
            client.println("</style></head>");

            // 网页标题
            client.println("<body><h1><p>ESP32Robot</p> <p>DC motor drive over Wi-Fi</p></h1>");

            // --- 根据当前指令动态生成按钮 ---
            // 这种逻辑使得按下的按钮会变成"OFF"状态，其他按钮为"ON"
            client.println("<p>Forward</p>");
            if (reponse != ORDER_ROBOT_FORWARD) {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("<p>Left <span class=\"marge\">Right</span></p>");
            if (reponse != ORDER_ROBOT_LEFT) {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a><span class=\"marge2\">");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a><span class=\"marge2\">");
            }
            if (reponse != ORDER_ROBOT_RIGHT) {
              client.println("<a href=\"/28/on\"><button class=\"button\">ON</button></a></span></p></p>");
            } else {
              client.println("<a href=\"/28/off\"><button class=\"button button2\">OFF</button></a></span></p></p>");
            }
            
            client.println("<p>Backward</p>");
            if (reponse != ORDER_ROBOT_BACKWARD) {
              client.println("<p><a href=\"/29/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/29/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("</body></html>");

            // HTTP响应以另一个空行结束
            client.println();
            // 跳出while循环，准备断开连接
            break;
          } else { // 如果收到了换行符，但不是空行
            currentLine = ""; // 清空当前行字符串，准备接收下一行
          }
        } else if (c != '\r') {  // 如果收到的不是回车符
          currentLine += c;      // 将其附加到当前行
        }
      }
    }
    
    // --- 清理与断开 ---
    header = ""; // 清空请求头字符串，为下次连接做准备
    client.stop(); // 关闭与客户端的连接
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  return reponse; // 返回解析到的指令
}
