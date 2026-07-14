#pragma once

#include "stdint.h"

typedef struct {
    uint8_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
} DISK;

// Initializes BIOS disk metadata for the selected boot drive.
bool DISK_Initialize(DISK* disk, uint8_t driveNumber);

// Reads sectors from an LBA address into a far memory buffer.
bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, uint8_t far* dataOut);
