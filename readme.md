# Medusa

Medusa is a small operating-system learning project. Right now, it is mostly a bootloader project: it builds a floppy disk image, boots it in an emulator, loads a second-stage bootloader from a FAT12 filesystem, and then proves that the second stage can run C code by printing text on the screen.

This README explains the project slowly and step by step. It assumes you may be new to operating systems, BIOS booting, assembly language, and C code running outside of a normal operating system.

## What This Project Currently Does

At a high level, the project does this:

1. Builds a 1.44 MB floppy disk image.
2. Formats that image as FAT12, which is an old filesystem commonly used by floppy disks.
3. Writes a first-stage bootloader into the first 512 bytes of the disk.
4. Copies `stage2.bin`, `kernel.bin`, and `test.txt` into the FAT12 filesystem.
5. Boots the floppy image in an emulator such as QEMU or Bochs.
6. The BIOS loads the first-stage bootloader into memory.
7. The first-stage bootloader searches the FAT12 root directory for `STAGE2.BIN`.
8. The first-stage bootloader loads `STAGE2.BIN` into memory.
9. The first-stage bootloader jumps to the second-stage bootloader.
10. The second-stage bootloader sets up a small 16-bit environment for C code.
11. The second-stage bootloader calls a C function named `cstart_`.
12. The C code prints `Hello World from C!` and exercises a tiny local `printf`.
13. The C code loops forever.

The project also builds a separate `kernel.bin`, but the current boot flow loads and jumps into stage 2 first. The current stage 2 code does not yet load and jump into the kernel.

## Important Concepts

### What Is a Bootloader?

When a computer starts, there is no operating system running yet. The CPU begins by running firmware code. On older PC-style systems, that firmware is called the BIOS.

The BIOS does not know how to load a full modern operating system by itself. Instead, it performs a small first step:

1. It finds a bootable disk.
2. It reads the first sector from that disk.
3. It places that sector at memory address `0x7C00`.
4. It jumps to that address and starts executing the code there.

That first sector is the bootloader.

The first sector is only 512 bytes, so there is not much room. Because of that, many operating systems use multiple bootloader stages:

- Stage 1 is tiny and fits in the first 512 bytes.
- Stage 2 is larger and can contain more complicated logic.
- Later stages may load the actual kernel.

This project follows that pattern.

### What Is Real Mode?

The bootloader runs in 16-bit real mode.

Real mode is the CPU mode used by old x86 PCs when they first boot. In real mode:

- The CPU uses 16-bit registers for most operations.
- Memory is addressed using segment and offset pairs, such as `2000:0000`.
- BIOS services are available through interrupts like `int 10h` and `int 13h`.
- There is no memory protection.
- There is no operating system to help with files, printing, memory allocation, or errors.

Because there is no operating system yet, the code cannot call normal library functions like `printf` from the host system. Anything the bootloader needs must either be written manually or provided by BIOS interrupts.

### What Is FAT12?

FAT12 is a simple filesystem used by floppy disks.

A filesystem is the structure that lets files exist on a disk. Without a filesystem, the disk is just raw bytes. FAT12 lets the project store files like:

- `STAGE2.BIN`
- `KERNEL.BIN`
- `TEST.TXT`

The first-stage bootloader understands just enough FAT12 to find and load `STAGE2.BIN`.

## Project Layout

```text
.
├── Makefile
├── run.sh
├── debug.sh
├── bochs_config
├── test.txt
├── tools/
│   └── fat/
│       └── fat.c
└── src/
    ├── bootloader/
    │   ├── stage1/
    │   │   ├── Makefile
    │   │   └── boot.asm
    │   └── stage2/
    │       ├── Makefile
    │       ├── linker.lnk
    │       ├── main.asm
    │       ├── main.c
    │       ├── stdio.c
    │       ├── stdio.h
    │       ├── stdint.h
    │       ├── x86.asm
    │       └── x86.h
    └── kernel/
        ├── Makefile
        └── main.asm
```

The most important files are:

- `Makefile`: Builds the floppy image, bootloaders, kernel, and helper tool.
- `src/bootloader/stage1/boot.asm`: The first 512-byte boot sector.
- `src/bootloader/stage2/main.asm`: The assembly entry point for stage 2.
- `src/bootloader/stage2/main.c`: The C entry point for stage 2.
- `src/bootloader/stage2/stdio.c`: Very small text output functions for stage 2.
- `src/bootloader/stage2/x86.asm`: BIOS interrupt wrapper used by C code.
- `src/kernel/main.asm`: A simple kernel stub.
- `tools/fat/fat.c`: A helper program for reading a file from the FAT12 disk image.

## Build Process

The root `Makefile` controls the overall build.

When you run:

```sh
make
```

the project builds:

1. `build/stage1.bin`
2. `build/stage2.bin`
3. `build/kernel.bin`
4. `build/main_floppy.img`
5. `build/tools/fat`

### Step 1: Build Stage 1

The stage 1 bootloader is built from:

```text
src/bootloader/stage1/boot.asm
```

Its Makefile runs NASM:

```sh
nasm boot.asm -f bin -o build/stage1.bin
```

This produces a flat binary file. A flat binary is raw machine code with no executable-file wrapper around it. That is important because the BIOS expects raw boot code in the first disk sector.

### Step 2: Build Stage 2

Stage 2 is built from both assembly and C files:

```text
src/bootloader/stage2/main.asm
src/bootloader/stage2/x86.asm
src/bootloader/stage2/main.c
src/bootloader/stage2/stdio.c
```

The assembly files are assembled into object files.

The C files are compiled with Open Watcom into 16-bit object files.

Then the linker combines all of those object files into:

```text
build/stage2.bin
```

The linker script is:

```text
src/bootloader/stage2/linker.lnk
```

It tells the linker to produce a raw binary and start at the symbol named `entry`.

### Step 3: Build the Kernel Stub

The kernel is currently built from:

```text
src/kernel/main.asm
```

It becomes:

```text
build/kernel.bin
```

At the moment, this kernel is a small 16-bit assembly program that prints a message and halts. The current stage 2 bootloader does not yet load this kernel.

### Step 4: Create the Floppy Image

The root Makefile creates a blank floppy image:

```sh
dd if=/dev/zero of=build/main_floppy.img bs=512 count=2880
```

This creates 2880 sectors of 512 bytes each.

```text
2880 * 512 = 1,474,560 bytes
```

That is the size of a standard 1.44 MB floppy disk.

### Step 5: Format the Image as FAT12

The image is formatted as FAT12:

```sh
mkfs.fat -F 12 -n "NBOS" build/main_floppy.img
```

This gives the image a filesystem so files can be copied into it.

### Step 6: Install Stage 1 Into the Boot Sector

The first-stage bootloader is written directly into the beginning of the floppy image:

```sh
dd if=build/stage1.bin of=build/main_floppy.img conv=notrunc
```

This overwrites the first sector with the bootloader code, while keeping the rest of the disk image intact.

The `conv=notrunc` part means "do not truncate the output file." Without that, `dd` could shrink the floppy image down to only the size of `stage1.bin`.

### Step 7: Copy Files Into the FAT12 Image

The Makefile uses `mcopy` to copy files into the image:

```sh
mcopy -i build/main_floppy.img build/stage2.bin "::stage2.bin"
mcopy -i build/main_floppy.img build/kernel.bin "::kernel.bin"
mcopy -i build/main_floppy.img test.txt "::test.txt"
```

These files are now visible inside the floppy image's FAT12 filesystem.

## Boot Flow

This section follows what happens after the image boots.

### Step 1: BIOS Loads Stage 1

When the emulator boots the floppy image, the BIOS reads the first sector from the disk and places it at:

```text
0x7C00
```

The code in `src/bootloader/stage1/boot.asm` starts with:

```asm
org 0x7C00
bits 16
```

`org 0x7C00` tells the assembler that this code expects to live at memory address `0x7C00`.

`bits 16` tells NASM to generate 16-bit x86 instructions.

### Step 2: Stage 1 Contains a FAT12 Boot Header

Near the top of `boot.asm`, there are fields like:

```asm
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_clusters:   db 1
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 0E0h
bdb_total_sectors:          dw 2880
bdb_sectors_per_fat:        dw 9
```

These values describe the FAT12 disk layout.

This matters because the disk image is formatted as FAT12. The bootloader needs to know where the root directory is, where the FAT tables are, and how large sectors and clusters are.

### Step 3: Stage 1 Sets Up Segments and Stack

Early in `boot.asm`, stage 1 sets important segment registers:

```asm
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7C00
```

In real mode, memory addresses are built from segment registers and offsets. The bootloader sets `DS`, `ES`, and `SS` to `0` so it can use predictable addresses.

It also sets the stack pointer to `0x7C00`. The stack is memory used for temporary storage, function calls, return addresses, and saved registers.

### Step 4: Stage 1 Saves the Boot Drive

The BIOS passes the boot drive number in the `DL` register.

For example:

- `0x00` usually means the first floppy drive.
- `0x80` usually means the first hard drive.

Stage 1 saves that value:

```asm
mov [ebr_drive_number], dl
```

This is important because later disk reads need to know which BIOS drive to read from.

### Step 5: Stage 1 Prints a Loading Message

Stage 1 prints:

```text
Loading...
```

It does this using BIOS interrupt `int 10h`, function `0x0E`.

BIOS interrupts are like tiny firmware services. Since there is no operating system yet, the bootloader asks the BIOS to do basic things like print characters or read sectors from disk.

### Step 6: Stage 1 Reads Disk Geometry

Stage 1 calls BIOS interrupt `int 13h`, function `08h`, to get disk geometry:

```asm
mov ah, 08h
int 13h
```

Disk geometry includes things like:

- sectors per track
- number of heads

The bootloader uses those values when converting logical sector numbers into BIOS-style cylinder/head/sector addresses.

### Step 7: Stage 1 Finds the FAT12 Root Directory

The root directory is where FAT12 stores the list of files in the top-level directory.

Stage 1 calculates where the root directory begins by using:

- reserved sectors
- number of FATs
- sectors per FAT
- number of root directory entries
- bytes per sector

Then it reads the root directory into memory at a buffer.

### Step 8: Stage 1 Searches for `STAGE2.BIN`

FAT12 stores filenames in an 8.3 format:

```text
STAGE2  BIN
```

That is 8 bytes for the name and 3 bytes for the extension. Notice the two spaces between `STAGE2` and `BIN`; that pads the filename to 8 characters.

Stage 1 compares each root directory entry against:

```asm
file_stage2_bin: db 'STAGE2  BIN'
```

If it finds the file, it reads the file's first cluster number from the directory entry.

### Step 9: Stage 1 Reads the FAT

The FAT, or File Allocation Table, tells the bootloader which clusters belong to a file.

A file may not be stored in one continuous block on disk. FAT12 stores a chain of clusters. Each cluster points to the next cluster.

Stage 1 reads the FAT into memory so it can follow the cluster chain for `STAGE2.BIN`.

### Step 10: Stage 1 Loads `STAGE2.BIN`

Stage 1 loads `STAGE2.BIN` into memory at:

```text
2000:0000
```

In real mode, this means:

```text
physical address = segment * 16 + offset
physical address = 0x2000 * 16 + 0x0000
physical address = 0x20000
```

So stage 2 is loaded at physical memory address `0x20000`.

The code uses:

```asm
KERNEL_LOAD_SEGMENT equ 0x2000
KERNEL_LOAD_OFFSET equ 0
```

The name says `KERNEL`, but in the current stage 1 code, this is actually where `STAGE2.BIN` is loaded.

### Step 11: Stage 1 Jumps to Stage 2

After loading `STAGE2.BIN`, stage 1 jumps to it:

```asm
jmp KERNEL_LOAD_SEGMENT:KERNEL_LOAD_OFFSET
```

This is a far jump. A far jump changes both:

- the code segment register `CS`
- the instruction pointer `IP`

After this jump, the CPU starts executing stage 2 at:

```text
2000:0000
```

## Stage 2 Bootloader

Stage 2 is split between assembly and C.

The assembly file starts execution first:

```text
src/bootloader/stage2/main.asm
```

The C file contains the main stage 2 logic:

```text
src/bootloader/stage2/main.c
```

### Why Stage 2 Starts in Assembly

Even though stage 2 wants to run C code, it cannot immediately start in C.

C code expects a few things to already be true:

- There should be a valid stack.
- Segment registers should point somewhere sensible.
- Function arguments should be passed using the expected calling convention.
- The linker must know where the C function lives.

The assembly entry point prepares those things.

### Stage 2 Assembly Entry Point

Stage 2 begins at the symbol named `entry`:

```asm
extern _cstart_
global entry

entry:
    cli
    push cs
    pop ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0
    mov bp, sp
    sti
```

`global entry` makes the `entry` label visible to the linker.

`extern _cstart_` tells the assembler that `_cstart_` exists somewhere else. It is defined by the C file after compilation.

The code copies `CS` into `DS`, `ES`, and `SS`. This makes the code, data, and stack segments point to the same stage 2 memory area.

### What `cli` and `sti` Mean

`cli` means "clear interrupt flag." It disables maskable hardware interrupts.

`sti` means "set interrupt flag." It enables maskable hardware interrupts again.

The code disables interrupts while changing the stack segment and stack pointer because an interrupt arriving during stack setup could use a half-configured stack.

### How Stage 2 Passes the Boot Drive to C

Stage 1 preserves the BIOS boot drive in `DL` before jumping to stage 2.

Stage 2 does this:

```asm
xor dh, dh
push dx
call _cstart_
```

`DL` contains the boot drive number. `DH` is the upper half of `DX`.

`xor dh, dh` clears `DH`, so `DX` becomes a clean 16-bit value containing the boot drive number.

`push dx` places that value on the stack.

Then `call _cstart_` calls the C function.

### Why the C Function Is Named `cstart_`

In C, the function is:

```c
void _cdecl cstart_(uint16_t bootDrive)
```

Open Watcom adds a leading underscore to C symbols, so the assembly code sees this function as:

```asm
_cstart_
```

That is why `main.asm` calls `_cstart_`.

The `_cdecl` part specifies the calling convention. A calling convention is an agreement about how functions receive arguments, how return values work, and who cleans up the stack.

In this case, the argument is passed on the stack, which matches the assembly code's `push dx`.

### What the Stage 2 C Code Does

The current C function is small, but it now tests both plain string output and formatted output:

```c
void _cdecl cstart_(uint16_t bootDrive){
    puts("Hello World from C!\r\n");
    printf("Formatted %% %c %s\r\n", 'a', "string");
    printf("Formatted %d %i %x %p %o %hd %hi %hhu %hhu\r\n",
           1234, 5678, 0xdead, 0xbeef, 012345,
           (short)27, (short)-42, (unsigned char)20, (char)-10);
    printf("Formatted %ld %lx %lld %llx\r\n",
           -100000000l, 0xdeadbeeful,
           10200300400ll, 0xdeadbeeffeebdaedull);
    for(;;);
}
```

It receives the boot drive number as `bootDrive`.

It currently does not use `bootDrive`, but future code could use it to keep reading from the same disk.

Then it prints:

```text
Hello World from C!
```

After that, it calls the project-local `printf` implementation to test:

- literal percent output with `%%`
- characters with `%c`
- strings with `%s`
- signed decimal integers with `%d` and `%i`
- unsigned decimal integers with `%u`
- hexadecimal integers and pointers with `%x`, `%X`, and `%p`
- octal integers with `%o`
- short and char-sized integer modifiers with `%h` and `%hh`
- long and long-long modifiers with `%l` and `%ll`

Finally, it enters an infinite loop:

```c
for(;;);
```

That keeps the CPU from returning into unknown code.

## How Printing Works in Stage 2

There is no normal C standard library here. This project provides its own tiny `putc`, `puts`, and `printf`.

### `puts`

In `src/bootloader/stage2/stdio.c`:

```c
void puts(const char* str){
    while(*str){
        putc(*str);
        str++;
    }
}
```

This walks through a C string one character at a time.

A C string ends with a zero byte, also called the null terminator. The loop continues until it reaches that zero byte.

### `putc`

Also in `stdio.c`:

```c
void putc(char c){
    x86_Video_WriteCharTeletype(c, 0);
}
```

`putc` prints one character by calling an assembly function.

### `printf`

The local `printf` is intentionally small. It is not a full hosted C library implementation, but it supports enough formatting to test variadic arguments in the 16-bit stage 2 environment.

Internally, `printf` walks the format string as a small state machine. Normal characters go directly to `putc`. When it sees `%`, it parses an optional length modifier, then dispatches the format specifier.

Numeric formats are converted by repeatedly dividing the value by the requested radix:

- radix 10 for decimal
- radix 16 for hexadecimal and pointers
- radix 8 for octal

Because the code runs in 16-bit real mode, 64-bit division is handled by an assembly helper named `x86_div64_32`.

The formatter also accounts for default C argument promotion. Values passed for `%h` and `%hh` arrive as `int` in the variadic argument list, so the formatter masks or sign-extends them back down before printing.

### BIOS Teletype Output

In `src/bootloader/stage2/x86.asm`, the assembly wrapper uses BIOS interrupt `int 10h`:

```asm
mov ah, 0Eh
mov al, [bp + 4]
mov bh, [bp + 6]
int 10h
```

Function `0Eh` is BIOS teletype output. It prints a character to the screen and advances the cursor.

This is how the C code is able to display text even though there is no operating system and no real C standard library.

The wrapper is called from C using `_cdecl`, so the character and page arguments are read from the stack instead of from arbitrary registers.

### 64-Bit Division Helper

Formatted number output uses this assembly helper:

```c
void _cdecl x86_div64_32(
    uint64_t dividend,
    uint32_t divisor,
    uint64_t* quotientOut,
    uint32_t* remainderOut
);
```

It divides a 64-bit value by a 32-bit radix and writes both the quotient and the remainder back through pointers. `printf` uses the remainder as the next digit and repeats until the quotient reaches zero.

## What the Linker Does

Stage 2 has multiple source files:

- assembly files
- C files
- header files
- a linker script

The assembler and compiler do not directly create the final stage 2 binary. They create object files first.

Object files contain compiled code, but they may still have unresolved references.

For example, `main.asm` says:

```asm
extern _cstart_
call _cstart_
```

That means:

```text
I want to call a function named _cstart_, but I do not know its address yet.
```

The C compiler produces an object file that contains `_cstart_`.

The linker combines the object files and connects references to definitions.

The stage 2 linker script says:

```text
FORMAT RAW BIN
START=entry
OFFSET=0
```

This means:

- produce a raw binary file
- use `entry` as the starting symbol
- place the binary as if it starts at offset `0`

The linker also orders code sections:

```text
SEGMENT _ENTRY
SEGMENT _TEXT
```

This helps ensure the entry code comes before the rest of the text/code section.

## Kernel Stub

The kernel file is:

```text
src/kernel/main.asm
```

It is also 16-bit assembly. It:

1. Sets up segment registers.
2. Sets up a stack.
3. Prints:

```text
Hello World from the Boot Loader!
```

4. Halts.

The kernel currently looks like a simple test program. The project builds it and copies it into the floppy image, but stage 2 does not yet load and execute it.

## Helper FAT Tool

The file:

```text
tools/fat/fat.c
```

builds into:

```text
build/tools/fat
```

This helper can open the floppy disk image, read the FAT12 structures, find a file, and print that file's contents.

It is useful for testing whether files were copied into the image correctly.

## Running the Project

The `run.sh` script does:

```sh
make
qemu-system-i386 -drive file=build/main_floppy.img,if=floppy,format=raw -boot a -display gtk,gl=off
```

So it builds the image and then starts QEMU with the floppy image attached as drive A.

If everything works, the boot process should eventually display:

```text
Loading...
Hello World from C!
Formatted % a string
Formatted 1234 5678 dead beef 12345 27 -42 20 246
Formatted -100000000 deadbeef 10200300400 deadbeeffeebdaed
```

The exact display may depend on emulator behavior and how the BIOS screen is initialized.

## Debugging With Bochs

The project also includes:

```text
debug.sh
bochs_config
```

`debug.sh` starts Bochs:

```sh
bochs -f bochs_config
```

Bochs is often useful for OS development because it gives detailed debugging tools for low-level boot code.

## Current Limitations

This project is still early. Some important limitations are:

- Stage 2 prints plain and formatted text from C, but does not yet load the kernel.
- The kernel is copied into the floppy image, but is not currently reached by the active boot path.
- The project stays in 16-bit real mode.
- There is no protected mode or long mode setup yet.
- There is no memory manager.
- There is no interrupt descriptor table.
- There are no drivers beyond BIOS-based screen and disk access.
- There is no filesystem support in C yet.
- The C standard library is not available; only tiny local replacements exist.
- The local `printf` supports common integer/string formats, but not field width, precision, floating point, dynamic allocation, or locale behavior.

These limitations are normal for an early operating-system project. The current code is focused on proving the basic boot path.

## Step-by-Step Summary

Here is the complete flow in one list:

1. You run `make` or `run.sh`.
2. NASM builds `stage1.bin` from `src/bootloader/stage1/boot.asm`.
3. NASM builds stage 2 assembly object files.
4. Open Watcom builds stage 2 C object files.
5. The linker combines stage 2 objects into `stage2.bin`.
6. NASM builds `kernel.bin` from `src/kernel/main.asm`.
7. A blank 1.44 MB image is created.
8. The image is formatted as FAT12.
9. `stage1.bin` is written into the first sector.
10. `stage2.bin`, `kernel.bin`, and `test.txt` are copied into the FAT12 image.
11. QEMU or Bochs boots the image.
12. The BIOS loads the first sector to `0x7C00`.
13. Stage 1 starts running in 16-bit real mode.
14. Stage 1 saves the BIOS boot drive number.
15. Stage 1 prints `Loading...`.
16. Stage 1 reads FAT12 disk information.
17. Stage 1 reads the root directory.
18. Stage 1 searches for `STAGE2.BIN`.
19. Stage 1 reads the FAT table.
20. Stage 1 follows the FAT cluster chain for `STAGE2.BIN`.
21. Stage 1 loads stage 2 into memory at `2000:0000`.
22. Stage 1 jumps to `2000:0000`.
23. Stage 2 assembly starts at `entry`.
24. Stage 2 assembly sets up segment registers and a stack.
25. Stage 2 assembly pushes the boot drive number onto the stack.
26. Stage 2 assembly calls the C function `_cstart_`.
27. The C function `cstart_` starts running.
28. C calls `puts` and `printf`.
29. `puts` calls `putc` for each character.
30. `putc` calls an assembly BIOS wrapper.
31. The BIOS wrapper uses `int 10h` to print each character.
32. The screen shows `Hello World from C!` and several formatted output test lines.
33. The C code loops forever.

## Good Next Steps

Natural next steps for this project would be:

- Rename `KERNEL_LOAD_SEGMENT` in stage 1 to something like `STAGE2_LOAD_SEGMENT`, since it currently loads stage 2.
- Expand `printf` only as needed, such as adding width or zero-padding support.
- Make stage 2 load `kernel.bin` from FAT12.
- Move shared disk-reading code into stage 2.
- Add a basic memory map query using BIOS interrupt `int 15h`.
- Switch from real mode to protected mode.
- Build a tiny kernel entry point that stage 2 can jump to.
- Add clearer error messages for missing files and failed reads.

The important milestone already achieved is this: the machine can boot from a generated floppy image, load a second-stage binary, enter C code, and print both plain and formatted text through BIOS services.
