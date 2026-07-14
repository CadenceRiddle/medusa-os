#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"
#include "x86.h"
#include "../../bootinfo.h"

#define KERNEL_LOAD_SEGMENT 0x1000
#define KERNEL_LOAD_OFFSET 0x0000
#define KERNEL_LOAD_ADDRESS ((uint8_t far*)0x10000000)
#define BOOTINFO_LOAD_SEGMENT 0x9000
#define BOOTINFO_LOAD_OFFSET 0x0000
#define BOOTINFO_LOAD_ADDRESS ((BootInfo far*)0x90000000)

void _cdecl cstart_(uint16_t bootDrive){
    static BootInfo bootInfo;

    DISK disk;
    if(!DISK_Initialize(&disk, bootDrive)){
        printf("Disk init error\r\n");
        goto end;
    }

    if (!FAT_Initialize(&disk)){
        printf("FAT init error\r\n");
        goto end;
    }

    printf("Loading kernel...\r\n");
    FAT_File far* fd = FAT_Open(&disk, "/kernel.bin");
    if (fd == NULL){
        printf("Could not open KERNEL.BIN\r\n");
        goto end;
    }

    uint32_t bytesRead = FAT_Read(&disk, fd, fd->Size, KERNEL_LOAD_ADDRESS);
    if (bytesRead != fd->Size){
        printf("Kernel read error\r\n");
        FAT_Close(fd);
        goto end;
    }
    FAT_Close(fd);

    bootInfo.bootDrive = bootDrive;
    bootInfo.kernelSegment = KERNEL_LOAD_SEGMENT;
    bootInfo.kernelOffset = KERNEL_LOAD_OFFSET;
    if (!x86_GetMemoryMap(bootInfo.memoryMapEntries, BOOTINFO_MEMORY_MAP_MAX_ENTRIES, &bootInfo.memoryMapEntryCount)){
        printf("Memory map unavailable\r\n");
    }

    *BOOTINFO_LOAD_ADDRESS = bootInfo;

    printf("Jumping to kernel...\r\n");
    x86_FarJumpWithBootInfo(KERNEL_LOAD_SEGMENT, KERNEL_LOAD_OFFSET, BOOTINFO_LOAD_SEGMENT, BOOTINFO_LOAD_OFFSET);

end:
    for(;;);
}
