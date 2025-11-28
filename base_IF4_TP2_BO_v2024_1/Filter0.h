/*
 * 学术许可证 - 仅供教学、学术研究和满足学位授予机构的课程要求使用。
 * 不得用于政府、商业或其他组织用途。
 *
 * 文件: Filter0.h
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
 * 这个头文件'Filter0.h'是Simulink模型'Filter0'自动生成的代码。
 * 它定义了数字滤波器模块的接口、数据结构和函数原型，以便在嵌入式系统中使用。
 * 通常，这类文件不建议手动修改，因为任何修改都可能在模型重新生成时被覆盖。
 */

#ifndef RTW_HEADER_Filter0_h_ // 防止头文件被重复包含
#define RTW_HEADER_Filter0_h_

#ifndef Filter0_COMMON_INCLUDES_ // 防止通用包含文件被重复包含
#define Filter0_COMMON_INCLUDES_
#include "rtwtypes.h" // 包含Simulink Coder生成的标准数据类型定义
#endif                                 /* Filter0_COMMON_INCLUDES_ */

#include "Filter0_types.h" // 包含模型特定的类型定义

/* 用于访问实时模型数据结构的宏定义 */
#ifndef rtmGetErrorStatus // 获取错误状态的宏
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus // 设置错误状态的宏
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* 系统'<Root>'的块状态 (默认存储) */
// DW_Filter0_T 结构体用于存储滤波器的内部状态变量。
// 这些状态变量在每次滤波器步进计算中都会更新，并用于下一次计算。
typedef struct {
  real_T DigitalFilter_FILT_STATES[2]; /* '<S2>/Digital Filter' - 数字滤波器的状态变量 */
  real_T DigitalFilter_FILT_STATES_g[2];/* '<S3>/Digital Filter' - 另一个数字滤波器的状态变量 */
} DW_Filter0_T;

/* 外部输入 (具有默认存储的根输入端口信号) */
// ExtU_Filter0_T 结构体用于存储滤波器的外部输入信号。
// 'u1'和'u2'代表了模型的两个输入端口。
typedef struct {
  real_T u1;                           /* '<Root>/u1' - 第一个输入信号 */
  real_T u2;                           /* '<Root>/u2' - 第二个输入信号 */
} ExtU_Filter0_T;

/* 外部输出 (由具有默认存储的根输出端口馈送的信号) */
// ExtY_Filter0_T 结构体用于存储滤波器的外部输出信号。
// 'y1'和'y2'代表了模型的两个输出端口。
typedef struct {
  real_T y1;                           /* '<Root>/y1' - 第一个输出信号 */
  real_T y2;                           /* '<Root>/y2' - 第二个输出信号 */
} ExtY_Filter0_T;

/* 实时模型数据结构 */
// tag_RTM_Filter0_T 结构体包含了实时模型运行时的数据，例如错误状态。
struct tag_RTM_Filter0_T {
  const char_T * volatile errorStatus; // 模型运行时的错误状态字符串
};

/* 块状态 (默认存储) */
// 声明Filter0_DW为全局变量，用于存储滤波器的所有内部状态。
extern DW_Filter0_T Filter0_DW;

/* 外部输入 (具有默认存储的根输入端口信号) */
// 声明Filter0_U为全局变量，用于访问滤波器的输入。
extern ExtU_Filter0_T Filter0_U;

/* 外部输出 (由具有默认存储的根输出端口馈送的信号) */
// 声明Filter0_Y为全局变量，用于访问滤波器的输出。
extern ExtY_Filter0_T Filter0_Y;

/* 模型入口点函数 */
// 以下声明了滤波器模块的关键函数原型。
extern void Filter0_initialize(void); // 滤波器初始化函数，用于设置初始状态和参数
extern void Filter0_step(void);       // 滤波器步进函数，每次调用执行一次滤波计算
extern void Filter0_terminate(void);  // 滤波器终止函数，用于清理资源 (在嵌入式中通常为空)

/* 实时模型对象 */
// 声明Filter0_M为全局常量指针，指向实时模型数据结构。
extern RT_MODEL_Filter0_T *const Filter0_M;

/*-
 * 由于优化而被从模型中消除的块:
 *
 * Block '<S4>/Check Signal Attributes' : 未使用的代码路径消除
 * Block '<S5>/Check Signal Attributes' : 未使用的代码路径消除
 *
 * **中文注释:**
 * 这部分注释说明了Simulink在代码生成过程中，由于优化原因，某些块可能被消除。
 * 这有助于理解生成的代码为何不包含模型中所有可见的块。
 */

/*-
 * 生成的代码包含的注释允许您直接追溯到模型中的相应位置。
 * 基本格式是 <系统>/块名称，其中系统是系统编号 (Simulink唯一分配)，
 * 块名称是块的名称。
 *
 * 请注意，这段特定的代码来源于子系统构建，其系统编号与父模型不同。
 * 请参阅下面的子系统层次结构，并使用MATLAB的hilite_system命令追溯生成的代码
 * 到父模型。例如，
 *
 * hilite_system('SimuESP32/Filter')    - 打开子系统 SimuESP32/Filter
 * hilite_system('SimuESP32/Filter/Kp') - 打开并选择块 Kp
 *
 * 这是此模型的系统层次结构
 *
 * '<Root>' : 'SimuESP32' - 根系统名为'SimuESP32'
 * '<S1>'   : 'SimuESP32/Filter' - 系统1是'SimuESP32'下的'Filter'子系统
 * '<S2>'   : 'SimuESP32/Filter/Digital Filter Design' - 系统2是'Filter'下的数字滤波器设计块
 * '<S3>'   : 'SimuESP32/Filter/Digital Filter Design1' - 系统3是'Filter'下的另一个数字滤波器设计块
 * '<S4>'   : 'SimuESP32/Filter/Digital Filter Design/Check Signal Attributes' - 系统4
 * '<S5>'   : 'SimuESP32/Filter/Digital Filter Design1/Check Signal Attributes' - 系统5
 *
 * **中文注释:**
 * 这部分是Simulink生成代码的标准注释，用于帮助开发者将生成的C代码追溯回原始的Simulink模型。
 * 它提供了模型层次结构信息以及如何使用MATLAB命令进行可视化追踪的示例。
 */
#endif                                 /* RTW_HEADER_Filter0_h_ */

/*
 * 生成代码的文件尾部。
 *
 * [EOF]
 */
