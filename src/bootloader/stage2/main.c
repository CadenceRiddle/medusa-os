#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"

static bool should_print_entry(FAT_DirectoryEntry* entry){
    if (entry->Name[0] == 0)
        return false;

    if (entry->Name[0] == 0xE5)
        return false;

    if ((entry->Attributes & FAT_ATTRIBUTE_LFN) == FAT_ATTRIBUTE_LFN)
        return false;

    if (entry->Attributes & FAT_ATTRIBUTE_VOLUME_ID)
        return false;

    return true;
}

void _cdecl cstart_(uint16_t bootDrive){
    static char buffer[128];

    DISK disk;
    if(!DISK_Initialize(&disk, bootDrive)){
        printf("Disk init error\r\n");
        goto end;
    }

    if (!FAT_Initialize(&disk)){
        printf("FAT init error\r\n");
        goto end;
    }

    FAT_File far* fd = FAT_Open(&disk, "/");
    FAT_DirectoryEntry entry;
    while (FAT_ReadEntry(&disk, fd, &entry)){
        if (entry.Name[0] == 0)
            break;

        if (!should_print_entry(&entry))
            continue;

        printf("    ");
        for (int i = 0; i < 11; i++){
            putc(entry.Name[i]);
        }
        printf("\r\n");
    }
    FAT_Close(fd);

    printf("\r\n--- LARGE.TXT ---\r\n");
    fd = FAT_Open(&disk, "/large.txt");
    if (fd == NULL){
        printf("Could not open LARGE.TXT\r\n");
        goto end;
    }

    uint32_t bytesRead;
    while ((bytesRead = FAT_Read(&disk, fd, sizeof(buffer), buffer)) > 0){
        for (uint32_t i = 0; i < bytesRead; i++){
            char c = buffer[i];
            if (c == '\n')
                putc('\n');
            else if (c == '\r' || c == '\t' || (c >= ' ' && c <= '~'))
                putc(c);
            else
                putc('.');
        }
    }
    FAT_Close(fd);

end:
    for(;;);
}
