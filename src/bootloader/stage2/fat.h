#pragma once
#include "stdint.h"
#include "disk.h"
#pragma pack(push, 1)

typedef struct{
    uint8_t Name[11];
    uint8_t Attributes;
    uint8_t _Reserved;
    uint8_t CreatedTimeTenths;
    uint16_t CreatedTime;
    uint16_t CreatedDate;
    uint16_t AccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t ModifiedTime;
    uint16_t ModifiedDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} FAT_DirectoryEntry;

#pragma pack(pop)

typedef struct{
    int Handle;
    bool IsDirectory;
    uint32_t Position;
    uint32_t Size;
} FAT_File;

enum FAT_Attributes{
    FAT_ATTRIBUTE_READ_ONLY     = 0x01,
    FAT_ATTRIBUTE_HIDDEN        = 0x02,
    FAT_ATTRIBUTE_SYSTEM        = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID     = 0x08,
    FAT_ATTRIBUTE_DIRECTORY     = 0x10,
    FAT_ATTRIBUTE_ARCHIVE       = 0x20,
    FAT_ATTRIBUTE_LFN           = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

// Initializes the FAT12 reader for the selected disk.
bool FAT_Initialize(DISK* disk);

// Opens a root-relative path and returns a bootloader file handle.
FAT_File far* FAT_Open(DISK* disk, const char* path);

// Reads bytes from an open FAT file into caller-provided memory.
uint32_t FAT_Read(DISK* disk, FAT_File far* file, uint32_t byteCount, void far* dataOut);

// Reads one directory entry from an open directory handle.
bool FAT_ReadEntry(DISK* disk, FAT_File far* file, FAT_DirectoryEntry* dataOut);

// Releases or rewinds a FAT file handle.
void FAT_Close(FAT_File far* file);
