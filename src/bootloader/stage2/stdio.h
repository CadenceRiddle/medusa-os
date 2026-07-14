#pragma once

// Writes one character to the bootloader console.
void putc(char c);

// Writes a null-terminated string to the bootloader console.
void puts(const char* str);

// Formats text for bootloader diagnostics using the local mini-printf.
void _cdecl printf(const char* fmt, ...);
