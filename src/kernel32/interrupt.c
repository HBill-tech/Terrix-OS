#include <interrupt.h>
#include <kernel.h>
#include <os.h>
#include <tty.h>
#include <pic.h>
#include <timer.h>
#include <rtc.h>

// 中断描述符表 IDT
gate_t int_table[INTERRUPT_GATE_SIZE];

/**
 * 默认中断处理函数
 * @param frame 传入的参数帧
 */
void handler_default(interrupt_frame_t frame) {
    tty_printf("default handler\n");
    while (TRUE)
    {
        HLT;
    }
}

/**
 * 除零中断
 * @param frame 传入的参数帧
 */
void handler_division(interrupt_frame_t frame) {
    tty_printf("division handler\n");
}

/**
 * 调试中断
 * @param frame 传入的参数帧
 */
void handler_debug(interrupt_frame_t frame) {
    tty_printf("debug handler\n");
}

/**
 * 不可屏蔽中断
 * @param frame 传入的参数帧
 */
void handler_nmi(interrupt_frame_t frame) {
    tty_printf("nmi handler\n");
}

/**
 * 断点异常
 * @param frame 传入的参数帧
 */
void handler_breakpoint(interrupt_frame_t frame) {
    tty_printf("breakpoint handler");
}

/**
 * 算数指令的溢出异常
 * @param frame 传入的参数帧
 */
void handler_overflow(interrupt_frame_t frame) {
    tty_printf("overflow handler");
}

/**
 * 数组越界异常
 * @param frame 传入的参数帧
 */
void handler_boundrange(interrupt_frame_t frame) {
    tty_printf("oound range handler");
}

/**
 * 执行非法指令异常
 * @param frame 传入的参数帧
 */
void handler_opcode(interrupt_frame_t frame) {
    tty_printf("opcode handler");
}

/**
 * 性能优化中断
 * @param frame 传入的参数帧
 */
void handler_device(interrupt_frame_t frame) {
    tty_printf("device handler");
}

/**
 * 双重故障异常
 * @param frame 传入的参数帧
 */
void handler_double(interrupt_frame_t frame) {
    tty_printf("double fault handler");
}

/**
 * 无效 TSS 异常
 * @param frame 传入的参数帧
 */
void handler_tss(interrupt_frame_t frame) {
    tty_printf("tss handler");
}

/**
 * 缺段异常
 * @param frame 传入的参数帧
 */
void handler_segment(interrupt_frame_t frame) {
    tty_printf("segment handler");
}

/**
 * 栈段异常
 * @param frame 传入的参数帧
 */
void handler_stack(interrupt_frame_t frame) {
    tty_printf("stack handler");
}

/**
 * 常规保护异常
 * @param frame 传入的参数帧
 */
void handler_protection(interrupt_frame_t frame) {
    tty_printf("protection handler");
}

/**
 * 缺页异常
 * @param frame 传入的参数帧
 */
void handler_page(interrupt_frame_t frame) {
    tty_printf("page handler");
}

/**
 * 浮点异常
 * @param frame 传入的参数帧
 */
void handler_fpu(interrupt_frame_t frame) {
    tty_printf("fpu handler");
}

/**
 * 对齐检查异常
 * @param frame 传入的参数帧
 */
void handler_align(interrupt_frame_t frame) {
    tty_printf("align handler");
}

/**
 * 机器检查异常
 * @param frame 传入的参数帧
 */
void handler_machine(interrupt_frame_t frame) {
    tty_printf("machine handler");
}

/**
 * SIMD 浮点异常
 * @param frame 传入的参数帧
 */
void handler_simd(interrupt_frame_t frame) {
    tty_printf("simd handler");
}

/**
 * 虚拟化异常
 * @param frame 传入的参数帧
 */
void handler_virtual(interrupt_frame_t frame) {
    tty_printf("virtual handler");
}

/**
 * 控制保护异常
 * @param frame 传入的参数帧
 */
void handler_control(interrupt_frame_t frame) {
    tty_printf("control handler");
}

/**
 * 设置中断门
 * @param vector    中断号
 * @param offset    中断处理函数的地址，由于系统是32位，所以这里地址也是32位
 * @param selector  代码段选择子
 * @param attr      中断门属性
 */
void set_interrupt_gate(int vector, uint32_t offset, uint32_t selector, uint16_t attr) {
    gate_t *entry = &int_table[vector];
    entry->offset_l = offset & 0xFFFF;
    entry->selector = selector;
    entry->attr = attr;
    entry->offset_h = (offset >> 16) & 0xFFFF;
}

void set_interrupt_handler(int vector, uint32_t handler) {
    uint16_t attr = GATE_ATTR_P | GATE_ATTR_DPL0 | GATE_TYPE_INT;
    set_interrupt_gate(vector, (uint32_t)handler, KERNEL_CODE_SEG, attr);
}

void interrupt_init() {
    uint16_t default_attr = GATE_ATTR_P | GATE_ATTR_DPL0 | GATE_TYPE_INT;
    // 初始化 int_table，表中的所有中断都设置为默认的 handler
    for (int i = 0; i < INTERRUPT_GATE_SIZE; i++)
    {
        set_interrupt_gate(i, (uint32_t)interrupt_handler_default, KERNEL_CODE_SEG, default_attr);
    }

    /************************************ 配置内中断 *******************************************/
    set_interrupt_handler(IRQ0_DE, (uint32_t)interrupt_handler_division);
    set_interrupt_handler(IRQ1_DB, (uint32_t)interrupt_handler_debug);
    set_interrupt_handler(IRQ2_NMI, (uint32_t)interrupt_handler_nmi);
    set_interrupt_handler(IRQ3_BP, (uint32_t)interrupt_handler_breakpoint);
    set_interrupt_handler(IRQ4_OF, (uint32_t)interrupt_handler_overflow);
    set_interrupt_handler(IRQ5_BR, (uint32_t)interrupt_handler_breakpoint);
    set_interrupt_handler(IRQ6_UD, (uint32_t)interrupt_handler_opcode);
    set_interrupt_handler(IRQ7_NM, (uint32_t)interrupt_handler_device);
    set_interrupt_handler(IRQ8_DF, (uint32_t)interrupt_handler_double);
    set_interrupt_handler(IRQA_TS, (uint32_t)interrupt_handler_tss);
    set_interrupt_handler(IRQB_NP, (uint32_t)interrupt_handler_segment);
    set_interrupt_handler(IRQC_SS, (uint32_t)interrupt_handler_stack);
    set_interrupt_handler(IRQD_GP, (uint32_t)interrupt_handler_protection);
    set_interrupt_handler(IRQE_PF, (uint32_t)interrupt_handler_page);
    set_interrupt_handler(IRQ10_MF, (uint32_t)interrupt_handler_fpu);
    set_interrupt_handler(IRQ11_AC, (uint32_t)interrupt_handler_align);
    set_interrupt_handler(IRQ12_MC, (uint32_t)interrupt_handler_machine);
    set_interrupt_handler(IRQ13_XM, (uint32_t)interrupt_handler_simd);
    set_interrupt_handler(IRQ14_VE, (uint32_t)interrupt_handler_virtual);
    set_interrupt_handler(IRQ15_CP, (uint32_t)interrupt_handler_control);

    /************************************** 配置外中断 ************************************/
    pic_init();     // 初始化外中断芯片 8259A
    timer_init();   // 初始化 8253 芯片（定时器中断）
    rtc_init();     // 初始化 CMOS (实时时钟中断)

    lidt((uint32_t)int_table, (uint16_t)sizeof(int_table));     // 把 int_table 正式加载到 IDT 中

    /**
     * 如果这里不开中断，即使 8259A 解除对某个中断的屏蔽还是不行
     * 8259A 的屏蔽中断是是否接受某个外设的中断请求
     * 而系统对中断的屏蔽会导致 82599A 即使接收到了中断请求，也不能触发中断向量
     */
    // sti();          // 开中断，允许触发中断向量（包括内外中断）
}