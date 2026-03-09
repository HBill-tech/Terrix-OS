#include <kernel.h>
#include <tty.h>
#include <cstd/stdio.h>
#include <interrupt.h>
#include <logf.h>
#include <cstd/time.h>
#include <task.h>

// 定义 tss 任务，给任务分配一个 selector 字段空间以及 tss 段空间
//      在 tss_init 中会分配一个可用的 selector 值，
//      并在 GDT 中创建一个选择子为 selector 的 tss 段描述符，
//      使得 selector 与 tss 段进行关联.
// static tss_task_t main_task, child_task;
static soft_task_t main_task, child_task;
// 如果是 1024 会导致子任务栈炸了，无法正常切换任务
static uint8_t child_task_stack[4096];

void main_task_entry() {
    int counter = 0;
    while (TRUE) {
        tty_logf("Main task running... Counter: %d", counter++);
        // tss_task_switch(&main_task, &child_task); // 切换到子任务
        soft_task_switch(&main_task, &child_task);
    }
}

void child_task_entry() {
    int counter = 0;
    while (TRUE) {
        tty_logf("Child task running... Counter: %d", counter++);
        // tss_task_switch(&child_task, &main_task); // 切换回主任务
        soft_task_switch(&child_task, &main_task);
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

    gdt32_init((gdt_table_t*)gdt_info);       // GDT 重载

    // tss_task_init(&child_task, (uint32_t)child_task_entry, &child_task_stack[4096]);
    // /**
    //  * 为什么主任务的 entry 和 stack 都是 0 ?
    //  *
    //  * 主任务 TSS 的初始 eip=0 和 esp=0 是无意义的，它们会在切换到子任务时被正确的值取代.
    //  */
    // tss_task_init(&main_task, 0, 0);
    // // tr 用来存储当前执行任务的 tss 段选择子，
    // // 在切换任务时将上下文保存到 tr 指向的 tss 段,
    // // 并将新任务的 tss 段选择子加载道 tr,再从 tr 指向的 tss 段加载上下文.
    // write_tr(main_task.selector);   // 初始化 tr 为主任务 tss 段的选择子

    soft_task_init(&child_task, (uint32_t)child_task_entry, (uint32_t)&child_task_stack[4096]);
    // 这里为什么entry 和 stack 都是 0 ?
    // 这里设置二者无意义，会在第一次call soft_switch 时把 entry 自动写入 eip 中
    // 并在 soft_switch 中把主进程的栈地址赋值给 stack, 
    soft_task_init(&main_task, 0, 0);
    
    main_task_entry();              // 进入主任务
}