#pragma once
#include "stdint.h"

// Copies bytes between far pointers.
void far* memcpy(void far* dst, const void far* src, uint16_t num);

// Fills a far-pointer memory range.
void far * memset(void far* ptr, int value, uint16_t num);

// Compares two far-pointer memory ranges.
int memcmp(const void far* ptr1, const void far* ptr2, uint16_t num);
