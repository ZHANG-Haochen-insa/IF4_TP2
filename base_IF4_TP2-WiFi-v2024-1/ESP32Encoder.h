#pragma once // 确保头文件只被包含一次
#include <Arduino.h>
#include <driver/gpio.h> // ESP-IDF GPIO驱动库
#include <driver/pcnt.h> // ESP-IDF 脉冲计数器(PCNT)驱动库

#define MAX_ESP32_ENCODERS PCNT_UNIT_MAX // 定义ESP32支持的最大编码器数量，等于PCNT单元的数量
#define _INT16_MAX 32766 // 定义一个16位有符号整数的最大值 (用于PCNT硬件限制)
#define _INT16_MIN -32766 // 定义一个16位有符号整数的最小值

// 定义编码器解码类型
enum encType {
	single, // 单边沿模式: 只在一个通道的一个边沿计数
	half,   // 半正交模式: 在一个通道的上升沿和下降沿都计数
	full    // 全正交模式: 在两个通道的上升沿和下降沿都计数，精度最高
};

// 定义内部上拉电阻类型
enum puType {
	UP,   // 启用上拉
	DOWN, // 启用下拉
	NONE  // 不启用
};

/**
 * @class ESP32Encoder
 * @brief 使用ESP32硬件脉冲计数器(PCNT)来处理正交编码器的C++类。
 *
 * 这个类封装了ESP-IDF的PCNT外设驱动，提供了一个简单易用的接口来读取
 * 电机等设备上的正交编码器。与基于软件中断的方法相比，它利用硬件
 * 进行计数，效率更高，性能更可靠，并且不会占用CPU资源。
 */
class ESP32Encoder {
private:
	/**
	 * @brief 私有的核心附加函数。
	 * @param aPinNumber 编码器A相引脚。
	 * @param bPinNumber 编码器B相引脚。
	 * @param et 编码器类型 (single, half, full)。
	 */
	void attach(int aPintNumber, int bPinNumber, enum encType et);
	bool attached = false; // 标记编码器是否已成功附加

	static pcnt_isr_handle_t user_isr_handle; // 静态成员，用于处理PCNT中断服务
    bool direction;      // 编码器旋转方向
    bool working;        // 标记编码器是否正在工作

	static bool attachedInterrupt; // 标记中断是否已附加
	int64_t getCountRaw(); // 获取原始计数值 (未使用)

    int64_t oldCount; // 上一次的计数值
    int64_t actualCount; // 当前的计数值

public:
	/**
	 * @brief 构造函数。
	 */
	ESP32Encoder();
	/**
	 * @brief 析构函数。
	 */
	~ESP32Encoder();

	/**
	 * @brief 以半正交模式附加编码器。
	 * 每个时钟周期计数两次。
	 * @param aPinNumber 编码器A相引脚。
	 * @param bPinNumber 编码器B相引脚。
	 */
	void attachHalfQuad(int aPinNumber, int bPinNumber);

	/**
	 * @brief 以全正交模式附加编码器。
	 * 每个时钟周期计数四次，提供最高精度。
	 * @param aPinNumber 编码器A相引脚。
	 * @param bPinNumber 编码器B相引脚。
	 */
	void attachFullQuad(int aPinNumber, int bPinNumber);

	/**
	 * @brief 以单边沿模式附加编码器。
	 * 每个时钟周期计数一次。
	 * @param aPinNumber 编码器A相引脚。
	 * @param bPinNumber 编码器B相引脚。
	 */
	void attachSingleEdge(int aPinNumber, int bPinNumber);

	/**
	 * @brief 获取当前编码器的计数值。
	 * @return 返回一个64位有符号整数表示的计数值。
	 */
    int64_t getCount();

    /**
     * @brief 获取自上次调用以来的计数值变化量 (未完全实现或使用)。
     */
    int64_t getDt();

	/**
	 * @brief 清除(重置)编码器的计数值为0。
	 * @return 返回清除前的计数值。
	 */
	int64_t clearCount();

	/**
	 * @brief 暂停PCNT单元的计数。
	 * @return 返回暂停前的计数值。
	 */
	int64_t pauseCount();

	/**
	 * @brief 恢复PCNT单元的计数。
	 * @return 返回当前的计数值。
	 */
	int64_t resumeCount();

	/**
	 * @brief 检查编码器是否已附加。
	 * @return 如果已附加返回true,否则返回false。
	 */
	boolean isAttached(){return attached;}

	/**
	 * @brief 设置编码器的计数值为指定值。
	 * @param value 要设置的新计数值。
	 */
	void setCount(int64_t value);

	/**
	 * @brief 设置PCNT硬件滤波器。
	 * 滤波器可以忽略指定宽度以下的毛刺信号。
	 * @param value 滤波器阈值，单位为APB总线时钟周期(通常为80MHz)。最大值为1023。
	 *              值为0时禁用滤波器。
	 */
	void setFilter(uint16_t value);

	// --- 公共成员变量 ---
	static ESP32Encoder *encoders[MAX_ESP32_ENCODERS]; // 静态数组，用于追踪所有实例化的编码器对象
	gpio_num_t aPinNumber;   // A相GPIO引脚号
	gpio_num_t bPinNumber;   // B相GPIO引脚号
	pcnt_unit_t unit;        // 使用的PCNT硬件单元 (例如 PCNT_UNIT_0)
	bool fullQuad = false;   // 标记是否为全正交模式
	int countsMode = 2;      // 计数模式 (2=半正交, 4=全正交)
	volatile int64_t count = 0; // 存储编码器计数值的变量 (volatile确保线程安全)
	pcnt_config_t r_enc_config; // PCNT外设的配置结构体
	static enum puType useInternalWeakPullResistors; // 控制是否使用内部上下拉电阻的静态变量
};

// 由Sloeber IDE添加的pragma once,功能重复
#pragma once
