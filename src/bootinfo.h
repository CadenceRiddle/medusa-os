#pragma once

#define BOOTINFO_MEMORY_MAP_MAX_ENTRIES 16

#pragma pack(push, 1)

typedef struct {
    uint32_t baseLow;
    uint32_t baseHigh;
    uint32_t lengthLow;
    uint32_t lengthHigh;
    uint32_t type;
    uint32_t acpiAttributes;
} BootMemoryMapEntry;

typedef struct {
    uint16_t bootDrive;
    uint16_t kernelSegment;
    uint16_t kernelOffset;
    uint16_t memoryMapEntryCount;
    BootMemoryMapEntry memoryMapEntries[BOOTINFO_MEMORY_MAP_MAX_ENTRIES];
} BootInfo;

#pragma pack(pop)
