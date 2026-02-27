// 为啥这里不 __code16_gcc__ ?
#include <task.h>
#include <cstd/string.h>

static void tss_init(tss_task_t *task, uint32_t entry, uint32_t esp)
{
    // 把 TSS 结构体清零
    kernel_memset(&task->tss, 0, sizeof(tss_t));
    // 设置任务的入口地址，eip表示该任务的下一条指令地址
    task->tss.eip = entry;
    // 设置任务的栈顶地址，esp表示该任务的栈顶地址，esp0是内核栈顶地址
    task->tss.esp = task->tss.esp0 = esp;
    // 把 es, ds, fs, gs, ss 和 ss0 都设置为内核数据段选择子
    task->tss.es = task->tss.ds =  task->tss.fs =  task->tss.gs = task->tss.ss = task->tss.ss0 = KERNEL_DATA_SEG;
    // 把 cs 设置为内核代码段选择子
    task->tss.cs = KERNEL_CODE_SEG;
    // 设置 EFLAGS 寄存器，启用中断(IF位)
    // task->tss.eflags = EFLAGS_DEFAULT | EFLAGS_IF;
}

/**
 * 初始化一个 TSS 任务
 * @param task  任务结构体指针
 * @param entry 任务入口地址
 * @param esp   任务栈顶地址
 */
void tss_task_init(tss_task_t *task, uint32_t entry, uint32_t esp)
{
    tss_init(task, entry, esp);
}

void tss_task_switch(tss_task_t *from, tss_task_t *to)
{
}