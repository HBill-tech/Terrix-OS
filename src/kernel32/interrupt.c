#include <interrupt.h>
#include <kernel.h>
#include <os.h>

// 中断描述符表 IDT
gate_t int_table[INTERRUPT_GATE_SIZE];

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

void interrupt_init() {
    uint16_t default_attr = GATE_ATTR_P | GATE_ATTR_DPL0 | GATE_TYPE_INT;
    for (int i = 0; i < INTERRUPT_GATE_SIZE; i++)
    {
        set_interrupt_gate(i, (uint32_t)interrupt_handler_default, KERNEL_CODE_SEG, default_attr);
    }
    
    lidt((uint32_t)int_table, INTERRUPT_GATE_SIZE);
}