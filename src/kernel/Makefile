BUILD_DIR?=build/
ASM?=nasm
CC32?=gcc
LD32?=ld
OBJCOPY?=objcopy
ASMFLAGS?=-f bin
ASMFLAGS32?=-f elf32
CFLAGS32?=-m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -nostdinc -I. -I.. -Wall -Wextra
LDFLAGS32?=-m elf_i386 -nostdlib -T linker32.ld

KERNEL32_C_OBJS=$(BUILD_DIR)/kernel/c/main32.o $(BUILD_DIR)/kernel/c/idt.o
KERNEL32_ASM_OBJS=$(BUILD_DIR)/kernel/asm/interrupt.o
KERNEL32_OBJS=$(KERNEL32_C_OBJS) $(KERNEL32_ASM_OBJS)
KERNEL32_ELF=$(BUILD_DIR)/kernel/main32.elf
KERNEL32_BIN=$(BUILD_DIR)/kernel/main32.bin

.PHONY: all kernel clean

all: kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: main.asm $(KERNEL32_BIN)
	@mkdir -p $(@D)
	$(ASM) $(ASMFLAGS) -DKERNEL32_BIN=\"$(KERNEL32_BIN)\" -o $@ main.asm

$(KERNEL32_BIN): $(KERNEL32_ELF)
	$(OBJCOPY) -O binary $< $@

$(KERNEL32_ELF): $(KERNEL32_OBJS) linker32.ld
	$(LD32) $(LDFLAGS32) -o $@ $(KERNEL32_OBJS)

$(BUILD_DIR)/kernel/c/%.o: %.c stdint.h idt.h ../bootinfo.h
	@mkdir -p $(@D)
	$(CC32) $(CFLAGS32) -c -o $@ $<

$(BUILD_DIR)/kernel/asm/%.o: %.asm
	@mkdir -p $(@D)
	$(ASM) $(ASMFLAGS32) -o $@ $<

clean:
	rm -f $(BUILD_DIR)/kernel.bin
	rm -rf $(BUILD_DIR)/kernel
