#include "fat.h"
#include "stdio.h"
#include "memdefs.h"
#include "utility.h"
#include "string.h"
#include "memory.h"
#include "ctype.h"
#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1
#define min(a, b) ((a) < (b) ? (a) : (b))
#pragma pack(push, 1)

typedef struct{
    uint8_t BootJumpINstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptionType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    //extended boot record
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;
    uint8_t VolumeLabel[11];
    uint8_t SystemId[8];
    
} FAT_BootSector;

#pragma pack(pop)

typedef struct{
    FAT_File Public;
    bool Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;
    uint8_t Buffer[SECTOR_SIZE];
} FAT_FileData;


typedef struct{
    union{
        FAT_BootSector FAT_BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;

    FAT_FileData RootDirectory;

    FAT_FileData OpenedFiles[MAX_FILE_HANDLES];
    
} FAT_Data;

static FAT_Data far* g_Data;
static uint8_t far* g_Fat = NULL;
static uint32_t g_DataSectionLba;

// Reads the FAT boot sector into the shared FAT workspace.
bool FAT_ReadBootSector(DISK* disk){
    return DISK_ReadSectors(disk, 0, 1, &g_Data->BS.BootSectorBytes);
}

// Loads the first FAT table so cluster chains can be followed in memory.
bool FAT_ReadFat(DISK* disk){
    return DISK_ReadSectors(disk, g_Data->BS.FAT_BootSector.ReservedSectors, g_Data->BS.FAT_BootSector.SectorsPerFat, g_Fat);
}

// Initializes the FAT12 reader, root directory handle, and shared filesystem buffers.
bool FAT_Initialize(DISK* disk){

    g_Data = (FAT_Data far*)MEMORY_FAT_ADDRESS;

    if (!FAT_ReadBootSector(disk)){
        printf("FAT: read boot sector failed\r\n");
        return false;
    }

    g_Fat = (uint8_t far*)g_Data + sizeof(FAT_Data);
    uint32_t fatSize = g_Data->BS.FAT_BootSector.BytesPerSector * g_Data->BS.FAT_BootSector.SectorsPerFat;

    if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE){
        printf("Fat: not enough memory to read FAT! Required %u, only have %lu\r\n", sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
        return false;
    }

    if (!FAT_ReadFat(disk)){
        printf("FAT: read FAT failed\r\n");
        return false;
    }

    uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BS.FAT_BootSector.DirEntryCount;
    uint32_t rootDirLba = g_Data->BS.FAT_BootSector.ReservedSectors + g_Data->BS.FAT_BootSector.SectorsPerFat * g_Data->BS.FAT_BootSector.FatCount;
    uint32_t rootDirSectors = (rootDirSize + g_Data->BS.FAT_BootSector.BytesPerSector - 1) / g_Data->BS.FAT_BootSector.BytesPerSector;

    g_Data->RootDirectory.Opened = true;
    g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    g_Data->RootDirectory.Public.IsDirectory = true;
    g_Data->RootDirectory.Public.Position = 0;
    g_Data->RootDirectory.Public.Size = rootDirSize;
    g_Data->RootDirectory.CurrentCluster = rootDirLba;
    g_Data->RootDirectory.FirstCluster = rootDirLba;
    g_Data->RootDirectory.CurrentSectorInCluster = 0;

    if (!DISK_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer)){
        printf("FAT: read root directory failed\r\n");
        return false;
    }

    g_DataSectionLba = rootDirLba + rootDirSectors;

    for (int i = 0; i < MAX_FILE_HANDLES; i++){
        g_Data->OpenedFiles[i].Opened = false;
    }

    return true;
}

// Converts a FAT cluster number into the disk LBA where that cluster begins.
uint32_t FAT_ClusterToLba(uint32_t cluster){
    return g_DataSectionLba + (cluster - 2) * g_Data->BS.FAT_BootSector.SectorsPerCluster;
}

// Allocates a file handle from a directory entry and reads its first sector.
FAT_File far* FAT_OpenEntry(DISK* disk, FAT_DirectoryEntry* entry){
    int handle = -1;
    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++){
        if (!g_Data->OpenedFiles[i].Opened){
            handle = i;
        }
    }

    if (handle < 0){
        printf("FAT: out of file handles\r\n");
        return NULL;
    }

    FAT_FileData far* fd = &g_Data->OpenedFiles[handle];
    fd->Public.Handle = handle;
    fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->Public.Position = 0;
    fd->Public.Size = entry->Size;
    fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
    fd->CurrentCluster = fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    if (!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer)){
        printf("FAT: read error\r\n");
        return NULL;
    }

    fd->Opened = true;
    return &fd->Public;
}

// Looks up the next cluster in a FAT12 cluster chain.
uint32_t FAT_NextCluster(uint32_t currentCluster){
    uint32_t fatIndex = currentCluster * 3 / 2;
    uint16_t far* fatEntry = (uint16_t far*)(g_Fat + fatIndex);

    if (currentCluster % 2 == 0)
        return *fatEntry & 0x0FFF;
    else
        return *fatEntry >> 4;
}

// Reads bytes from an open file, advancing through sectors and FAT cluster chains as needed.
uint32_t FAT_Read(DISK* disk, FAT_File far* file, uint32_t byteCount, void far* dataOut){
    FAT_FileData far* fd = (file->Handle == ROOT_DIRECTORY_HANDLE) ? &g_Data->RootDirectory : &g_Data->OpenedFiles[file->Handle];
    uint8_t far* u8DataOut = (uint8_t far*)dataOut;
    uint32_t bytesRead = 0;

    byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);

    while (byteCount > 0){
        uint32_t leftInBuffer = SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
        uint32_t take = min(byteCount, leftInBuffer);

        memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
        u8DataOut += take;
        fd->Public.Position += take;
        byteCount -= take;
        bytesRead += take;

        if(fd->Public.Position < fd->Public.Size && (fd->Public.Position % SECTOR_SIZE) == 0){
            if(fd->Public.Handle == ROOT_DIRECTORY_HANDLE){
                ++fd->CurrentCluster;
                if (!DISK_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer)){
                    printf("FAT: read error!\r\n");
                    break;
                }
            }else{
                if (++fd->CurrentSectorInCluster >= g_Data->BS.FAT_BootSector.SectorsPerCluster){
                    fd->CurrentSectorInCluster = 0;
                    fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
                }

                if (fd->CurrentCluster >= 0xFF8){
                    printf("FAT: read error! Invalid next cluster!\r\n");
                    break;
                }

                if (!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer)){
                    printf("FAT: read error!\r\n");
                    break;
                }
            }
        }
    }

    return bytesRead;
}

// Reads a single directory entry from an open directory file handle.
bool FAT_ReadEntry(DISK* disk, FAT_File far* file, FAT_DirectoryEntry* dataOut){
    return FAT_Read(disk, file, sizeof(FAT_DirectoryEntry), dataOut) == sizeof(FAT_DirectoryEntry);
}

// Closes a file handle or rewinds the root directory pseudo-handle.
void FAT_Close(FAT_File far* file){
    if (file == NULL){
        return;
    }

    if (file->Handle == ROOT_DIRECTORY_HANDLE){
        file->Position = 0;
        g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    }else{
        g_Data->OpenedFiles[file->Handle].Opened = false;
    }
}

// Searches an open directory for a path component converted into FAT 8.3 format.
bool FAT_findFile(DISK* disk, FAT_File far* file, const char* name, FAT_DirectoryEntry* entryOut){
    char fatName[11];
    FAT_DirectoryEntry entry;

    memset(fatName, ' ', sizeof(fatName));
    const char* ext = strchr(name, '.');
    const char* nameEnd = ext;
    if (nameEnd == NULL){
        nameEnd = name + strlen(name);
    }

    for (int i = 0; i < 8 && name + i < nameEnd; i++){
        fatName[i] = toupper(name[i]);
    }

    if (ext != NULL){
        ext++;
        for (int i = 0; i < 3 && ext[i]; i++){
            fatName[8 + i] = toupper(ext[i]);
        }
    }

    while (FAT_ReadEntry(disk, file, &entry)){
        if (entry.Name[0] == 0){
            break;
        }

        if (entry.Name[0] == 0xE5 || entry.Attributes == FAT_ATTRIBUTE_LFN){
            continue;
        }

        if (memcmp(fatName, entry.Name, 11) == 0){
            *entryOut = entry;
            return true;
        }
    }

    return false;
}

// Opens a root-relative FAT path by walking directory entries one component at a time.
FAT_File far* FAT_Open(DISK* disk, const char* path){
    char name[MAX_PATH_SIZE];

    if (path[0] == '/'){
        path++;
    }

    FAT_File far* parent = NULL;
    FAT_File far* current = &g_Data->RootDirectory.Public;
    current->Position = 0;
    g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    if (!DISK_ReadSectors(disk, g_Data->RootDirectory.FirstCluster, 1, g_Data->RootDirectory.Buffer)){
        printf("FAT: read root directory failed\r\n");
        return NULL;
    }

    while (*path) {
        bool isLast = false;
        const char* delim = strchr(path, '/');
        if (delim != NULL){
            uint16_t len = delim - path;
            memcpy(name, path, len);
            name[len] = '\0';
            path = delim + 1;
        }else{
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len] = '\0';
            path += len;
            isLast = true;
        }

        FAT_DirectoryEntry entry;
        if (FAT_findFile(disk, current, name, &entry)){
            FAT_Close(parent);

            if (!isLast && (entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) == 0){
                printf("FAT: %s not a directory\r\n", name);
                return NULL;
            }

            parent = current;
            current = FAT_OpenEntry(disk, &entry);
            if (current == NULL){
                return NULL;
            }
        }else{
            FAT_Close(current);
            printf("FAT: %s not found\r\n", name);
            return NULL;
        }
    }

    return current;
}


