bits 16

section _TEXT class=CODE

global __U4D:
; Open Watcom helper for unsigned 32-bit division in 16-bit code.
__U4D:
    shl edx, 16
    mov dx, ax
    mov eax, edx
    xor edx, edx

    shl ecx, 16
    mov cx, bx

    div ecx
    mov ebx, edx
    mov ecx, edx
    shr ecx, 16

    mov edx, eax
    shr edx, 16

    ret

global __U4M
; Open Watcom helper for unsigned 32-bit multiplication in 16-bit code.
__U4M:
    shl edx, 16
    mov dx, ax
    mov eax, edx

    shl ecx, 16
    mov cx, bx

    mul ecx
    mov edx, eax
    shr edx, 16

    ret

global _x86_div64_32
; Divides a 64-bit value by a 32-bit value for the bootloader printf formatter.
_x86_div64_32:

    push bp
    mov bp, sp

    push bx

    mov eax, [bp + 8]
    mov ecx, [bp + 12]
    xor edx, edx
    div ecx

    mov bx, [bp + 16]
    mov [bx + 4], eax

    mov eax, [bp + 4]
    div ecx

    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx 

    pop bx

    mov sp, bp
    pop bp
    ret

global _x86_Video_WriteCharTeletype
; Calls BIOS int 10h teletype output to print one character.
_x86_Video_WriteCharTeletype:
    push bp
    mov bp, sp

    mov ah, 0Eh
    mov al, [bp + 4]
    mov bh, [bp + 6]

    int 10h

    mov sp, bp
    pop bp
    ret

global _x86_Disk_Reset
; Calls BIOS int 13h to reset the selected disk drive.
_x86_Disk_Reset:
    push bp
    mov bp, sp

    mov ah, 0
    mov dl, [bp + 4]
    stc
    int 13h

    mov ax, 1
    sbb ax, 0

    mov sp, bp
    pop bp
    ret

global _x86_Disk_Read
; Calls BIOS int 13h to read sectors using CHS addressing.
_x86_Disk_Read:
    push bp
    mov bp, sp

    push es
    push bx

    mov dl, [bp + 4]

    mov ch, [bp + 6]
    mov cl, [bp + 7]
    shl cl, 6

    mov dh, [bp + 10]

    mov al, [bp + 8]
    and al, 3Fh
    or cl, al

    mov al, [bp + 12]

    mov bx, [bp + 16]
    mov es, bx
    mov bx, [bp + 14]

    mov ah, 02h

    stc
    int 13h

    mov ax, 1
    sbb ax, 0

    pop bx
    pop es

    mov sp, bp
    pop bp
    ret

global _x86_Disk_GetDriveParams
; Calls BIOS int 13h to query drive geometry used for CHS conversion.
_x86_Disk_GetDriveParams:
    push bp
    mov bp, sp

    push es
    push bx
    push si
    push di

    mov dl, [bp + 4]
    mov ah, 08h
    mov di, 0
    mov es, di
    stc
    int 13h

    mov ax, 1
    sbb ax, 0

    mov si, [bp + 6]
    mov [si], bl

    mov bl, ch
    mov bh, cl
    shr bh, 6
    mov si, [bp + 8]
    mov [si], bx

    xor ch, ch
    and cl, 3Fh
    mov si, [bp + 10]
    mov [si], cx

    mov si, [bp + 8]
    inc word [si]

    mov cl, dh
    mov si, [bp + 12]
    inc cx
    mov [si], cx

    pop di
    pop si
    pop bx
    pop es

    mov sp, bp
    pop bp
    ret

global _x86_FarJump
; Performs a real-mode far jump to a caller-provided segment and offset.
_x86_FarJump:
    push bp
    mov bp, sp

    push word [bp + 4]
    push word [bp + 6]
    retf

global _x86_GetMemoryMap
; Collects BIOS E820 memory map entries into the caller-provided buffer.
_x86_GetMemoryMap:
    push bp
    mov bp, sp

    push es
    push bx
    push cx
    push dx
    push si
    push di

    mov di, [bp + 4]
    mov ax, [bp + 6]
    mov es, ax
    xor ebx, ebx
    xor si, si

.next_entry:
    cmp si, [bp + 8]
    jae .success

    mov eax, 0E820h
    mov edx, 0534D4150h
    mov ecx, 24
    mov dword [es:di + 20], 1
    int 15h

    jc .done_or_failed
    cmp eax, 0534D4150h
    jne .failed
    cmp ecx, 20
    jb .failed

    inc si
    add di, 24

    test ebx, ebx
    jnz .next_entry

.success:
    mov di, [bp + 10]
    mov [di], si
    mov ax, 1
    jmp .done

.done_or_failed:
    cmp si, 0
    jne .success

.failed:
    mov di, [bp + 10]
    mov word [di], 0
    xor ax, ax

.done:
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop es

    mov sp, bp
    pop bp
    ret

global _x86_FarJumpWithBootInfo
; Sets ES:BX to BootInfo and far-jumps to the loaded kernel.
_x86_FarJumpWithBootInfo:
    push bp
    mov bp, sp

    mov ax, [bp + 8]
    mov es, ax
    mov bx, [bp + 10]

    push word [bp + 4]
    push word [bp + 6]
    retf
