#include "stdint.h"
#include "../bootinfo.h"

void _cdecl kernel_putchar(char c);

void kernel_puts(const char* str){
    while (*str){
        if (*str == '\n')
            kernel_putchar('\r');

        kernel_putchar(*str);
        str++;
    }
}

void kernel_print_hex_digit(uint8_t value){
    value &= 0x0F;
    kernel_putchar(value < 10 ? '0' + value : 'A' + value - 10);
}

void kernel_print_hex16(uint16_t value){
    kernel_print_hex_digit(value >> 12);
    kernel_print_hex_digit(value >> 8);
    kernel_print_hex_digit(value >> 4);
    kernel_print_hex_digit(value);
}

void kernel_print_hex32(uint32_t value){
    uint16_t* words = (uint16_t*)&value;
    kernel_print_hex16(words[1]);
    kernel_print_hex16(words[0]);
}

void kernel_print_memory_map_entry(BootMemoryMapEntry far* entry){
    kernel_puts("  base=0x");
    kernel_print_hex32(entry->baseHigh);
    kernel_print_hex32(entry->baseLow);
    kernel_puts(" length=0x");
    kernel_print_hex32(entry->lengthHigh);
    kernel_print_hex32(entry->lengthLow);
    kernel_puts(" type=0x");
    kernel_print_hex32(entry->type);
    kernel_puts("\n");
}

void _cdecl kernel_main(BootInfo far* bootInfo){
    uint16_t entriesToPrint;

    kernel_puts("Hello from the Medusa C kernel!\n");
    kernel_puts("The bootloader loaded kernel.bin and jumped here.\n");
    kernel_puts("Boot drive: 0x");
    kernel_print_hex16(bootInfo->bootDrive);
    kernel_puts("\nKernel loaded at: ");
    kernel_print_hex16(bootInfo->kernelSegment);
    kernel_putchar(':');
    kernel_print_hex16(bootInfo->kernelOffset);
    kernel_puts("\n");
    kernel_puts("Memory map entries: 0x");
    kernel_print_hex16(bootInfo->memoryMapEntryCount);
    kernel_puts("\n");

    entriesToPrint = bootInfo->memoryMapEntryCount;
    if (entriesToPrint > 4)
        entriesToPrint = 4;

    for (uint16_t i = 0; i < entriesToPrint; i++){
        kernel_print_memory_map_entry(&bootInfo->memoryMapEntries[i]);
    }
}
