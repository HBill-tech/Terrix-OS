#ifndef HLOS_TIME_H
#define HLOS_TIME_H
/**
 * 系统时间
 */

#include <kernel.h>

/**
 * 描述时间的结构体
 */
typedef struct tm
{
    uint8_t tm_sec;     // 秒
    uint8_t tm_min;     // 分
    uint8_t tm_hour;    // 时
    uint8_t tm_mday;    // 日
    uint8_t tm_mon;     // 月
    uint8_t tm_year;    // 年
    uint8_t century;    // 世纪
    uint8_t tm_wday;    // 星期几
    int tm_isdst;    // 是否夏令时
} tm;

typedef uint32_t time_t;

void time_read_bcd(tm *time);
void time_read(tm *time, uint8_t timezone);
time_t mk_time_stamp(tm *time);
void time_init(uint8_t timezone);

#endif