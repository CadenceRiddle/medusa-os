#include "stdint.h"
#include "idt.h"
#include "../bootinfo.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((uint16_t*)0xB8000)
#define VGA_COLOR 0x0A

static uint16_t g_cursor_row;
static uint16_t g_cursor_col;

static void vga_newline(void){
    g_cursor_col = 0;
    if (++g_cursor_row >= VGA_HEIGHT)
        g_cursor_row = VGA_HEIGHT - 1;
}

static void vga_putc(char c){
    if (c == '\n'){
        vga_newline();
        return;
    }

    VGA_MEMORY[g_cursor_row * VGA_WIDTH + g_cursor_col] = ((uint16_t)VGA_COLOR << 8) | (uint8_t)c;

    if (++g_cursor_col >= VGA_WIDTH)
        vga_newline();
}

static void vga_puts(const char* str){
    while (*str){
        vga_putc(*str);
        str++;
    }
}

static void vga_clear(void){
    for (uint16_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++){
        VGA_MEMORY[i] = ((uint16_t)VGA_COLOR << 8) | ' ';
    }
    g_cursor_row = 0;
    g_cursor_col = 0;
}

static void print_hex_digit(uint8_t value){
    value &= 0x0F;
    vga_putc(value < 10 ? '0' + value : 'A' + value - 10);
}

static void print_hex16(uint16_t value){
    print_hex_digit(value >> 12);
    print_hex_digit(value >> 8);
    print_hex_digit(value >> 4);
    print_hex_digit(value);
}

static void print_hex32(uint32_t value){
    print_hex16((uint16_t)(value >> 16));
    print_hex16((uint16_t)value);
}

static void print_memory_map_entry(const BootMemoryMapEntry* entry){
    vga_puts("  base=0x");
    print_hex32(entry->baseHigh);
    print_hex32(entry->baseLow);
    vga_puts(" len=0x");
    print_hex32(entry->lengthHigh);
    print_hex32(entry->lengthLow);
    vga_puts(" type=0x");
    print_hex32(entry->type);
    vga_putc('\n');
}

void __attribute__((section(".text.entry"))) kernel_main32(const BootInfo* bootInfo){
    uint16_t entriesToPrint;

    vga_clear();
    vga_puts("Entered 32-bit protected mode\n");
    idt_initialize();
    vga_puts("Loaded protected-mode IDT\n");
    vga_puts("Hello from the Medusa protected-mode C kernel\n");
    vga_puts("Boot drive: 0x");
    print_hex16(bootInfo->bootDrive);
    vga_puts("\nKernel loaded at: ");
    print_hex16(bootInfo->kernelSegment);
    vga_putc(':');
    print_hex16(bootInfo->kernelOffset);
    vga_puts("\nMemory map entries: 0x");
    print_hex16(bootInfo->memoryMapEntryCount);
    vga_putc('\n');

    entriesToPrint = bootInfo->memoryMapEntryCount;
    if (entriesToPrint > 4)
        entriesToPrint = 4;

    for (uint16_t i = 0; i < entriesToPrint; i++){
        print_memory_map_entry(&bootInfo->memoryMapEntries[i]);
    }
}
