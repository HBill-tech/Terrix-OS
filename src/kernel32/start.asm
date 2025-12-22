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
%macro INTERRUPT_HANDLER 3
global interrupt_handler_%1
extern handler_%1

interrupt_handler_%1:
    ; %if %3 == 0
    ;     push dword 0    ; 如果第三个参数等于0，压入一个 0 作为错误码占位符
    ; %endif
    ; push dword %2       ; 压入中断向量号
    pusha
    push ds
    push es
    push fs
    push gs

    push esp
    call handler_%1
    add esp, 4          ; 跳过 push esp 压入栈的数据，指向 gs

    pop gs
    pop fs
    pop es
    pop ds
    popa
    ; add esp, 8        ; 跳过 错误码 和 中断向量，使得 esp 指向 eip
    iret
%endmacro

; 使用宏生成各个中断处理的函数
INTERRUPT_HANDLER default,          -1,     0
INTERRUPT_HANDLER division,         0,      0
INTERRUPT_HANDLER debug,            1,      0
INTERRUPT_HANDLER nmi,              2,      0
INTERRUPT_HANDLER breakpoint,       3,      0
INTERRUPT_HANDLER overflow,         4,      0
INTERRUPT_HANDLER boundrange,       5,      0
INTERRUPT_HANDLER opcode,           6,      0
INTERRUPT_HANDLER device,           7,      0
INTERRUPT_HANDLER double,           8,      1
INTERRUPT_HANDLER tss,              10,     1
INTERRUPT_HANDLER segment,          11,     1
INTERRUPT_HANDLER stack,            12,     1
INTERRUPT_HANDLER protection,       13,     1
INTERRUPT_HANDLER page,             14,     1
INTERRUPT_HANDLER fpu,              16,     0
INTERRUPT_HANDLER align,            17,     1
INTERRUPT_HANDLER machine,          18,     0
INTERRUPT_HANDLER simd,             19,     0
INTERRUPT_HANDLER virtual,          20,     0
INTERRUPT_HANDLER control,          21,     1