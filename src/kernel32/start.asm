section .text
extern hlos_init

global _start
_start:
    ; 最简单直接的字符显示 - 在屏幕左上角显示 'K'
    ; ; 不使用栈帧，直接使用栈中的参数
    ; mov edi, 0xB8000  ; VGA文本缓冲区起始地址
    ; mov ah, 0x0F      ; 属性字节：黑底白字
    ; mov al, 'K'       ; 字符'K'
    ; mov [edi], ax     ; 将字符和属性写入显存

    push ebp
    mov ebp, esp
    mov eax, [esp + 4]      ; 第一个参数
    mov ebx, [esp + 8]      ; 第二个参数
    push ebx
    push eax
    call hlos_init
    jmp $

section .text
global interrupt_handler_default
interrupt_handler_default:
    nop
    iret