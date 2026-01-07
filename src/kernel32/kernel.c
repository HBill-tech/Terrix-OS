#include <kernel.h>
#include <tty.h>
#include <stdio.h>
#include <interrupt.h>
#include <logf.h>

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

    /**
     * 如果这里不开中断，即使 8259A 解除对某个中断的屏蔽还是不行
     * 8259A 的屏蔽中断是是否接受某个外设的中断请求
     * 而系统对中断的屏蔽会导致 82599A 即使接收到了中断请求，也不能触发中断向量
     */
    sti();          // 开中断，允许触发中断向量（包括内外中断）
    
    while (TRUE);
}