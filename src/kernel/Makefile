BUILD_DIR?=build/
ASM?=nasm
WATCOM?=$(HOME)/watcom
CC16?=$(WATCOM)/binl64/wcc
LD16?=$(WATCOM)/binl64/wlink
CFLAGS16?=-4 -d3 -s -wx -ms -zl -zq -za99
ASMFLAGS?=-f obj

SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/kernel/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.obj, $(SOURCES_ASM))

.PHONY: all kernel clean

all: kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	$(LD16) NAME $(BUILD_DIR)/kernel.bin FILE \{ $(OBJECTS_ASM) $(OBJECTS_C) \} OPTION MAP=$(BUILD_DIR)/kernel.map @linker.lnk

$(BUILD_DIR)/kernel/c/%.obj: %.c
	@mkdir -p $(@D)
	$(CC16) $(CFLAGS16) -fo=$@ $<

$(BUILD_DIR)/kernel/asm/%.obj: %.asm
	@mkdir -p $(@D)
	$(ASM) $(ASMFLAGS) -o $@ $<

clean:
	rm -f $(BUILD_DIR)/kernel.bin
