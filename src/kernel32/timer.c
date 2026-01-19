#include <timer.h>
#include <pic.h>
#include <kernel.h>
#include <tty.h>
#include <logf.h>

static uint32_t ticks = 0;

/**
 * @param beeping               简单理解为已经蜂鸣的时长
 * @param beeping_threshold     蜂鸣时长的阈值，超过了就停止
 */
static uint32_t beeping = 0;
static uint32_t beeping_threshold = 1000;    // 蜂鸣器的 "鸣叫" or "关闭" 的周期时长为 1000 ms

/**
 * 时钟中断处理函数。
 * @param frame 传入的参数帧
 */
void handler_timer(interrupt_frame_t frame) {
    
    // if (beeping == 0) {
    //     // beeping = 0，开始蜂鸣
    //     start_beep();
    // } else if (beeping > beeping_threshold) {
    //     // beeping 超过了阈值，停止蜂鸣
    //     stop_beep();
    // }
    
    // // 由于时钟中断每 10 ms 触发一次，因此每调用一次 handler_timer 后， beeping 要加 10 ms
    // // 如果 beeping 达到了 2 倍阈值，一个 蜂鸣 -> 停止 周期完成，重新开始蜂鸣.
    // beeping = beeping == beeping_threshold * 2 ? 0 : beeping + 10;

    ticks++;                // 时钟打点次数加一
    send_eoi(IRQ0_TIMER);   // 表示中断处理完毕
    tty_logf("beeping: %d\n", beeping);
}

void start_beep() {
    // SPEAKER_REG 最后两位设置为 1
    outb(SPEAKER_REG, inb(SPEAKER_REG) | 0b11);
}

void stop_beep() {
    // SPEAKER_REG 最后两位设置为 0
    outb(SPEAKER_REG, inb(SPEAKER_REG) & 0xFC);
}

/**
 * 时钟中断芯片 8253 初始化
 * 配置计数器使得 handler_timer 能够被周期性调用
 * 配置蜂鸣器使得计算机能够发出声音
 */
void timer_init() {

    /********************************** 配置计数器 *************************************/
    // 时钟中断间隔内的脉冲数
    uint32_t interval_pulse_num = PIT_OSC_FREQ * (OS_TICKS_MS / 1000.0);
    // tty_logf(interval);

    // 向端口映射写入数据
    /**
     * 通道 0 的输出连接到 8259A 的 IRQ0 引脚，从而能触发 CPU 中断！
     */
    outb(PIT_CMD_MODE_PORT, PIT_CHANNEL_0 | PIT_LOAD_LOHI | PIT_MODE_3);    // 写入通道 0，由低到高写，在模式 3 工作
    outb(PIT_CHL0_DATA_PORT, interval_pulse_num & 0xFF);            // 写入间隔脉冲数的低 8 位
    outb(PIT_CHL0_DATA_PORT, (interval_pulse_num >> 8) & 0xFF);     // 写入间隔脉冲数的高 8 位

    /********************************** 配置蜂鸣器 *************************************/
    outb(PIT_CMD_MODE_PORT, PIT_CHANNEL_2 | PIT_LOAD_LOHI | PIT_MODE_3);
    outb(PIT_CHL2_DATA_PORT, (uint8_t)BEEP_COUNTER);
    outb(PIT_CHL2_DATA_PORT, (uint8_t)BEEP_COUNTER >> 8);


    // 在中断向量表中注册时钟中断的处理函数
    set_interrupt_handler(IRQ0_TIMER, (uint32_t)interrupt_handler_timer);
    irq_enable(IRQ0_TIMER);
}