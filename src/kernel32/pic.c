#include <kernel.h>
#include <pic.h>

/**
 * pic 初始化函数
 * 初始化一块板子要依次设置 ICW1 ~ ICW4
 */
void pic_init() {
    uint8_t ICW1, ICW2, ICW3, ICW4, IMR;

    ICW1 = PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4;
    ICW2 = IRQ_PIC0_START;
    ICW3 = 1 << 2;
    ICW4 = PIC_ICW4_8086;

    // 初始化 PIC0
    outb(PIC0_CMD, ICW1);  // 设置 ICW1
    outb(PIC0_DATA, ICW2);                    // 设置 ICW2 : 设置中断向量号偏移   从此调用者只需要触发系统中断向量号即可触发PIC中断
    outb(PIC0_DATA, ICW3);                            // 设置 ICW3 : 主片的IRQ2连接了从片（所以位2=1）
    outb(PIC0_DATA, ICW4);                     // 设置 ICW4 : 8086/88 模式


    ICW2 = IRQ_PIC1_START;
    // 初始化 PIC1
    outb(PIC1_CMD, ICW1);
    outb(PIC1_DATA, ICW2);
    outb(PIC1_DATA, ICW3);                            // 连接到主片的引脚
    outb(PIC1_DATA, ICW4);

    // 屏蔽中断
    IMR = 0xFF & ~(1 << 2);
    outb(PIC0_DATA, IMR);                   // 必须允许IRQ2，否则从片的中断无法通过
    IMR = 0xFF;
    outb(PIC1_DATA, IMR);                   // 屏蔽从片的全部中断
}

/**
 * 解除对某中断向量的屏蔽
 * @param irq_number    要解除屏蔽的中断向量号
 */
void irq_enable(int irq_number) {
    // 非法中断向量号
    if (irq_number < IRQ_PIC0_START || irq_number > IRQ_PIC_END)
    {
        return;
    }
    
    uint8_t IMR;
    irq_number -= IRQ_PIC0_START;

    // 解除对当前中断号的屏蔽
    if (irq_number < 8)
    {   
        /**
         * 从8259A的数据端口（0x21或0xA1）读取到的值
         * 就是当前中断屏蔽寄存器（IMR - Interrupt Mask Register）的掩码值
         */
        IMR = inb(PIC0_DATA) & ~(1 << irq_number);
        outb(PIC0_DATA, IMR);
    } else {
        irq_number -= 8;
        IMR = inb(PIC1_DATA) & ~(1 << irq_number);
        outb(PIC1_DATA, IMR);
    }
}

/**
 * 屏蔽某中断向量
 * @param irq_number    要屏蔽的中断向量
 */
void iqr_disable(int irq_number) {
    // 非法中断向量号
    if (irq_number < IRQ_PIC0_START || irq_number > IRQ_PIC_END)
    {
        return;
    }
    
    uint8_t IMR;
    irq_number -= IRQ_PIC0_START;
    // 屏蔽当前中断号
    if (irq_number < 8)
    {
        IMR = inb(PIC0_DATA) | (1 << irq_number);
        outb(PIC0_DATA, IMR);
    } else {
        irq_number -= 8;
        IMR = inb(PIC1_DATA) | (1 << irq_number);
        outb(PIC0_DATA, IMR);
    }
}

/**
 * 这是一个发送EOI（End Of Interrupt，中断结束）命令的函数。
 * 它在中断处理程序结束时调用，告诉8259A中断控制器"这个中断我已经处理完了"。
 * @param irq_number    即将执行完的中断向量号
 */
void send_eoi(int irq_number) {
    uint8_t OCW2 = PIC_OCW2_EOI;
    int irq_local = irq_number - IRQ_PIC0_START;    // 将中断向量号转化为局部的 IRQ 编号

    
    // 如果 IRQ 编号 >= 8，说明是从片的中断，要向从片发送 EOI
    if (irq_local >= 8)
    {
        outb(PIC1_CMD, OCW2);       // 向从片命令端口写入 OCW2 从而发送 EOI
    }

    // 不论是主片还是从片的中断，都要向主片发送 EOI
    outb(PIC0_CMD, OCW2);            // 向主片命令端口写入 OCW2 从而发送 EOI
}