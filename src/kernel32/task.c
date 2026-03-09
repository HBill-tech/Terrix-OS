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
    /**
     * 这里有一些寄存器值没有初始化，都默认为 0.
     * 
     *     这些寄存器值不会影响将被调度的任务（其他任务）执行. 当将被调度的任务第一次运行时，
     * 纵使从tss段加载的部分寄存器值缺失，该任务会在prolog过程建立这些寄存器的值，使得任务在切换之前能够正常进行.
     * 在切换到其他任务的时候，那些未初始化的寄存器值被填充到 tss 段使得 tss 段有意义.
     * 
     *     反之，对于主任务，寄存器中的值早就有了，也更不需要用tss段的值来初始化寄存器.
     * 因此纵使tss段所有值缺失，也根本不会影响主任务运行. 更不用谈论那些没有被初始化的 tss 字段值了.
     * 因此主任务在切换之前一定能正常运行. 主任务会在第一次切换其他任务的时候更新tss段中的这些字段值使得tss段有意义.
     *     
     *     主任务以及其他任务一旦建立了完整的有意义的 tss 段，后续反复调度的任务都将正常进行。
     * 
     *     总之，纵使一些寄存器没有初始化，也不影响任务的正常执行.
     */
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
 *      ebp
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
         *     初始化被调用者保存寄存器（callee-saved）的值。
         * 在 x86 调用约定中，EBP、EBX、ESI、EDI 属于被调用者保存寄存器，
         * 这意味着函数若使用它们必须先保存原值并在返回前恢复。
         * 
         *     此处这些寄存器的值会在该任务第一次被调度后被prolog覆盖。
         * 因此我们可以任意设置它们的初始值，只要保证它们是确定的即可。
         *
         *     当任务第一次被调度时，soft_switch 会从栈中弹出这些值到对应寄存器，
         * 然后通过 ret 跳转到入口函数。入口函数的序言（prolog）会立即保存这些
         * 寄存器（例如 push ebp; push ebx; ...），并建立自己的栈帧。
         * 此后，这些初始值仅作为“上一级”的上下文被保存在栈上，不再影响任务执行。
         * 在后续的任务切换中，soft_switch 会保存和恢复任务当前的寄存器值，
         * 因此这些初始值只用于第一次启动，之后任务完全自主维护自己的寄存器状态。
         *     总之，将 EBX/ESI/EDI 设为 1/2/3、EBP 设为 0 是安全且合理的。
         * 
         * 重要说明：
         *     这些初始化值只会在任务第一次被调度的那一刻加载到寄存器，
         * 建立 prolog 之后就被“尘封”在栈中了。对于后续的每一次调度
         * （即任务被切换回来时），寄存器值都是从该任务上次被切换出去时
         * 保存的上下文中恢复的，与初始值无关。
         *     同样，如果该任务入口函数被其他代码作为普通函数直接调用
         * （而非通过调度器），调用者会按照调用约定提供寄存器值，
         * 此处的这些初始值也不会对寄存器值产生任何影响。
         *     因此，这些初始值仅在“该任务第一次从调度器获得 CPU”这一特定场景下短暂存在于寄存器，
         * 之后便被任务的正常执行流完全取代，既安全又便于调试。
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
