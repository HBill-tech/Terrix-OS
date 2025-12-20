#ifndef HLOS_INTERRUPT_H
#define HLOS_INTERRUPT_H

#include <types.h>


/**
 * 调用中断处理函数时需要传递的参数帧
 */
typedef struct interrupt_frame_t
{
    // 手动压入
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    // 自动压入
    uint32_t eip, cs, eflags;
}interrupt_frame_t;


// 默认的中断处理函数
extern void interrupt_handler_default();
void handler_default();

// 除 0 异常
#define IRQ0_DE             0
extern void interrupt_handler_division();
void handler_division(interrupt_frame_t frame);

// 调试 Debug
#define IRQ1_DB             1
extern void interrupt_handler_debug();
void handler_debug(interrupt_frame_t frame);

#define INTERRUPT_GATE_SIZE     0x100           // 中断描述符表的长度



/**
 * 0101    任务门
 * 0110    16位中断门
 * 0111    16位陷阱门
 * 1110    32位中断门  ← 这就是 0xE
 * 1111    32位陷阱门
 */
#define GATE_TYPE_INT           (0xE << 8)      // 中断门类型
#define GATE_ATTR_P             (1 << 15)       // 存在位
#define GATE_ATTR_DPL0          (0 << 13)       // 描述符特权级0 (内核级)
#define GATE_ATTR_DPL3          (3 << 13)       // 描述符特权级3 (用户级)

/**
 * 中断描述符表记录的结构，每一条记录叫做一个 “中断门”
 */
typedef struct gate_t
{
    uint16_t offset_l;      // 处理函数地址低 16 位
    uint16_t selector;      // 代码段选择子
    uint16_t attr;          // 属性（类型，DPL，P位等）
    uint16_t offset_h;      // 处理函数地址高 16 位
} gate_t;


void set_interrupt_gate(int vector, uint32_t offset, uint32_t selector, uint16_t attr);

void interrupt_init();

#endif