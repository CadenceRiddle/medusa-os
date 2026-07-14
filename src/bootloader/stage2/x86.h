#include "stdint.h"
#pragma once

#include "../../bootinfo.h"

// Writes one character through BIOS teletype output.
void _cdecl x86_Video_WriteCharTeletype(char c, uint8_t page);

// Divides a 64-bit value by a 32-bit divisor for code that lacks native support.
void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotientOut, uint32_t* remainderOut);

// Resets the selected BIOS disk drive.
bool _cdecl x86_Disk_Reset(uint8_t drive);

// Reads sectors from a BIOS disk using cylinder/head/sector addressing.
bool _cdecl x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, uint8_t far * dataOut);

// Queries BIOS disk geometry for the selected drive.
bool _cdecl x86_Disk_GetDriveParams(uint8_t drive, uint8_t* driveTypeOut, uint16_t* cylindersOut, uint16_t* sectorsOut, uint16_t* headsOut);

// Reads BIOS E820 memory map entries into a far buffer.
bool _cdecl x86_GetMemoryMap(BootMemoryMapEntry far* entries, uint16_t maxEntries, uint16_t* entryCountOut);

// Far-jumps to a real-mode segment and offset.
void _cdecl x86_FarJump(uint16_t segment, uint16_t offset);

// Sets ES:BX to BootInfo and far-jumps to the kernel entry.
void _cdecl x86_FarJumpWithBootInfo(uint16_t segment, uint16_t offset, uint16_t bootInfoSegment, uint16_t bootInfoOffset);
