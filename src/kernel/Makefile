BUILD_DIR?=build/
ASM?=nasm
CC32?=gcc
LD32?=ld
OBJCOPY?=objcopy
ASMFLAGS?=-f bin
CFLAGS32?=-m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -nostdinc -I. -I.. -Wall -Wextra
LDFLAGS32?=-m elf_i386 -nostdlib -T linker32.ld

KERNEL32_OBJ=$(BUILD_DIR)/kernel/c/main32.o
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

$(KERNEL32_ELF): $(KERNEL32_OBJ) linker32.ld
	$(LD32) $(LDFLAGS32) -o $@ $(KERNEL32_OBJ)

$(KERNEL32_OBJ): main32.c stdint.h ../bootinfo.h
	@mkdir -p $(@D)
	$(CC32) $(CFLAGS32) -c -o $@ main32.c

clean:
	rm -f $(BUILD_DIR)/kernel.bin
	rm -rf $(BUILD_DIR)/kernel
