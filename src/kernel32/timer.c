#include <timer.h>
#include <pic.h>
#include <kernel.h>

static uint32_t ticks = 0;

void handler_timer(interrupt_frame_t frame) {
    ticks++;                // 时钟打点次数加一
    send_eoi(IRQ0_TIMER);   // 表示中断处理完毕
}

void timer_init() {

    // 时钟中断间隔内的脉冲数
    uint32_t interval_pulse_num = PIT_OSC_FREQ * (OS_TICKS_MS / 1000);

    // 向端口映射写入数据
    /**
     * 通道 0 的输出连接到 8259A 的 IRQ0 引脚，从而能触发 CPU 中断！
     */
    outb(PIT_CMD_MODE_PORT, PIT_CHANNEL_0 | PIT_LOAD_LOHI | PIT_MODE_3);    // 写入通道 0，由低到高写，在模式 3 工作
    outb(PIT_CHL0_DATA_PORT, interval_pulse_num & 0xFF);            // 写入间隔脉冲数的低 8 位
    outb(PIT_CHL0_DATA_PORT, (interval_pulse_num >> 8) & 0xFF);     // 写入间隔脉冲数的高 8 位

    // 在中断向量表中注册时钟中断的处理函数
    set_interrupt_handler(IRQ0_TIMER, (uint32_t)interrupt_handler_timer);
    irq_enable(IRQ0_TIMER);
}