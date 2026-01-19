#include <kernel.h>
#include <tty.h>
#include <cstd/stdio.h>
#include <interrupt.h>
#include <logf.h>
#include <cstd/time.h>

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
    tty_init();
    interrupt_init();       // 中断初始化

    // 测试串口中断
    tty_logf_init();
    tty_logf("KERNEL VERSION: %s, OS VERSION: %s", KERNEL_VERSION, OP_SYS_VERSION);

    time_init(OS_TZ);
    
    while (TRUE);
}