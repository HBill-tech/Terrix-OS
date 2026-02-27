#ifndef HLOS_TASK_H
#define HLOS_TASK_H

#include <kernel.h>

/**
 * tss_t 结构体定义了任务状态段（TSS）的格式，用于保存任务的寄存器上下文信息。
 */
typedef struct tss_t{
    uint32_t pre_link;
    // esp0 和 ss0 是内核栈的栈顶地址和段选择子，分别用于特权级 0 的任务切换
    // esp1 和 ss1 是特权级 1 的栈顶地址和段选择子
    // esp2 和 ss2 是特权级 2 的栈顶地址和段选择子
    uint32_t esp0, ss0, esp1, ss1, esp2, ss2;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t iomap;
} tss_t;

/**
 * tss_task_t 结构体封装了一个任务
 */
typedef struct tss_task_t {
    tss_t tss;          // 该任务 tss 段，其首地址将在后续作为该 tss 段在 GDT 中的 base
    uint16_t selector;  // 该任务 tss 段对应于 GDT 中的选择子
} tss_task_t;

void tss_task_init(tss_task_t* task, uint32_t entry, uint32_t esp);

void tss_task_switch(tss_task_t *from, tss_task_t *to);

#endif