#ifndef HLOS_RTC_H
#define HLOS_RTC_H

/**
 * 实时时钟中断 RTC
 */

#include <kernel.h>

/*********************************** CMOS 控制线/数据线 *************************************/
#define CMOS_ADDR   0x70    // CMOS 地址寄存器，用于选择要访问的CMOS寄存器
#define CMOS_DATA   0x71    // CMOS 数据寄存器，从选定的CMOS寄存器读取或写入数据的端口
#define CMOS_NMI    0x80    // 当设置CMOS地址寄存器时，如果设置这一位，将禁用NMI中断。以防被CMOS操作被NMI打断导致不准确

/*********************************** 寄存器端口映射 *****************************************/
// RTC的控制/状态寄存器
#define CMOS_A      0x0A    // 控制更新频率、分频器等
#define CMOS_B      0x0B    // 控制各种功能，如24/12小时制、夏令时、更新中断等
#define CMOS_C      0x0C    // 包含中断标志，读取后自动清除
#define CMOS_D      0x0D    // 指示CMOS电池状态

// 闹钟寄存器
#define CMOS_SECOND_ALARM   0x01
#define CMOS_MINUTE_ALARM   0x03
#define CMOS_HOUR_ALARM     0x05

// 系统时间寄存器定义
#define CMOS_SECOND     0x00
#define CMOS_MINUTE     0x02
#define CMOS_HOUR       0x04
#define CMOS_WEEKDAY    0x06
#define CMOS_DAY        0x07
#define CMOS_MONTH      0x08
#define CMOS_YEAR       0x09
#define CMOS_CENTURY    0x32

// 控制位 Flags
#define CMOS_B_24HOUR   (1 << 1)        // 控制时间格式, 24h
#define CMOS_B_AIE      (1 << 5)        // 开启闹钟中断
#define CMOS_B_PIE      (1 << 6)        // 开启周期性中断

// 当发生周期性中断时，该位会自动置 1. 读取寄存器 C 后，该位会被清除.
#define CMOS_C_AF       (1 << 5)        // 周期性中断标志

// 当闹钟时间到达并触发中断时，该位会自动置 1. 读取寄存器 C 后会被清除.
#define CMOS_C_PF       (1 << 6)        // 闹钟中断标志

void rtc_init();

uint8_t cmos_read(uint8_t addr);

void cmos_write(uint8_t addr, uint8_t value);

void create_alarm(uint32_t value);

#endif