#ifndef HLOS_PIC_H
#define HLOS_PIC_H


#define PIC0_CMD    0x20    // PIC0 命令端口
#define PIC0_DATA   0x21    // PIC0 数据端口

#define PIC1_CMD    0xA0    // PIC1 命令端口
#define PIC1_DATA   0xA1    // PIC1 数据端口

// 初始化命令字（ICW）的位定义
#define PIC_ICW1_ALWAYS_1   (1 << 4)    // 设置 ICW1 时第4位必须为1，表示这是ICW1
#define PIC_ICW1_ICW4       (1 << 0)    // 设置 ICW1 时第0位=1表示需要ICW4
#define PIC_ICW4_8086       (1 << 0)    // 设置 ICW4 时第0位=1表示8086/88模式

// 操作命令字（OCW）的位定义
#define PIC_OCW2_EOI        (1 << 5)    // EOI标识

#define IRQ_PIC0_START      0x20        // PIC0 中断向量号偏移（将 IRQ0 - 7 映射到 0x20 - 0x27）
#define IRQ_PIC1_START      0x28        // PIC1 中断向量号偏移 (将 IRQ8 ~ 15 映射到 0x28 ~ 0x2F)
#define IRQ_PIC_END             0x2F        // PIC 对应的最后一个系统中断向量号

void pic_init();

void irq_enable(int irq_number);

void iqr_disable(int irq_number);

void send_eoi(int irq_number);

#endif