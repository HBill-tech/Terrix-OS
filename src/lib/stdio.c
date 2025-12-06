#include <stdio.h>
#include <stdarg.h>
#include <kernel.h>

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
 * 将整数转换为指定进制的字符串
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
char *number(char *str, int num, int base, int size, int precision, int type) {
    char padding, sign, tmp[36];
    const char * digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;

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

    // 符号占用的缓冲区空间
    if (sign) {
        size--;
    }
    // 进制标识站用的缓冲区空间
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
        tmp[i++] = 0;
    } else {
        while (num != 0) {
            // 除 n 取余法由 10 进制转 n 进制，此时 tmp 存储 n 进制数字的倒序
            tmp[i++] = do_div(num, base) + '0';
        }
    }

    if (i > precision) {
        precision = i;
    }
    size -= precision;  // 剩余宽度减去数字长度
    
    /**
     * 不用零填充且不左对齐
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
     * 不是左对齐
     * 此判断只会在 零填充 非左对齐 时候执行
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
     * 如果是右对齐，前面的字符都输入完了，用空格填充最后的内容
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

}