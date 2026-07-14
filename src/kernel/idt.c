#include "idt.h"

#define IDT_ENTRY_COUNT 256
#define KERNEL_CODE_SELECTOR 0x08
#define IDT_PRESENT_INTERRUPT_GATE 0x8E
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((uint16_t*)0xB8000)
#define EXCEPTION_COLOR 0x4F

typedef struct __attribute__((packed)){
    uint16_t offsetLow;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t offsetHigh;
} IdtEntry;

typedef struct __attribute__((packed)){
    uint16_t limit;
    uint32_t base;
} IdtDescriptor;

extern void* interrupt_stub_table[];

static IdtEntry g_idt[IDT_ENTRY_COUNT];
static IdtDescriptor g_idtDescriptor;

static void idt_set_gate(uint8_t vector, uint32_t handler){
    g_idt[vector].offsetLow = (uint16_t)handler;
    g_idt[vector].selector = KERNEL_CODE_SELECTOR;
    g_idt[vector].zero = 0;
    g_idt[vector].flags = IDT_PRESENT_INTERRUPT_GATE;
    g_idt[vector].offsetHigh = (uint16_t)(handler >> 16);
}

static void idt_load(void){
    __asm__ volatile ("lidt %0" : : "m"(g_idtDescriptor));
}

void idt_initialize(void){
    for (uint16_t i = 0; i < IDT_ENTRY_COUNT; i++){
        g_idt[i].offsetLow = 0;
        g_idt[i].selector = 0;
        g_idt[i].zero = 0;
        g_idt[i].flags = 0;
        g_idt[i].offsetHigh = 0;
    }

    for (uint8_t vector = 0; vector < 32; vector++){
        idt_set_gate(vector, (uint32_t)interrupt_stub_table[vector]);
    }

    g_idtDescriptor.limit = sizeof(g_idt) - 1;
    g_idtDescriptor.base = (uint32_t)g_idt;
    idt_load();
}

static void exception_write_at(uint16_t row, uint16_t col, const char* str){
    while (*str && row < VGA_HEIGHT){
        VGA_MEMORY[row * VGA_WIDTH + col] = ((uint16_t)EXCEPTION_COLOR << 8) | (uint8_t)*str;
        str++;
        col++;
        if (col >= VGA_WIDTH){
            col = 0;
            row++;
        }
    }
}

static void exception_write_hex32(uint16_t row, uint16_t col, uint32_t value){
    const char* digits = "0123456789ABCDEF";

    for (int8_t shift = 28; shift >= 0; shift -= 4){
        VGA_MEMORY[row * VGA_WIDTH + col] = ((uint16_t)EXCEPTION_COLOR << 8) | digits[(value >> shift) & 0x0F];
        col++;
    }
}

void kernel_exception_handler(uint32_t vector, uint32_t errorCode){
    exception_write_at(22, 0, "CPU exception 0x");
    exception_write_hex32(22, 16, vector);
    exception_write_at(23, 0, "Error code    0x");
    exception_write_hex32(23, 16, errorCode);

    for (;;){
        __asm__ volatile ("cli; hlt");
    }
}
