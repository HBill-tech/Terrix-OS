#include <logf.h>
#include <kernel.h>
#include <cstd/stdarg.h>
#include <cstd/string.h>
#include <cstd/stdio.h>

/**
 * 初始化串口 tty 功能
 */
void tty_logf_init() {
    outb(COM2_PORT + 1, 0x00);      // 0x3F9 -> 中断使能寄存器IER  禁用所有串口中断

    // 设置 LCR，启用 DLAB，允许访问波特率除数寄存器
    outb(COM2_PORT + 3, 0x80);      

    // 设置波特率除数（115200 / 0x03），DLAB=1 时 0x3F8 和 0x3F9 分别表示低 8 位和高 8 位
    outb(COM2_PORT + 0, 0x03);      // 低 8 位
    outb(COM2_PORT + 1, 0x00);      // 高 8 位

    // 设置 LCR，关闭 DLAB
    outb(COM2_PORT + 3, 0x03);      

    // 设置 FIFO 控制寄存器（FCR)，阈值 14 字节，立即清空缓冲区
    // 设置了 FCR，多个字符可以积累在缓冲区，CPU一次读取
    outb(COM2_PORT + 2, 0xC7);

    // 设置 MODEM 控制寄存器（MCR)
    // 在PC架构中，OUT2信号控制中断到8259A的硬件通路，硬件通路必须打开
    outb(COM2_PORT + 4, 0x0B);
}

/**
 * 通过串口打印字符到 stdio
 */
void tty_logf(const char *fmt, ...) {
    char buf[1024];
    va_list args;
    kernel_memset(buf, '\0', sizeof(buf));  // 缓冲区清零
    va_start(args, fmt);
    vsprintf(buf, fmt, args);   // 把格式化字符串和可变参数列表拼起来
    va_end(args);

    const char *p = buf;
    while (*p != 0)     // 字符串结束('\0')为止
    {
        // while 循环的条件：确保移位寄存器为空，上一个字符已完全发送
        while (!(inb(COM2_PORT + 5) & (1 << 6)));
        outb(COM2_PORT, *p++);
    }
    while (!(inb(COM2_PORT + 5) & (1 << 6)));
    outb(COM2_PORT, '\r');
    while (!(inb(COM2_PORT + 5) & (1 << 6)));
    outb(COM2_PORT, '\n');
}