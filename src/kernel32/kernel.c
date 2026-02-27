#include <kernel.h>
#include <tty.h>
#include <cstd/stdio.h>
#include <interrupt.h>
#include <logf.h>
#include <cstd/time.h>
#include <task.h>

static tss_task_t main_task, child_task;
static uint8_t child_task_stack[1024];

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

    tss_task_init(&child_task, (uint32_t)child_task_entry, &child_task_stack[1024]);
    tss_task_init(&main_task, 0, 0);

    write_tr(main_task.selector); // 加载主任务的 TSS 选择子
    
    main_task_entry(); // 进入主任务
}