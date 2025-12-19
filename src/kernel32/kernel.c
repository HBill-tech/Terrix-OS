#include <kernel.h>
#include <tty.h>
#include <stdio.h>
#include <interrupt.h>

void test(int a, ...) {
    va_list args;
    va_start(args, a);
    // int p = *(int*)args;

    int n = va_arg(args, int);
    int k = va_arg(args, int);
    // int c = *(int*)args;
    va_end(args);
}


void hlos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    interrupt_init();
    
    int a = 3 / 0;      // 测试中断

    tty_init();
    tty_printf("hello, world!\n");
    tty_printf("I LOVE U GSY! %d", 1314520);
    while (TRUE);
}