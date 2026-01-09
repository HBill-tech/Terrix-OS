#ifndef HLOS_RTC_H
#define HLOS_RTC_H

#include <kernel.h>

#define CMOS_ADDR   0x70    // CMOS 地质寄存器，用于选择要访问的CMOS寄存器
#define CMOS_DATA   0x71    // CMOS 数据寄存器，从选定的CMOS寄存器读取或写入数据的端口
#define CMOS_NMI    0x80    // 当设置CMOS地址寄存器时，如果设置这一位，将禁用NMI中断。以防被CMOS操作被NMI打断导致不准确

// RTC的控制/状态寄存器
#define CMOS_A      0x0A    // 控制更新频率、分频器等
#define CMOS_B      0x0B    // 控制各种功能，如24/12小时制、夏令时、更新中断等
#define CMOS_C      0x0C    // 包含中断标志，读取后自动清除
#define CMOS_D      0x0D    // 指示CMOS电池状态

// 时间寄存器定义
#define CMOS_SECOND 0x01
#define CMOS_MINUTE 0x03
#define CMOS_HOUR   0x05

void rtc_init();

uint8_t cmos_read(uint8_t addr);

void cmos_write(uint8_t addr, uint8_t value);

#endif