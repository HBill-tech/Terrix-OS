#include <interrupt.h>
#include <kernel.h>
#include <os.h>
#include <tty.h>

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
    for (int i = 0; i < INTERRUPT_GATE_SIZE; i++)
    {
        set_interrupt_gate(i, (uint32_t)interrupt_handler_default, KERNEL_CODE_SEG, default_attr);
    }

    set_interrupt_handler(IRQ0_DE, (uint32_t)interrupt_handler_division);
    set_interrupt_handler(IRQ1_DB, (uint32_t)interrupt_handler_debug);
    
    lidt((uint32_t)int_table, INTERRUPT_GATE_SIZE);
}