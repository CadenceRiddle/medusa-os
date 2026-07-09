#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"
#include "x86.h"

#define KERNEL_LOAD_SEGMENT 0x1000
#define KERNEL_LOAD_OFFSET 0x0000
#define KERNEL_LOAD_ADDRESS ((uint8_t far*)0x10000000)

void _cdecl cstart_(uint16_t bootDrive){
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

    printf("Jumping to kernel...\r\n");
    x86_FarJump(KERNEL_LOAD_SEGMENT, KERNEL_LOAD_OFFSET);

end:
    for(;;);
}
