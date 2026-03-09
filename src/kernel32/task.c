// 为啥这里不 __code16_gcc__ ?
#include <task.h>
#include <cstd/string.h>

/**
 * tss 初始化
 *      1.分配一个 selector 给 tss 段.
 *      2.把 tss 段地址和 selector 关联.
 *      3.初始化 tss 段内容.
 * @param task  tss_task_t 段结构体的首地址
 * @param entry 该任务的入口
 * @param esp   该任务的栈顶
 */
static void tss_init(tss_task_t *task, uint32_t entry, uint32_t esp)
{
    uint32_t selector = alloc_gdt_table_entry(); // 从 GDT 中分配一个描述符
    if (selector < 0)
    {
        return; // 没有可用的描述符，初始化失败
    }
    
    // 指向 tss 段结构体的指针
    tss_t *tss = &(task->tss);

    // 设置 GDT 描述符，属性为 0x89（存在、特权级 0、类型为 9 的 TSS 描述符）
    // 将 GDT 中新增的段与当前任务的 tss 结构体绑定，构成一个 TSS 段
    set_gdt_table_entry(selector, (uint32_t)tss, sizeof(tss_t) - 1, 
        SEG_ATTR_P | SEG_ATTR_DPL0 | SEG_TYPE_TSS); 

    // 把 TSS 结构体清零，之后设置 TSS 的相关字段
    kernel_memset(tss, 0, sizeof(tss_t));
    // 设置任务的入口地址，eip表示该任务的下一条指令地址
    tss->eip = entry;
    // 设置任务的栈顶地址，esp表示该任务的栈顶地址，esp0是内核栈顶地址
    tss->esp = tss->esp0 = esp;
    // 把 es, ds, fs, gs, ss 和 ss0 都设置为内核数据段选择子
    tss->es = tss->ds = tss->fs = tss->gs = tss->ss = tss->ss0 = KERNEL_DATA_SEG;
    // 把 cs 设置为内核代码段选择子
    tss->cs = KERNEL_CODE_SEG;
    // 设置 EFLAGS 寄存器，启用中断(IF位)
    tss->eflags = EFLAGS_DEFAULT | EFLAGS_IF;

    // 初始化任务 TSS 段对应的 GDT 选择子
    task->selector = selector;
}

/**
 * 初始化一个任务
 * @param task  任务结构体指针
 * @param entry 任务入口地址
 * @param esp   任务栈顶地址
 */
void tss_task_init(tss_task_t *task, uint32_t entry, uint32_t esp)
{
    tss_init(task, entry, esp);
}

/**
 * 切换任务
 * @param from 当前任务
 * @param to   目标任务
 */
void tss_task_switch(tss_task_t *from, tss_task_t *to)
{
    /**
     * 当CPU执行一条远跳转（JMP）或远调用（CALL）指令，
     * 且目标选择子指向 GDT 中的一个可用 TSS 描述符（类型为0x9或0xB）时，
     * 硬件会自动完成以下步骤：
     *      1. 保存当前任务状态：CPU将当前所有通用寄存器、段寄存器、
     *          EFLAGS、EIP等现场信息自动保存到当前TR寄存器所指向的TSS中.
     *      2. 加载新任务状态：从目标 TSS 中恢复所有寄存器的值，包括 EIP、ESP 等，
     *          从而跳转到新任务的执行流.
     *      3. 更新TR：将TR寄存器指向新任务的TSS，并标记新TSS为“忙”.
     * 整个过程无需软件干预，CPU硬件自动完成。
     */
    far_jump(to->selector, 0); // 远跳转到目标任务的入口地址，触发任务切换
}


/**
 * 初始化 soft task.
 * i386压栈顺序
 *      param2
 *      param1
 *      eip
 *      ebp: 调用者的 ebp，和被调用函数的任务无关，但被调用者有义务在结束任务时恢复 ebp 值.
 *              被调用者会在 push ebp 后建立自己的 ebp.
 *      ebx
 *      esi
 *      edi
 * @param task  任务结构体指针
 * @param entry 任务入口函数
 * @param esp   任务栈顶
 */
void soft_task_init(soft_task_t* task, uint32_t entry, uint32_t esp) {
    uint32_t *pesp = (uint32_t*)esp;
    if (pesp)
    {
        *(--pesp) = entry;      // eip
        /**
         * 以下是“被调用者保存”寄存器，被调用者有义务维护这些寄存器的值最终不变.
         * 但是这些值的初始值是什么不重要，那是调用者的事情.
         * 
         * 如果一个函数是通过调度器切换进入的，那么它不扮演“被调用者”的角色.
         * 因此初始化一些“不被调用”的任务时，这些值赋值为确定值就好.
         * 
         * 此处设立了寄存器值只约束通过调度器进入该任务的情况. 不约束通过函数调用该任务的情况.
         */
        *(--pesp) = 0;          // ebp
        *(--pesp) = 1;          // ebx
        *(--pesp) = 2;          // esi
        *(--pesp) = 3;          // edi
        task->stack = pesp;     // 把当前的栈指针当作存储任务上下文的栈顶
    }
}

/**
 * 任务切换
 * @param from  from->stack的指针，指向栈地址的指针
 * @param to    to->stack，栈地址
 */
extern void soft_switch(uint32_t **from, uint32_t *to);

/**
 * 软切换的函数
 * @param from  当前任务
 * @param to    目标任务
 */
void soft_task_switch(soft_task_t* from, soft_task_t* to) {
    soft_switch(&from->stack, to->stack);
}
