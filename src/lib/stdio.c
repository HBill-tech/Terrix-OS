#include <stdio.h>
#include <stdarg.h>
#include <kernel.h>
#include <types.h>
#include <string.h>

#define is_digit(c)     ((c) >= '0' && (c) <= '9')      // 判断一个 ASCII 码是否是数字

/**
 * ASCII 码数字转化为 int 类型的数字，同时跳过输入字符串的数字部分
 * @param   s       字符串二级指针，指向目标字符串
 * @return  itg     字符串中的数字部分转化为 int 类型的结果 
 */
static int skip_atoi(const char **s) {
    
    int itg = 0;            // 存储解析出来的数字
    while (is_digit(**s))
    {
        itg = itg * 10 + (**s - '0');
        (*s)++;
    }
    return itg;
}

/************************************* 定义 vsprintf 中的格式化标志宏 **********************************/
#define ZEROPAD     1   // 用 0 填充空白，否则使用空格      %08d → 数字宽度不足8位时在前面补0
#define SIGN        2   // 指示当前处理的是有符号数，可能输出负号
#define PLUS        4   // 正数前显示加号    %+d → 正数显示为"+123"，负数显示为"-123"
#define SPACE       8   // 正数前加空格      % d → 正数显示为" 123"，负数显示为"-123"
#define LEFT        16  // 左对齐           %-10s → 字符串左对齐，右侧填充空格
#define SPECIAL     32  // 特殊前缀         8 进制前加 0, 16 进制前加 0x
#define SMALL       64  // 16进制使用小写字母

/**
 * 余数除法
 * 
 * 
 * 汇编指令 divl r
 * 输入: edx:eax -------> 64位被除数, 寄存器 r ------------> 除数
 * 输出：eax -----------> 32位商，edx ------------> 32位余数
 * 
 * @param   n       32 位被除数
 * @param   base    32 位除数
 * @return  __res   32 位余数
 * @return  n       32 位商
 */
#define do_div(n, base) ({              \
    int __res;                          \
    __asm__("divl %4"                   \
            :"=a"(n), "=d"(__res)       \
            :"0"(n), "1"(0), "r"(base)  \
    );                                  \
    __res;                              \
})

/**
 * 将整数转换为指定格式的字符串，tty_printf的核心逻辑在这里.
 * @param   str       输出缓冲区指针
 * @param   num       要转换的数字
 * @param   base      进制 [2, 36]
 * @param   size      缓冲区长度(宽度)
 * @param   precision 精度，输出数字的最小位数
 * @param   type      格式化标志位
 * 
 * @return  str       写入该数字之后，指针在缓冲区中指向的位置，方便调用者接着写
 * 
 * type & FLAG = True 说明 type 中含有这个 Flag，反之不含有
 */
static char *number(char *str, int num, int base, int size, int precision, int type) {
    char padding, sign, tmp[36];
    const char * digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;

    /******************************* 检查标志位的内容 *********************************/
    if (type & SMALL) {
        digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    }
    if (type & LEFT) {
        type &= ~ZEROPAD;   // 左对齐时不用 0 填充，把 ZEROPAD 置为 0
    }
    // 进制在合理范围
    if (base < 2 || base > 36) {
        return 0;
    }

    // 确定符号
    if (type & SIGN && num < 0) {
        sign = '-';
        num = -num;
    } else {
        sign = (type & PLUS) ? '+' : (type & SPACE) ? ' ' : 0;
    }

    // 减去符号占用的缓冲区宽度
    if (sign) {
        size--;
    }
    // 进制标识站用的缓冲区宽度
    if (type & SPECIAL) {
        if (base == 16) {
            size -= 2;
        } else if (base == 8) {
            size--;
        }
    }
    // 确定填充字符是 0 还是空格
    padding = (type & ZEROPAD) ? '0' : ' ';

    // 将数字转化为指定进制的字符串
    i = 0;  // i 用来索引 tmp，同时用来表示最终 tmp 中数字的位数
    if (num == 0) {
        tmp[i++] = digits[0];   // '0'
    } else {
        while (num != 0) {
            // 除 n 取余法由 10 进制转 n 进制，此时 tmp 存储 n 进制数字的倒序
            int res = do_div(num, base);
            tmp[i++] = digits[res];
        }
    }

    if (i > precision) {    // 这里可以有效处理 precision = -1 的情况
        precision = i;
    }
    size -= precision;  // 剩余宽度减去数字长度
    

    /********************************** 开始向 str 中写入数字 ******************************/
    /**
     * 空格填充 且 右对齐
     * 符号、前缀之前输出空格
     */
    if (!(type & (ZEROPAD | LEFT))) {
        while (size > 0)
        {
            *str++ = ' ';
            size--;
        }
    }

    /**
     * 如果是右对齐，空格填充，符号应该在空格之后
     * 如果是右对齐，0填充，符号应该在空格之前
     * 如果是左对齐，这是输出的第一个字符
     */
    if (sign) {     // 如果有符号字符
        *str++ = sign;
    }

    if (type & SPECIAL) {   // 如果需要输出进制标志
        if (base == 8) {
            *str++ = '0';
        } else if (base == 16) {
            *str++ = '0';
            *str++ = digits[33];
        }
    }

    /**
     * 此判断只会在 零填充 右对齐 时候执行
     * 符号、前缀之后，数字之前输出补位字符
     */
    if (!(type & LEFT))
    {
        while (size > 0)
        {
            *str++ = padding;
            size--;
        }
    }
    
    /**
     * 如果数字长度没有精度要求长，那么要高位补零
     */
    while (i < precision)
    {
        *str++ = '0';
        precision--;
    }

    /**
     * 输出数字的内容
     */
    while (i > 0)
    {
        i--;
        *str++ = tmp[i];
    }

    /**
     * 左对齐，前面的字符都输入完了，用空格填充最后的内容
     */
    while (size > 0)
    {
        *str++ = ' ';
        size--;
    }

    return str;
}

/**
 * 将可变参数列表插入到格式化字符串中，放置到缓冲区
 * @param   buf     缓冲区
 * @param   fmt     格式化字符串
 * @return  args    可变参数列表 
 */
int vsprintf(char *buf, const char *fmt, va_list args) {
    int len;            // 存储 %s 情况中的字符串长度
    int i;              // for 循环的循环变量
    char * str;         // 跟踪缓冲区的当前位置
    char * s;           // 处理 %s 情况时存储字符串指针
    int * ip;
    int condition;      // do - while 循环中用到的判断变量

    int flags;          // number 函数中用到的 flags

    int field_width;    // 输出缓冲区宽度
    int precision;      // 输出的数字的最小位数

    int qualifier;      /* 整数字段格式说明符的修饰符。其中 h: short ; l: long; L: long long */

    for (str = buf; *fmt; fmt++) {
        if (*fmt != '%')        // 如果不是 % 号，那么一定没到格式化部分
        {
            *str++ = *fmt;
            continue;
        }

        /**
         * 如果第一个 if 分支没有拦截成功，那么说明遇到了 %
         * 此时把 % 之后所有可能出现的格式化标志符都拦截一下，下一次循环就一定是普通字符了。
         */

        fmt++;          // 跳过 % 号，开始处理格式化标志符

        /* 处理标志位 flags */
        flags = 0;      // 初始化 flags
        
        do {
            switch (*fmt) {
                case '-':
                    flags |= LEFT;
                    fmt++;
                    condition = 1;
                    break;
                case '+':
                    flags |= PLUS;
                    fmt++;
                    condition = 1;
                    break;
                case ' ':
                    flags |= SPACE;
                    fmt++;
                    condition = 1;
                    break;
                case '#':
                    flags |= SPECIAL;
                    fmt++;
                    condition = 1;
                    break;
                case '0':
                    flags |= ZEROPAD;
                    fmt++;
                    condition = 1;
                    break;
                default:
                    condition = 0;
                    break;
            }
        } while (condition);

        /* 获取输出缓冲区宽度 */
        field_width = -1;
        if (is_digit(*fmt)) {
            field_width = skip_atoi(&fmt);
        } else if (*fmt == '*') {    // 从可变参数列表当前参数获取宽度
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = - field_width;
                flags |= LEFT;
            }
            fmt++;
        }

        /* 获取输出精度 */
        precision = -1;
        if (*fmt == '.') {
            fmt++;
            if (is_digit(*fmt)) {
                precision = skip_atoi(&fmt);
            } else if (*fmt == '*') {   // 从可变参数列表当前参数获取精度
                precision = va_arg(args, int);
                fmt++;
            }
            if (precision < 0) {
                precision = 0;
            }
        }

        /* 获取整数标志位修饰符 */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
            qualifier = *fmt;
            fmt++;
        }

        /* 按照指定类型输出 */
        switch (*fmt)
        {
            /**
             * %[type_flag] -> 输出无符号整数流程: 
             *      1.%[type_flag] 被 tty_printf 接收
             *      2.传入 vsprintf 被解析成 flags
             *      3.将要打印数字的类型转化为 number 函数在当前 flags 下所要求的输入数字类型，否则会发生错误
             *      4.通过 number 函数将输入的数字转化为 flags 所指定格式的字符串
             * 
             * 问题，格式化输出中 %[type_flag] 的输出格式是通过什么实现的？
             *      是上述一整个流程的作用，而不是某个孤立的环节。
             */
            case 'c':
                if (!(flags & LEFT)) {  // 右对齐
                    while (--field_width > 0) {
                        *str++ = ' ';   // 左侧补空格
                    }   
                }
                *str++ = va_arg(args, char);
                while (--field_width > 0) { // 左对齐
                    *str++ = ' ';
                }
                break;
            case 'd':
                flags |= SIGN;  // %d 输出有符号数
                // 不用管 field_width，如果小于零，那么 number 中仅仅是不填充了，也就是紧密输出
                str = number(str, va_arg(args, int), 10, field_width, precision, flags);
                break;
            case 'i':
                flags |= SIGN;  // %d 输出有符号数
                // 不用管 field_width，如果小于零，那么 number 中仅仅是不填充了，也就是紧密输出
                str = number(str, va_arg(args, int), 10, field_width, precision, flags);
                break;
            case 'o':
                flags |= SPECIAL;
                str = number(str, va_arg(args, unsigned int), 8, field_width, precision, flags);
                break;
            case 'p':
                if (field_width == -1) {
                    field_width = 8;
                }
                flags |= ZEROPAD;
                flags |= SPECIAL;
                str = number(str, (unsigned long) va_arg(args, void *), 16, field_width, precision, flags);
                break;
            case 's':
                s = va_arg(args, char *);
                len = kernel_strlen(s);
                if (precision < 0) {
                    precision = len;
                } else if (precision < len) {   // 精度小于len，那么只输出精度的长度
                    len = precision;
                }
                if (!(flags & LEFT)) {      // 右对齐
                    while (len < field_width--) {
                        *str++ = ' ';
                    }
                }
                for (i = 0; i < len; i++) {
                    *str++ = *s++;
                }
                while (len < field_width--) {   // 左对齐
                    *str++ = ' ';
                }
                break;
            case 'x':
                flags |= SPECIAL;
                flags |= SMALL;
                str = number(str, va_arg(args, unsigned int), 16, field_width, precision, flags);
                break;
            case 'X':
                flags |= SPECIAL;
                str = number(str, va_arg(args, unsigned int), 16, field_width, precision, flags);
                break;
            case 'u':
                str = number(str, va_arg(args, unsigned int), 10, field_width, precision, flags);
                break;
            case 'n':
                ip = va_arg(args, int*);
                *ip = (str - buf);
                break;

            default:
                if (*fmt == '%') {  // 如果这个字符是是百分号，那么%%输出%
                    *str++ = '%';
                } else if(*fmt) {   // 未知字符，输出 %[未知字符]
                    *str++ = '%';
                    *str++ = *fmt;
                } else {    // 当前字符是字符串末尾 '\0'
                    fmt--;  // 格式化字符串回退一位，保证循环正确结束
                }
        }
    }

    *str = '\0';        // 字符串结尾
    return str - buf;   // 输出字符串长度
}