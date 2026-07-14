#pragma once

#include "stdint.h"

void idt_initialize(void);
void kernel_exception_handler(uint32_t vector, uint32_t errorCode);
