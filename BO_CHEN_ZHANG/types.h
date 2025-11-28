/*
 * types.h - 自定义类型定义和状态枚举
 *
 * 创建于: 2016年9月11日
 * 最后修改: 2017年2月27日
 * 作者: Florian Bianco (florian.bianco@univ-lyon1.fr)
 *         Romain Delpoux (romain.delpoux@insa-lyon.fr)
 *
 * **中文注释:**
 * 这个头文件'types.h'定义了一系列方便使用的自定义数据类型别名和状态枚举。
 * 这些定义旨在提高代码的可读性和平台独立性，使得代码在不同环境中更容易理解和移植。
 * 它们是项目特有的，而不是Simulink自动生成的。
 */

#ifndef TYPES_H_ // 防止头文件被重复包含
#define TYPES_H_

// --- 有符号整数类型别名 ---
// 这些typedef提供了固定大小的有符号整数类型，有助于代码在不同架构下的可移植性。
typedef signed char s8;   // 8位有符号整数 (signed char)
typedef signed short s16;  // 16位有符号整数 (signed short)
typedef signed long s32;   // 32位有符号整数 (signed long)

// --- 无符号整数类型别名 ---
// 使用C99标准库stdint.h中定义的固定宽度整数类型，确保精确的位宽。
typedef uint8_t u8;   // 8位无符号整数
typedef uint16_t u16;  // 16位无符号整数
typedef uint32_t u32;  // 32位无符号整数

// --- 状态枚举定义 ---
// 定义了一组表示程序或模块不同运行状态的枚举常量。
// 这种方式使得状态管理更加清晰和易读。
enum {
	STATE_ERROR = -1, // 错误状态，通常表示发生不可恢复的问题
	STATE_INIT  = 0,  // 初始化状态，表示模块或系统正在启动
	STATE_OK,         // 正常状态或成功状态，表示操作成功
	STATE_RUN,        // 运行状态，表示模块或系统正在正常运行
	STATE_STOP,       // 停止状态，表示模块或系统已停止运行
};

#endif /* TYPES_H_ */
