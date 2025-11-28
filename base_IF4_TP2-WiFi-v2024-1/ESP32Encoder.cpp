/*
 * ESP32Encoder.cpp
 *
 * 创建于: 2018年10月15日
 * 作者: hephaestus
 *
 * **中文注释:**
 * 这是ESP32Encoder类的实现文件。它利用ESP32的脉冲计数器(PCNT)硬件外设
 * 来高效地处理正交编码器信号。
 */

#include "ESP32Encoder.h"
#include "soc/pcnt_struct.h" // 包含PCNT硬件寄存器的底层结构体定义

// --- 静态成员变量初始化 ---
enum puType ESP32Encoder::useInternalWeakPullResistors = DOWN; // 默认使用内部下拉电阻
// 用于存储所有编码器实例指针的静态数组，初始化为NULL
ESP32Encoder *ESP32Encoder::encoders[MAX_ESP32_ENCODERS] = { NULL, NULL, NULL,
	NULL,
	NULL, NULL, NULL, NULL };

bool ESP32Encoder::attachedInterrupt = false; // 标记PCNT中断服务是否已注册
pcnt_isr_handle_t ESP32Encoder::user_isr_handle = NULL; // PCNT中断服务的句柄

/**
 * @brief ESP32Encoder类的构造函数。
 *
 * 初始化所有成员变量为默认值。
 */
ESP32Encoder::ESP32Encoder() {
	attached = false;
	aPinNumber = (gpio_num_t) 0;
	bPinNumber = (gpio_num_t) 0;
	working = false;
	direction = false;
	unit = (pcnt_unit_t) -1; // -1 表示尚未分配PCNT单元
}

/**
 * @brief ESP32Encoder类的析构函数。
 */
ESP32Encoder::~ESP32Encoder() {
	// 在这里可以添加资源释放的代码
}

/**
 * @brief PCNT中断服务程序 (ISR - Interrupt Service Routine)。
 *
 * **核心功能:**
 * ESP32的PCNT硬件计数器是16位的，范围大约是[-32767, 32767]。当硬件计数器
 * 达到这个范围的极限时(上溢或下溢)，会触发一个中断。这个中断处理函数就是被
 * 调用来处理这个事件的。
 *
 * 它通过检查是哪个PCNT单元触发了中断，然后将硬件计数器的极限值
 * (例如+32766或-32766)累加到一个64位的软件计数器(ptr->count)中。
 * 这样，就巧妙地将一个16位的硬件计数器扩展成了一个范围极大的64位软件计数器，
 * 防止了因溢出造成的数据丢失。
 *
 * @param arg 传递给中断处理函数的参数 (此处未使用)。
 */
static void IRAM_ATTR pcnt_example_intr_handler(void *arg) {
	ESP32Encoder * ptr;

	uint32_t intr_status = PCNT.int_st.val; // 读取所有PCNT单元的中断状态寄存器
	int i;

	for (i = 0; i < PCNT_UNIT_MAX; i++) { // 遍历所有PCNT单元
		if (intr_status & (BIT(i))) { // 检查第i个单元是否触发了中断
			ptr = ESP32Encoder::encoders[i]; // 获取与该单元关联的编码器对象指针
			
			// 确定是上溢出还是下溢出，并将对应的极限值累加到64位软件计数器中
			int64_t status = 0;
			if(PCNT.status_unit[i].h_lim_lat){ // 发生了上溢 (达到 counter_h_lim)
				status = ptr->r_enc_config.counter_h_lim;
			}
			if(PCNT.status_unit[i].l_lim_lat){ // 发生了下溢 (达到 counter_l_lim)
				status = ptr->r_enc_config.counter_l_lim;
			}
			PCNT.int_clr.val = BIT(i); // 清除该单元的中断标志位，以便下次能再次触发
			ptr->count = status + ptr->count; // 累加到64位软件计数器
		}
	}
}

/**
 * @brief 核心的私有附加函数，用于配置和启动一个编码器。
 * @param a 编码器A相引脚。
 * @param b 编码器B相引脚。
 * @param et 编码器解码类型 (single, half, full)。
 */
void ESP32Encoder::attach(int a, int b, enum encType et) {
	if (attached) {
		Serial.println("Already attached, FAIL!");
		return;
	}

	// 1. 寻找一个可用的PCNT单元
	int index = 0;
	for (; index < MAX_ESP32_ENCODERS; index++) {
		if (ESP32Encoder::encoders[index] == NULL) {
			encoders[index] = this; // 将当前对象指针存入静态数组
			break;
		}
	}
	if (index == MAX_ESP32_ENCODERS) {
		Serial.println("Too many encoders, FAIL!"); // 所有PCNT单元都已被占用
		return;
	}

	// 2. 设置成员变量
	unit = (pcnt_unit_t) index;
	this->aPinNumber = (gpio_num_t) a;
	this->bPinNumber = (gpio_num_t) b;
	fullQuad = (et == full); // 标记是否为全正交模式

	// 3. 配置GPIO引脚
	gpio_reset_pin(aPinNumber);
    gpio_reset_pin(bPinNumber);
	gpio_set_direction(aPinNumber, GPIO_MODE_INPUT);
	gpio_set_direction(bPinNumber, GPIO_MODE_INPUT);

	// 根据设置启用内部上/下拉电阻
	if (useInternalWeakPullResistors == DOWN){
		gpio_pulldown_en(aPinNumber);
		gpio_pulldown_en(bPinNumber);
	}
	if (useInternalWeakPullResistors == UP){
		gpio_pullup_en(aPinNumber);
		gpio_pullup_en(bPinNumber);
	}

	// 4. 配置PCNT单元的通道0
	r_enc_config.pulse_gpio_num = aPinNumber; // A相作为脉冲输入
	r_enc_config.ctrl_gpio_num = bPinNumber;  // B相作为方向控制输入
	r_enc_config.unit = unit;
	r_enc_config.channel = PCNT_CHANNEL_0;
	
	// 根据B相电平决定A相脉冲是增加还是减少计数值
	r_enc_config.pos_mode = PCNT_COUNT_DEC; // B相为高时, A相脉冲导致计数值减少 (可根据需要调整为INC)
	r_enc_config.neg_mode = PCNT_COUNT_INC;   // B相为低时, A相脉冲导致计数值增加
	r_enc_config.lctrl_mode = PCNT_MODE_KEEP; // 控制信号从低到高时的行为: 保持当前计数方向
	r_enc_config.hctrl_mode = PCNT_MODE_REVERSE; // 控制信号从高到低时的行为: 反转计数方向

	r_enc_config.counter_h_lim = _INT16_MAX; // 设置硬件计数器上溢阈值
	r_enc_config.counter_l_lim = _INT16_MIN; // 设置硬件计数器下溢阈值
	pcnt_unit_config(&r_enc_config); // 应用配置

	// 5. 如果是全正交模式，额外配置通道1
	if (et == full) {
		// 通道1的配置与通道0类似，但输入引脚交换
		r_enc_config.pulse_gpio_num = bPinNumber; // B相作为脉冲输入
		r_enc_config.ctrl_gpio_num = aPinNumber;  // A相作为方向控制输入
		r_enc_config.channel = PCNT_CHANNEL_1;
		
		// 逻辑与通道0相反或调整，以捕捉B相的边沿
		r_enc_config.pos_mode = PCNT_COUNT_DEC;
		r_enc_config.neg_mode = PCNT_COUNT_INC;
		r_enc_config.lctrl_mode = PCNT_MODE_REVERSE;
		r_enc_config.hctrl_mode = PCNT_MODE_KEEP;
		pcnt_unit_config(&r_enc_config);
	} else {
		// 如果不是全正交模式，确保通道1被禁用
		r_enc_config.channel = PCNT_CHANNEL_1;
		r_enc_config.pos_mode = PCNT_COUNT_DIS; // 禁用计数
		r_enc_config.neg_mode = PCNT_COUNT_DIS;
		r_enc_config.lctrl_mode = PCNT_MODE_DISABLE;
		r_enc_config.hctrl_mode = PCNT_MODE_DISABLE;
		pcnt_unit_config(&r_enc_config);
	}

	// 6. 设置滤波器、中断和启动计数器
	setFilter(250); // 设置硬件滤波器，忽略宽度小于250*APB时钟周期的毛刺

	// 启用硬件计数器的上溢和下溢事件，这样它们才能触发中断
	pcnt_event_enable(unit, PCNT_EVT_H_LIM);
	pcnt_event_enable(unit, PCNT_EVT_L_LIM);

	pcnt_counter_pause(unit); // 先暂停计数器
	pcnt_counter_clear(unit); // 清零硬件计数器

	// 注册全局中断处理函数 (如果尚未注册)
	if (attachedInterrupt == false){
		attachedInterrupt = true;
		esp_err_t er = pcnt_isr_register(pcnt_example_intr_handler, (void *) NULL, 0, &user_isr_handle);
		if (er != ESP_OK){
			Serial.println("Encoder wrap interrupt failed");
		}
	}
	pcnt_intr_enable(unit); // 为当前PCNT单元启用中断
	pcnt_counter_resume(unit); // 恢复计数

	attached = true; // 标记附加成功
}


// --- 公共 `attach` 函数的实现 ---
void ESP32Encoder::attachHalfQuad(int a, int b) {
	attach(a, b, half);
}
void ESP32Encoder::attachSingleEdge(int a, int b) {
	attach(a, b, single);
}
void ESP32Encoder::attachFullQuad(int a, int b) {
	attach(a, b, full);
}

/**
 * @brief 设置编码器的计数值。
 * @param value 要设置的新计数值。
 */
void ESP32Encoder::setCount(int64_t value) {
	// 通过调整64位软件计数器的值来实现
	count = value - getCountRaw();
}

/**
 * @brief 获取16位硬件计数器的原始值。
 * @return 返回16位硬件计数值。
 */
int64_t ESP32Encoder::getCountRaw() {
	int16_t c;
	pcnt_get_counter_value(unit, &c);
	return c;
}

/**
 * @brief 获取完整的64位计数值。
 * @return 返回 64位软件计数器值 + 16位硬件计数器值。
 */
int64_t ESP32Encoder::getCount() {
	return getCountRaw() + count;
}

/**
 * @brief 获取自上次调用以来的计数值变化量。
 */
int64_t ESP32Encoder::getDt() {
  oldCount = actualCount;
  actualCount = getCount();
  return actualCount - oldCount;
}

/**
 * @brief 清零计数器。
 * 同时清零64位软件计数器和16位硬件计数器。
 */
int64_t ESP32Encoder::clearCount() {
	count = 0;
	return pcnt_counter_clear(unit);
}

/**
 * @brief 暂停硬件计数器。
 */
int64_t ESP32Encoder::pauseCount() {
	return pcnt_counter_pause(unit);
}

/**
 * @brief 恢复硬件计数器。
 */
int64_t ESP32Encoder::resumeCount() {
	return pcnt_counter_resume(unit);
}

/**
 * @brief 设置PCNT硬件滤波器。
 * @param value 滤波器阈值。单位是APB时钟周期数。最大1023。值为0则禁用。
 */
void ESP32Encoder::setFilter(uint16_t value) {
	if(value == 0) {
		pcnt_filter_disable(unit);
	} else {
		pcnt_set_filter_value(unit, value);
		pcnt_filter_enable(unit);
	}
}
