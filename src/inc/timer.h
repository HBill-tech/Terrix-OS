#ifndef HLOS_TIMER_H
#define HLOS_TIMER_H

/**
 * 可编程间隔定时器（PIT） 的相关配置
 */

#include <interrupt.h>

#define OS_TICKS_MS     10          // 操作系统的时间片粒度，操作系统每10ms产生一次时钟中断
#define PIT_OSC_FREQ    1193182     // PIT芯片的输入时钟频率


/************************** PIT 端口映射定义 ****************************/
#define PIT_CMD_MODE_PORT   0x43    // 控制 / 命令端口
#define PIT_CHL0_DATA_PORT  0x40    // 通道 0 的数据端口


/************************** PIT 控制字定义 ****************************/

/**
 * 位7-6: 通道选择    (00=通道0, 01=通道1, 10=通道2, 11=保留)
 * 位5-4: 访问模式    (00=锁存计数, 01=只低字节, 10=只高字节, 11=先低后高)
 * 位3-1: 工作模式    (000=模式0, 001=模式1, 010=模式2, 011=模式3, 100=模式4, 101=模式5)
 * 位0:   数制选择    (0=二进制, 1=BCD码)
 */
#define PIT_CHANNEL_0       (0 << 6)    // 选择通道 0
#define PIT_LOAD_LOHI       (3 << 4)    // 先写低字节，再写高字节
#define PIT_MODE_3          (3 << 1)    // 模式3: 方波发生器


void timer_init();

#endif