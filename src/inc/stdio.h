#ifndef HLOS_STDIO_H
#define HLOS_STDIO_H

#include <stdarg.h>

/************************************* 定义 vsprintf 中的格式化标志宏 **********************************/
#define ZEROPAD     1   // 用 0 填充空白，否则使用空格      %08d → 数字宽度不足8位时在前面补0
#define SIGN        2   // 指示当前处理的是有符号数，可能输出负号
#define PLUS        4   // 正数前显示加号    %+d → 正数显示为"+123"，负数显示为"-123"
#define SPACE       8   // 正数前加空格      % d → 正数显示为" 123"，负数显示为"-123"
#define LEFT        16  // 左对齐           %-10s → 字符串左对齐，右侧填充空格
#define SPECIAL     32  // 特殊前缀         8 进制前加 0, 16 进制前加 0x
#define SMALL       64  // 16进制使用小写字母

int vsprintf(char *buf, const char *fmt, va_list args);

char *number(char *str, int num, int base, int size, int precision, int type);
#endif