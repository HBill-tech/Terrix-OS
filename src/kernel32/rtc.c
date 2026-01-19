#include <rtc.h>
#include <pic.h>
#include <tty.h>
#include <kernel.h>
#include <interrupt.h>
#include <cstd/time.h>
#include <cstd/stdlib.h>

/**
 * 从 CMOS 寄存器中读取数据
 * @param   addr  CMOS 寄存器的内存映射地址
 * @return  从 CMOS 寄存器读出的数据
 */
uint8_t cmos_read(uint8_t addr) {
    outb(CMOS_ADDR, CMOS_NMI | addr);
    return inb(CMOS_DATA);
}

/**
 * 向 CMOS 寄存器写入数据
 * @param addr  CMOS 寄存器的内存映射地址
 * @param value 写入 CMOS 寄存器的数据
 */
void cmos_write(uint8_t addr, uint8_t value) {
    outb(CMOS_ADDR, CMOS_NMI | addr);
    outb(CMOS_DATA, value);
}

/**
 * 强制编译器：
 * 1. 每次读取都从内存加载
 * 2. 每次写入都立即写入内存
 * 防止被优化成局部变量，导致读取的数据不准确
 */
static uint32_t volatile count = 0;

/**
 * rtc 中断的处理函数
 * @param frame 传入的参数帧
 */
void handler_rtc(interrupt_frame_t frame) {

    /**
     * CMOS_C寄存器
     * 只读，用来标识RTC中断的原因。
     * 如果不读，中断标志会保持设置，RTC可能不会产生新的中断。
     * 这里读取到的 vc 中第 5, 6 位分别是周期终端和闹钟中断的标志位
     */
    uint8_t vc = cmos_read(CMOS_C);      // 确保下一次 RTC 中断能够被触发
    
    create_alarm(1);

    // 判断触发 RTC 中断的类型
    if (vc & CMOS_C_AF) {
        // 闹钟中断
        tty_printf("A");
    } else if (vc & CMOS_C_PF) {
        // 周期中断
        tty_printf(".");
    }

    send_eoi(IRQ1_RTC);     // 告诉 PIC 中断结束
}

void create_alarm(uint32_t value) {
    tm time;
    time_read(&time, OS_TZ);    // 读取当前时间
    // sec 和 min 的范围就是 [0 ~ 60]，使用 uint8_t 足够
    uint8_t sec = value % 60;
    value /= 60;
    uint32_t min = value % 60;
    value /= 60;
    uint32_t hour = value;

    // 计算出 value 秒后的时间数据结构 time
    time.tm_sec += sec;
    if (time.tm_sec >= 60) {
        time.tm_sec %= 60;
        time.tm_min += 1;
    }

    time.tm_min += min;
    if (time.tm_min >= 60) {
        time.tm_min %= 60;
        time.tm_hour += 1;
    }
    
    time.tm_hour += hour;
    if (time.tm_hour >= 24)
    {
        // 例: 21 点设置 5 小时后的闹钟，取余后为 2 点，即会在第二天 2 点触发.
        time.tm_hour %= 24;
    }

    // 设置闹钟
    uint8_t h = bin_to_bcd(time.tm_hour - OS_TZ);
    uint8_t m = bin_to_bcd(time.tm_min);
    uint8_t s = bin_to_bcd(time.tm_sec);
    cmos_write(CMOS_HOUR_ALARM, bin_to_bcd(time.tm_hour - OS_TZ));
    cmos_write(CMOS_MINUTE_ALARM, bin_to_bcd(time.tm_min));
    cmos_write(CMOS_SECOND_ALARM, bin_to_bcd(time.tm_sec));
}

/**
 * 实时时钟中断的初始化函数
 */
void rtc_init() {
    /**
     * CMOS_B 端口写入数据各个位的意义
     * 位7: SET - 设置位 (0=正常操作，1=停止更新用于设置时间)
     * 位6: PIE - 周期性中断使能 (1=启用)
     * 位5: AIE - 闹钟中断使能 (1=启用)
     * 位4: UIE - 更新结束中断使能 (1=启用)
     * 位3: SQWE - 方波输出使能 (1=启用)
     * 位2: DM - 数据模式 (0=BCD，1=二进制)
     * 位1: 24/12 - 时间格式 (0=12小时制，1=24小时制)
     * 位0: DSE - 夏令时使能 (1=启用)
     */

    // 周期中断，闹钟中断
    uint8_t prev = cmos_read(CMOS_B);
    cmos_write(CMOS_B, prev | CMOS_B_24HOUR | CMOS_B_PIE | CMOS_B_AIE);
    // 清除所有潜在的中断标志，确保下一次 RTC 中断能够正常触发
    cmos_read(CMOS_C);
    
    // 设置闹钟
    create_alarm(1);

    // 设置中断频率 1110->250 ms 的周期
    // 0~3 位表示中断周期代码
    uint8_t prev_ = cmos_read(CMOS_A);
    cmos_write(CMOS_A, (prev_ & 0xF0) | 0b1110);

    set_interrupt_handler(IRQ1_RTC, (uint32_t)interrupt_handler_rtc);
    irq_enable(IRQ0_CASCADE);   // 使能级联引脚
    irq_enable(IRQ1_RTC);       // 使能 RTC
}