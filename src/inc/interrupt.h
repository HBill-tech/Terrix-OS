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
void handler_default(interrupt_frame_t frame);

// 除 0 异常
#define IRQ0_DE             0
extern void interrupt_handler_division();
void handler_division(interrupt_frame_t frame);

// 调试 Debug
#define IRQ1_DB             1
extern void interrupt_handler_debug();
void handler_debug(interrupt_frame_t frame);

// 不可屏幕中断
#define IRQ2_NMI            2
extern void interrupt_handler_nmi();
void handler_nmi(interrupt_frame_t frame);

// 断点异常
#define IRQ3_BP             3
extern void interrupt_handler_breakpoint();
void handler_breakpoint(interrupt_frame_t frame);

// 算数指令的溢出异常
#define IRQ4_OF             4
extern void interrupt_handler_overflow();
void handler_overflow(interrupt_frame_t frame);

// 数组越界异常
#define IRQ5_BR             5
extern void interrupt_handler_boundrange();
void handler_boundrange(interrupt_frame_t frame);

// 执行非法指令异常
#define IRQ6_UD             6
extern void interrupt_handler_opcode();
void handler_opcode(interrupt_frame_t frame);

// 性能优化中断
#define IRQ7_NM             7
extern void interrupt_handler_device();
void handler_device(interrupt_frame_t frame);

//双重故障异常
#define IRQ8_DF             8
extern void interrupt_handler_double();
void handler_double(interrupt_frame_t frame);

// 协处理器段溢出       已废弃
#define IRQ9_CSO            9

// 无效 TSS 异常
#define IRQA_TS             10
extern void interrupt_handler_tss();
void handler_tss(interrupt_frame_t frame);

// 缺段异常
#define IRQB_NP             11
extern void interrupt_handler_segment();
void handler_segment(interrupt_frame_t frame);

// 栈段异常
#define IRQC_SS             12
extern void interrupt_handler_stack();
void handler_stack(interrupt_frame_t frame);

// 常规保护异常             
#define IRQD_GP             13
extern void interrupt_handler_protection();
void handler_protection(interrupt_frame_t frame);

// 缺页异常
#define IRQE_PF             14
extern void interrupt_handler_page();
void handler_page(interrupt_frame_t frame);

// 保留，未使用
#define IRQF_RTN            15

// 浮点异常
#define IRQ10_MF            16
extern void interrupt_handler_fpu();
void handler_fpu(interrupt_frame_t frame);

// 对齐检查异常
#define IRQ11_AC            17
extern void interrupt_handler_align();
void handler_align(interrupt_frame_t frame);

// 机器检查异常
#define IRQ12_MC            18
extern void interrupt_handler_machine();
void handler_machine(interrupt_frame_t frame);

// SIMD 浮点异常
#define IRQ13_XM            19
extern void interrupt_handler_simd();
void handler_simd(interrupt_frame_t frame);

// 虚拟化异常
#define IRQ14_VE            20
extern void interrupt_handler_virtual();
void handler_virtual(interrupt_frame_t frame);

// 控制保护异常
#define IRQ15_CP            21
extern void interrupt_handler_control();
void handler_control(interrupt_frame_t frame);

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