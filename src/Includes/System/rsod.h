#pragma once
#include <types.h>

void DivideZero();
void PageError();
void OpcodeError();
void DoubleFaultError();
void GeneralProqtectionError();
void PMMError();
void PMMBitMapPlaceNotfound();
void GeneralProtectionError();

void PanicU(u32 *str);
void PanicC(char *str);

#define Panic(x) _Generic((x), u32 *: PanicU, char *: PanicC)(x)
