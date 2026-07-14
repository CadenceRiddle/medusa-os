#pragma once

#define BOOTINFO_MEMORY_MAP_MAX_ENTRIES 16

#pragma pack(push, 1)

// Describes one BIOS E820 physical memory range.
typedef struct {
    uint32_t baseLow;
    uint32_t baseHigh;
    uint32_t lengthLow;
    uint32_t lengthHigh;
    uint32_t type;
    uint32_t acpiAttributes;
} BootMemoryMapEntry;

// Carries bootloader-provided context into the protected-mode kernel.
typedef struct {
    uint16_t bootDrive;
    uint16_t kernelSegment;
    uint16_t kernelOffset;
    uint16_t memoryMapEntryCount;
    BootMemoryMapEntry memoryMapEntries[BOOTINFO_MEMORY_MAP_MAX_ENTRIES];
} BootInfo;

#pragma pack(pop)
