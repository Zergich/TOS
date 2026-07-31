#pragma once
#include <stdint.h>

#define bool uint8_t
#define true 1
#define false 0

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uintptr_t uptr; // для указателей (в зависимости от битности автоматом
                        // подстраивается разрядноть)

#define StringLenght 32768
typedef u32 string15[StringLenght];

// вычисление длины статического массива
#define statlen(arr) (sizeof(arr) / sizeof((arr)[0]))
