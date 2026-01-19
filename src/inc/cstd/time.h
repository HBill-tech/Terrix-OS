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
    int tm_sec;     // 秒
    int tm_min;     // 分
    int tm_hour;    // 时
    int tm_mday;    // 日
    int tm_mon;     // 月
    int tm_year;    // 年
    int century;    // 世纪
    int tm_wday;    // 星期几
    int tm_isdst;    // 是否夏令时
} tm;

typedef uint32_t time_t;

void time_read_bcd(tm *time);
void time_read(tm *time, int timezone);
time_t mk_time_stamp(tm *time);
void time_init(int timezone);

#endif