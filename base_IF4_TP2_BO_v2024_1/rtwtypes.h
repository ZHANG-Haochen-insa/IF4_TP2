/*
 * 学术许可证 - 仅供教学研究和满足学位授予机构的课程要求使用。
 * 不得用于政府、商业或其他组织用途。
 *
 * 文件: rtwtypes.h
 *
 * 为Simulink模型'Control'生成的代码。
 * (注意: 尽管此处提到'Control'模型,但此文件是Simulink代码生成器通用的一部分,
 *        适用于所有生成的C/C++代码,提供标准类型定义。)
 *
 * 模型版本                  : 1.3
 * Simulink Coder 版本         : 9.4 (R2020b) 2020年7月29日
 * C/C++ 源代码生成时间        : 2021年4月27日 23:09:49
 *
 * 目标选择: ert.tlc
 * 嵌入式硬件选择: Intel->x86-64 (Windows64)
 * 代码生成目标: 未指定
 * 验证结果: 未运行
 *
 * **中文注释:**
 * 这个头文件`rtwtypes.h`是Simulink Coder生成的标准类型定义文件。
 * 它的主要作用是提供一套与平台无关的、固定宽度的基本数据类型，
 * 以及布尔类型和复数类型定义，确保Simulink模型生成的C/C++代码在不同编译器和目标硬件上具有一致的行为。
 * 此文件是Simulink生成的嵌入式C代码库中的一个基础组件。
 */

#ifndef RTWTYPES_H // 防止头文件被重复包含
#define RTWTYPES_H

/* 逻辑类型定义 (Logical type definitions) */
#if (!defined(__cplusplus)) // 如果不是C++编译环境
#ifndef false // 定义false宏，如果尚未定义
#define false                          (0U)
#endif

#ifndef true // 定义true宏，如果尚未定义
#define true                           (1U)
#endif
#endif

/*=======================================================================*
 * 目标硬件信息 (Target hardware information)
 *   设备类型: Intel->x86-64 (Windows64)
 *   位宽:       char:   8    short:   16    int:  32
 *                         long:  32
 *                         native word size:  64
 *   字节序: 小端 (LittleEndian)
 *   有符号整数除法向: 零 (Zero) 取整
 *   有符号整数右移作为算术右移: 启用 (on)
 *
 * **中文注释:**
 * 这部分注释提供了生成代码所针对的硬件平台的关键特性信息，
 * 包括数据类型的位宽、字节序以及整数运算的行为，这对于跨平台兼容性很重要。
 *=======================================================================*/

/*=======================================================================*
 * 固定宽度字大小数据类型 (Fixed width word size data types):
 *   int8_T, int16_T, int32_T     - 有符号 8, 16, 或 32 位整数
 *   uint8_T, uint16_T, uint32_T  - 无符号 8, 16, 或 32 位整数
 *   real32_T, real64_T           - 32 和 64 位浮点数
 *
 * **中文注释:**
 * 这些`_T`后缀的类型是Simulink Coder为了保证代码可移植性而定义的标准类型别名，
 * 它们映射到C语言中具有特定位宽的原生类型。
 *=======================================================================*/
typedef signed char int8_T;   // 8位有符号整数
typedef unsigned char uint8_T;  // 8位无符号整数
typedef short int16_T;        // 16位有符号整数
typedef unsigned short uint16_T; // 16位无符号整数
typedef int int32_T;          // 32位有符号整数
typedef unsigned int uint32_T; // 32位无符号整数
typedef float real32_T;       // 32位单精度浮点数
typedef double real64_T;      // 64位双精度浮点数

/*===========================================================================*
 * 通用类型定义 (Generic type definitions): boolean_T, char_T, byte_T, int_T, uint_T,
 *                           real_T, time_T, ulong_T.
 *
 * **中文注释:**
 * 这些是Simulink模型中常用的通用数据类型。
 * 例如，`real_T`通常用于表示模型的浮点信号，`boolean_T`用于布尔逻辑。
 *===========================================================================*/
typedef double real_T;      // 默认的实数类型，通常是双精度浮点数
typedef double time_T;      // 时间类型，通常是双精度浮点数
typedef unsigned char boolean_T; // 布尔类型，通常是无符号字符
typedef int int_T;          // 通用整数类型
typedef unsigned int uint_T;  // 通用无符号整数类型
typedef unsigned long ulong_T; // 通用无符号长整数类型
typedef char char_T;        // 字符类型
typedef unsigned char uchar_T; // 无符号字符类型
typedef char_T byte_T;      // 字节类型，等同于char_T

/*===========================================================================*
 * 复数类型定义 (Complex number type definitions)
 *
 * **中文注释:**
 * Simulink支持复数运算，这部分定义了不同精度（单精度、双精度、整数）的复数结构体。
 * 每个结构体包含实部（re）和虚部（im）。
 *===========================================================================*/
#define CREAL_T // 宏定义，指示存在通用复数类型

typedef struct {
  real32_T re; // 单精度复数的实部
  real32_T im; // 单精度复数的虚部
} creal32_T;

typedef struct {
  real64_T re; // 双精度复数的实部
  real64_T im; // 双精度复数的虚部
} creal64_T;

typedef struct {
  real_T re; // 默认实数类型的复数的实部
  real_T im; // 默认实数类型的复数的虚部
} creal_T;

#define CINT8_T // 宏定义，指示存在8位有符号整数复数类型

typedef struct {
  int8_T re; // 8位有符号整数复数的实部
  int8_T im; // 8位有符号整数复数的虚部
} cint8_T;

#define CUINT8_T // 宏定义，指示存在8位无符号整数复数类型

typedef struct {
  uint8_T re; // 8位无符号整数复数的实部
  uint8_T im; // 8位无符号整数复数的虚部
} cuint8_T;

#define CINT16_T // 宏定义，指示存在16位有符号整数复数类型

typedef struct {
  int16_T re; // 16位有符号整数复数的实部
  int16_T im; // 16位有符号整数复数的虚部
} cint16_T;

#define CUINT16_T // 宏定义，指示存在16位无符号整数复数类型

typedef struct {
  uint16_T re; // 16位无符号整数复数的实部
  uint16_T im; // 16位无符号整数复数的虚部
} cuint16_T;

#define CINT32_T // 宏定义，指示存在32位有符号整数复数类型

typedef struct {
  int32_T re; // 32位有符号整数复数的实部
  int32_T im; // 32位有符号整数复数的虚部
} cint32_T;

#define CUINT32_T // 宏定义，指示存在32位无符号整数复数类型

typedef struct {
  uint32_T re; // 32位无符号整数复数的实部
  uint32_T im; // 32位无符号整数复数的虚部
} cuint32_T;

/*=======================================================================*
 * 最小值和最大值 (Min and Max):
 *   int8_T, int16_T, int32_T     - 有符号 8, 16, 或 32 位整数
 *   uint8_T, uint16_T, uint32_T  - 无符号 8, 16, 或 32 位整数
 *
 * **中文注释:**
 * 这些宏定义了不同整数类型的最大值和最小值，方便在代码中进行范围检查或初始化。
 *=======================================================================*/
#define MAX_int8_T                     ((int8_T)(127))     // 8位有符号整数最大值
#define MIN_int8_T                     ((int8_T)(-128))    // 8位有符号整数最小值
#define MAX_uint8_T                    ((uint8_T)(255U))   // 8位无符号整数最大值
#define MAX_int16_T                    ((int16_T)(32767))    // 16位有符号整数最大值
#define MIN_int16_T                    ((int16_T)(-32768))   // 16位无符号整数最大值
#define MAX_uint16_T                   ((uint16_T)(65535U))  // 16位无符号整数最大值
#define MAX_int32_T                    ((int32_T)(2147483647)) // 32位有符号整数最大值
#define MIN_int32_T                    ((int32_T)(-2147483647-1)) // 32位有符号整数最小值
#define MAX_uint32_T                   ((uint32_T)(0xFFFFFFFFU)) // 32位无符号整数最大值

/* 块 D-Work 指针类型 (Block D-Work pointer type) */
// `pointer_T` 是一个通用指针类型，通常用于指向Simulink模型中的D-Work（离散状态或工作向量）数据。
typedef void * pointer_T;

#endif                                 /* RTWTYPES_H */

/*
 * 生成代码的文件尾部。
 *
 * [EOF]
 */
