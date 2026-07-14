#include "disk.h"
#include "x86.h"

// Queries BIOS for disk geometry and stores the values needed for later reads.
bool DISK_Initialize(DISK* disk, uint8_t driveNumber){
    uint8_t driveType;
    uint16_t cylinders, sectors, heads;

    disk->id = driveNumber;

    if (!x86_Disk_GetDriveParams(disk->id, &driveType, &cylinders, &sectors, &heads))
        return false;

    disk->id = driveNumber;
    disk->cylinders = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;

    return true;
}

// Converts a logical block address into BIOS cylinder/head/sector coordinates.
void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylindersOut, uint16_t* sectorsOut, uint16_t* headsOut){
    *sectorsOut = lba % disk->sectors + 1;

    *cylindersOut = (lba / disk->sectors) / disk->heads;

    *headsOut = (lba / disk->sectors) % disk->heads;
}

// Reads one or more sectors with retry/reset handling for flaky BIOS disk calls.
bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, uint8_t far* dataOut){
    uint16_t cylinder, sector, head;

    DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

    for(int i = 0; i < 3; i++){
        bool ok = x86_Disk_Read(disk->id, cylinder, sector, head, sectors, dataOut);
        if (ok)
            return true;

        ok = x86_Disk_Reset(disk->id);
    }

    return false;
}
