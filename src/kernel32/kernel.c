#include <kernel.h>
#include <tty.h>
#include <cstd/stdio.h>
#include <interrupt.h>
#include <logf.h>
#include <cstd/time.h>
#include <task.h>

// 不同任务的 tss 段
static tss_task_t main_task, child_task;
// 如果是 1024 会导致子任务栈炸了，无法正常切换任务
static uint8_t child_task_stack[4096];

void main_task_entry() {
    int counter = 0;
    while (TRUE) {
        tty_logf("Main task running... Counter: %d", counter++);
        tss_task_switch(&main_task, &child_task); // 切换到子任务
    }
}

void child_task_entry() {
    int counter = 0;
    while (TRUE) {
        tty_logf("Child task running... Counter: %d", counter++);
        tss_task_switch(&child_task, &main_task); // 切换回主任务
    }
}

void hlos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();
    interrupt_init();       // 中断初始化

    // 测试串口中断
    tty_logf_init();
    tty_logf("KERNEL VERSION: %s, OS VERSION: %s", KERNEL_VERSION, OP_SYS_VERSION);
    tty_logf("Move to gitcode...");
    time_init(OS_TZ);

    // 发现这里的 gdt_info 与 bootloader 传入的不一样，是因为在跳转到 32 位内核期间被移动了
    gdt32_init((gdt_table_t*)gdt_info);       // GDT 重载

    /**
     * 为什么主任务的 entry 和 stack 都是 0 ?
     * 
     * 当调用 write_tr(main_task.selector) 后，TR 寄存器指向主任务的 TSS.
     * 
     * 此时 TSS 内容（如 eip、esp）尚未使用.
     * 
     * 后续执行 tss_task_switch(&main_task, &child_task) 切换到子任务时，
     *      CPU 会自动将当前所有寄存器（包括 EIP、ESP）保存到 TR 所指向的 TSS（即主任务的 TSS）中，
     *      覆盖掉原有的 0 值.
     * 
     * 因此，主任务 TSS 的初始 eip=0 和 esp=0 是无意义的，它们会在切换时被正确的值取代.
     */
    tss_task_init(&child_task, (uint32_t)child_task_entry, &child_task_stack[4096]);
    tss_task_init(&main_task, 0, 0);

    // tr 用来存储当前执行任务的 tss 段选择子
    // 初始化 tr 为主任务 tss 段的选择子, 
    // 后续任务切换时候直接保存主任务上下文到 tr 在 GDT 中指向的 tss 段
    write_tr(main_task.selector);   
    main_task_entry();              // 进入主任务
}