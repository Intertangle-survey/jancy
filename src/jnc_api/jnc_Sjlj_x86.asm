section .text

    JB_BP  equ 0
    JB_BX  equ 1
    JB_DI  equ 2
    JB_SI  equ 3
    JB_SP  equ 4
    JB_IP  equ 5
    PCOFF  equ 0
    JMPBUF equ 4

;
; int
; _jnc_setJmp(jmp_buf env);
;
; Parameters:
;   [ESP+04h] - jmp_buf env
; Registers:
;   None
; Returns:
;   0
; Notes:
;   Sets up the jmp_buf
;

global _jnc_setJmp
_jnc_setJmp:
    xor eax, eax
    mov edx, [esp + JMPBUF]

    ; Save registers.
    mov [edx + JB_BP*4], ebp   ; Save caller's frame pointer.
    mov [edx + JB_BX*4], ebx
    mov [edx + JB_DI*4], edi
    mov [edx + JB_SI*4], esi
    lea ecx, [esp + JMPBUF]    ; Save SP as it will be after we return.
    mov [edx + JB_SP*4], ecx
    mov ecx, [esp + PCOFF]     ; Save PC we are returning to now.
    mov [edx + JB_IP*4], ecx
    ret

;
; void
; _jnc_longJmp(jmp_buf env, int value);
;
; Parameters:
;   [ESP+04h] - jmp_buf setup by _setjmp
;   [ESP+08h] - int     value to return
; Registers:
;   None
; Returns:
;   Doesn't return
; Notes:
;   Non-local goto
;

global _jnc_longJmp
_jnc_longJmp:
    mov ecx, [esp + JMPBUF]   ; User's jmp_buf in %ecx.

    mov eax, [esp + 8]        ; Second argument is return value.
    ; Save the return address now.
    mov edx, [ecx + JB_IP*4]
    ; Restore registers.
    mov ebp, [ecx + JB_BP*4]
    mov ebx, [ecx + JB_BX*4]
    mov edi, [ecx + JB_DI*4]
    mov esi, [ecx + JB_SI*4]
    mov esp, [ecx + JB_SP*4]
    ; Jump to saved PC.
    jmp edx
