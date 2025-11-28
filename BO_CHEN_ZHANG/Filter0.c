/*
 * 学术许可证 - 仅供教学、学术研究和满足学位授予机构的课程要求使用。
 * 不得用于政府、商业或其他组织用途。
 *
 * 文件: Filter0.c
 *
 * 为Simulink模型'Filter0'生成的代码。
 *
 * 模型版本                  : 1.2
 * Simulink Coder 版本         : 9.4 (R2020b) 2020年7月29日
 * C/C++ 源代码生成时间        : 2021年4月27日 22:00:35 (周二)
 *
 * 目标选择: ert.tlc
 * 嵌入式硬件选择: Intel->x86-64 (Windows64)
 * 代码生成目标: 未指定
 * 验证结果: 未运行
 *
 * **中文注释:**
 * 这个源文件'Filter0.c'是Simulink模型'Filter0'自动生成的C语言实现文件。
 * 它包含了数字滤波器的具体算法逻辑，以及与模型数据交互所需的全局变量。
 * 和其他Simulink生成的文件一样，通常不建议手动修改此文件。
 */

#include "Filter0.h"         // 包含滤波器的公共接口和数据结构定义
#include "Filter0_private.h" // 包含滤波器的私有声明 (在此案例中仅包含rtwtypes.h)

/* 块状态 (默认存储) */
// Filter0_DW 结构体实例，存储数字滤波器的内部状态（如延迟线元素）。
// 这些状态在每次滤波器计算时更新，并在调用之间保持其值。
DW_Filter0_T Filter0_DW;

/* 外部输入 (具有默认存储的根输入端口信号) */
// Filter0_U 结构体实例，用于接收滤波器的外部输入信号（u1, u2）。
// 外部代码在使用滤波器前，需要将输入数据写入此结构体。
ExtU_Filter0_T Filter0_U;

/* 外部输出 (由具有默认存储的根输出端口馈送的信号) */
// Filter0_Y 结构体实例，用于存储滤波器的计算结果（y1, y2）。
// 外部代码在调用滤波器步进函数后，可以从此处读取输出数据。
ExtY_Filter0_T Filter0_Y;

/* 实时模型 */
// Filter0_M_ 是实时模型数据结构的一个静态实例，它包含了模型运行时的元数据。
static RT_MODEL_Filter0_T Filter0_M_;
// Filter0_M 是一个指向 Filter0_M_ 实例的常量指针，提供对模型数据的全局访问。
RT_MODEL_Filter0_T *const Filter0_M = &Filter0_M_;

/* 模型步进函数 */
/**
 * @brief 数字滤波器的主步进函数。
 *
 * 此函数执行一个时间步长的数字滤波计算。
 * 它根据输入信号 `Filter0_U` 和当前的内部状态 `Filter0_DW` 来计算新的输出 `Filter0_Y`，
 * 并更新内部状态 `Filter0_DW` 以供下一个时间步长使用。
 *
 * 该函数实现了两个独立的数字滤波器（对应u1/y1和u2/y2），它们可能采用
 * 双二阶（biquad）滤波器结构，其系数直接嵌入在代码中。
 */
void Filter0_step(void)
{
  real_T denAccum; // 用于存储分母累加计算的中间变量

  /* S-Function (sdspbiquad): '<S2>/Digital Filter' 包含了:
   *  输入端口: '<Root>/u1'
   *
   * 这是第一个数字滤波器的计算部分，处理输入u1。
   * 计算涉及到当前的输入u1、滤波器的历史状态DigitalFilter_FILT_STATES[0]和[1]，
   * 以及由Simulink模型生成的固定系数。
   */
  denAccum = (0.0674552738890719 * Filter0_U.u1 - -1.1429805025399011 *
              Filter0_DW.DigitalFilter_FILT_STATES[0]) - 0.41280159809618877 *
    Filter0_DW.DigitalFilter_FILT_STATES[1];

  /* 输出端口: '<Root>/y1' 包含了:
   *  S-Function (sdspbiquad): '<S2>/Digital Filter'
   *
   * 根据计算出的denAccum和滤波器的状态，得出第一个输出y1。
   */
  Filter0_Y.y1 = (2.0 * Filter0_DW.DigitalFilter_FILT_STATES[0] + denAccum) +
    Filter0_DW.DigitalFilter_FILT_STATES[1];

  /* S-Function (sdspbiquad): '<S2>/Digital Filter' */
  // 更新第一个数字滤波器的内部状态，为下一个时间步长做准备。
  Filter0_DW.DigitalFilter_FILT_STATES[1] =
    Filter0_DW.DigitalFilter_FILT_STATES[0];
  Filter0_DW.DigitalFilter_FILT_STATES[0] = denAccum;

  /* S-Function (sdspbiquad): '<S3>/Digital Filter' 包含了:
   *  输入端口: '<Root>/u2'
   *
   * 这是第二个数字滤波器的计算部分，处理输入u2，其结构和系数与第一个滤波器类似。
   */
  denAccum = (0.0674552738890719 * Filter0_U.u2 - -1.1429805025399011 *
              Filter0_DW.DigitalFilter_FILT_STATES_g[0]) - 0.41280159809618877 *
    Filter0_DW.DigitalFilter_FILT_STATES_g[1];

  /* 输出端口: '<Root>/y2' 包含了:
   *  S-Function (sdspbiquad): '<S3>/Digital Filter'
   *
   * 根据计算出的denAccum和滤波器的状态，得出第二个输出y2。
   */
  Filter0_Y.y2 = (2.0 * Filter0_DW.DigitalFilter_FILT_STATES_g[0] + denAccum) +
    Filter0_DW.DigitalFilter_FILT_STATES_g[1];

  /* S-Function (sdspbiquad): '<S3>/Digital Filter' */
  // 更新第二个数字滤波器的内部状态。
  Filter0_DW.DigitalFilter_FILT_STATES_g[1] =
    Filter0_DW.DigitalFilter_FILT_STATES_g[0];
  Filter0_DW.DigitalFilter_FILT_STATES_g[0] = denAccum;
}

/* 模型初始化函数 */
/**
 * @brief 数字滤波器的初始化函数。
 *
 * 此函数用于在滤波器开始工作前进行必要的初始化。
 * 在此生成的代码中，此函数是空的，表示当前滤波器实现不需要显式的初始化步骤
 * (例如，状态变量可能已在定义时默认初始化为零，或者其初始值不影响稳定性)。
 */
void Filter0_initialize(void)
{
  /* (无需初始化代码) */
}

/* 模型终止函数 */
/**
 * @brief 数字滤波器的终止函数。
 *
 * 此函数用于在滤波器不再需要时进行资源清理。
 * 在此生成的代码中，此函数是空的，表示当前滤波器实现不需要显式的终止步骤
 * (例如，没有动态分配的内存需要释放)。
 */
void Filter0_terminate(void)
{
  /* (无需终止代码) */
}

/*
 * 生成代码的文件尾部。
 *
 * [EOF]
 */
