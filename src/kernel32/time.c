/**
 * 系统时间
 */
#include <cstd/time.h>
#include <cstd/stdlib.h>
#include <rtc.h>
#include <tty.h>
#include <logf.h>

#define SECONDS_PER_MINUTE  60                          // 每分钟的秒数
#define SECONDS_PER_HOUR    (60 * SECONDS_PER_MINUTE)   // 每小时的秒数
#define SECONDS_PER_DAY     (24 * SECONDS_PER_HOUR)     // 每天的秒数
#define SECONDS_PER_YEAR    (365 * SECONDS_PER_DAY)     // 每年的描述，按照365天算  

static int passed_day_of_month_leap_year[13] = {
    0,      // 没有 0 月，占位
    0,                                
    31,                                 
    31 + 29,                            
    31 + 29 + 31,                       
    31 + 29 + 31 + 30,
    31 + 29 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
};

static int passed_day_of_month_common_year[13] = {
    0,      // 没有 0 月，占位
    0,                                
    31,                                 
    31 + 28,                            
    31 + 28 + 31,                       
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
};


time_t time_stamp;      // 时间戳

/**
 * 从 CMOS 时间寄存器中读取 BCD 格式的时间数据
 * @param time  记录时间的数据结构指针
 */
void time_read_bcd(tm *time) {
    do {
        time->tm_sec = cmos_read(CMOS_SECOND);
        time->tm_min = cmos_read(CMOS_MINUTE);
        time->tm_hour = cmos_read(CMOS_HOUR);
        time->tm_wday = cmos_read(CMOS_WEEKDAY);
        time->tm_mday = cmos_read(CMOS_DAY);
        time->tm_mon = cmos_read(CMOS_MONTH);
        time->tm_year = cmos_read(CMOS_YEAR);
        time->century = cmos_read(CMOS_CENTURY);
        
    } while (time->tm_sec != cmos_read(CMOS_SECOND));
    // 如果秒数发生了更新，那么要重新读取所有数据，确保所有的数据都是两次 CMOS 更新之间读到的
}

/**
 * 将从 CMOS 读取到的时间转化为 BCD 格式，并且换算到当前时区
 * @param time      记录时间的数据结构指针
 * @param timezone  时区
 */
void time_read(tm *time, int timezone) {
    time_read_bcd(time);
    // 将时间数据从 bcd 格式转化为二进制格式，方便后续数据处理与打印
    time->tm_sec = bcd_to_bin(time->tm_sec);
    time->tm_min = bcd_to_bin(time->tm_min);
    time->tm_hour = (bcd_to_bin(time->tm_hour) + timezone) % 24;
    time->tm_wday = bcd_to_bin(time->tm_wday);
    time->tm_mday = bcd_to_bin(time->tm_mday);
    time->tm_mon = bcd_to_bin(time->tm_mon);
    time->tm_year = bcd_to_bin(time->tm_year);
    time->tm_isdst = -1;
    time->century = bcd_to_bin(time->century);
}

/**
 * 构造当前时间下的时间戳
 * @param time  tm 类型的时间数据指针
 */
time_t mk_time_stamp(tm *time) {
    time_t res;
    /**
     * year  = time->tm_year     +       100         +       -70
     *        A.D.2000 based        A.D.1900 based      A.D.1970 based
     */
    int year = time->tm_year + 30;
    // 这些年（从1970开始）经过的秒数
    res = SECONDS_PER_YEAR * year;

    // 从 1970 年开始到当前经过了闰年多过的秒数，不考虑世纪年，但是适用于 20 ～ 21世纪
    res += SECONDS_PER_DAY * ((year + 1) / 4);

    // 今年已经过完的月份的时间
    if ((year + 2) % 4 == 0) {
        res += passed_day_of_month_leap_year[time->tm_mon] * SECONDS_PER_DAY;
    } else {
        res += passed_day_of_month_common_year[time->tm_mon] * SECONDS_PER_DAY;
    }

    // 这个月已经过去的天
    res += SECONDS_PER_DAY * (time->tm_mday - 1);

    // 今天过去的小时
    res += SECONDS_PER_HOUR * time->tm_hour;

    // 这个小时过去的分钟
    res += SECONDS_PER_MINUTE * time->tm_min;

    // 这个分钟已经过去的秒
    res += time->tm_sec;

    return res;
}

/**
 * 初始化系统时间
 */
void time_init(int timezone) {
    tm time;
    time_read(&time, timezone);
    
    tty_printf("=======================STARTUP TIME: %d%02d-%02d-%02d %02d:%02d:%02d========================\n",
        time.century,
        time.tm_year,
        time.tm_mon,
        time.tm_mday,
        time.tm_hour,
        time.tm_min,
        time.tm_sec
    );

    time_stamp = mk_time_stamp(&time);
    tty_logf("time stamp %d\n", time_stamp);
}