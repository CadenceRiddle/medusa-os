#pragma once

#include "stdint.h"

// Builds and loads the protected-mode IDT.
void idt_initialize(void);

// Handles CPU exceptions forwarded by assembly stubs.
void kernel_exception_handler(uint32_t vector, uint32_t errorCode);
