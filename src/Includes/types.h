#pragma once
#include <stdint.h>
#define bool uint8_t
#define true 1
#define false 0
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define StringLenght 32768
typedef char string[StringLenght];

// вычисление длины статического массива
#define statlen(arr) (sizeof(arr) / sizeof((arr)[0]))
