BUILD_DIR?=build/
ASM?=nasm
WATCOM?=$(HOME)/watcom
CC16?=$(WATCOM)/binl64/wcc
LD16?=$(WATCOM)/binl64/wlink
CFLAGS16?=-4 -d3 -s -wx -ms -zl -zq -za99
ASMFLAGS?=-f obj

SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/stage2/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/stage2/asm/%.obj, $(SOURCES_ASM))

.PHONY: all stage2 clean always

all: stage2

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	$(LD16) NAME $(BUILD_DIR)/stage2.bin FILE \{ $(OBJECTS_ASM) $(OBJECTS_C) \} OPTION MAP=$(BUILD_DIR)/stage2.map @linker.lnk
	@echo "--> Created stage2.bin"

$(BUILD_DIR)/stage2/c/%.obj: %.c
	@mkdir -p $(@D)
	$(CC16) $(CFLAGS16) -fo=$@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/stage2/asm/%.obj: %.asm
	@mkdir -p $(@D)
	$(ASM) $(ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean: 
	rm -f $(BUILD_DIR)/stage2.bin
