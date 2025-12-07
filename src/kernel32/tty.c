#include <string.h>
#include <kernel.h>
#include <tty.h>
#include <stdarg.h>
#include <stdio.h>

/***************************************** 宏定义 **********************************************/

// 将 CRT_ADDR_REG 设置为具体的寄存器索引，从而使得 CRT_DATA_REG 成为该寄存器的 “代理”
// 这两个宏定义了与CRT控制器通信的I/O端口，用于配置显示参数
#define CRT_ADDR_REG        0x3D4   // CRT控制器地址寄存器的端口
#define CRT_DATA_REG        0x3D5   // CRT控制器数据寄存器的端口

// 一些寄存器索引
// 寄存器数据：当前屏幕第一个字符在显存中的字符偏移量
#define CRT_START_ADDR_H    0xC     // 高8位寄存器
#define CRT_START_ADDR_L    0xD     // 低8位寄存器

// 寄存器数据：当前光标所在位置在显存中的字符偏移量
#define CRT_CURSOR_H        0xE     // 高8位寄存器
#define CRT_CURSOR_L        0xF     // 低8位寄存器

// 内存映射宏定义
#define MEM_BASE    0xB8000                 // 文本模式现存起始地址
#define MEM_SIZE    0X4000                  // 显存大小(16KB)
#define MEM_END     (MEM_BASE + MEM_SIZE)   // 显存结束地址，该地址是恰好不属于显存的第一个地址

// 屏幕参数宏定义
#define WIDTH       80                  // 屏幕宽度
#define HEIGHT      25                  // 屏幕高度
#define ROW_SIZE    (WIDTH * 2)         // 每行占用字节数，每显示一个字符需要一个字节设置内容一个字节设置颜色
#define SCR_SIZE    (ROW_SIZE * HEIGHT) // 整个屏幕占用的字节数

// 定义一些字符的 ASCII 码
#define ASCII_NULL  0x00    // 空字符
#define ASCII_ENQ   0x05    // 询问字符
#define ASCII_BEL   0x07    // 响铃字符
#define ASCII_BS    0x08    // 退格键
#define ASCII_HT    0x09    // 水平制表符
#define ASCII_LF    0x0A    // 换行符
#define ASCII_VT    0x0B    // 垂直制表符
#define ASCII_FF    0x0C    // 换页符
#define ASCII_CR    0x0D    // 回车符
#define ASCII_DEL   0x7F    // 删除字符

/********************************************* 全局变量定义 ******************************************/

// 屏幕首字符在显存的当前位置，字节编址
static uint32_t screen;

// 光标在显存的位置，字节编址
static uint32_t cursor;

/**
 * @param x, y  光标位置，使用字符编址
 * 既然这两个变量并不会直接参与到光标位置的设定中，那这里为什么要用到光标的字符编制坐标呢? 
 * 1. 我们可以利用这两个坐标进行更简单的条件判断
 * 2. x, y 参与到了特殊字符的操作中，如 CR 回车符，如果 x 更新不正确将导致 CR 操作不正确
 */
static uint16_t x = 0, y = 0;
// 字符样式
static uint8_t attr = 7;
// 空格字符，两字节大小
static uint16_t erase = 0x0720;


/***************************** CRT(阴极射线管)寄存器的 getter 和 setter方法 **************************/

/**
 * 向 screen 中读取当前屏幕第一个字符在显存中的位置
 */
static void get_screen() {
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);   // 选择CRT_START_ADDR_H寄存器
    screen = inb(CRT_DATA_REG) << 8;        // 将读取到的地址左移8位
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);   // 选择CRT_START_ADDR_L寄存器
    screen |= inb(CRT_DATA_REG);            // 合并高低字节的地址
    // screen 转化为字节单位(x2)
    screen <<= 1;
    // 计算实际显存地址
    screen += MEM_BASE;
}

/**
 * 使显示器从 screen 所在的显存位置开始显示
 */
static void set_screen() {
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    outb(CRT_DATA_REG, (screen - MEM_BASE) >> 9 & 0xFF);        // 将screen的高8位(字符单位)写入CRT_START_ADDR_H寄存器
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    outb(CRT_DATA_REG, (screen - MEM_BASE) >> 1 & 0xFF);        // 将screen的低8位(字符单位)写入CRT_START_ADDR_L寄存器
}

/**
 * 获取当前游标的内存位置以及游标在当前屏幕中的横纵坐标
 */
static void get_cursor() {
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    cursor = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    cursor |= inb(CRT_CURSOR_L);

    // 转换为字节编址
    cursor << 1;
    // 计算当前光标的在内存的绝对地址
    cursor += MEM_BASE;

    get_screen();

    // 计算当前 cursor 相对于 screen 的字符偏移量
    uint32_t relative_cursor = (cursor - screen) >> 1;
    x = relative_cursor % WIDTH;    // 计算出光标所在的列(0开始)
    y = relative_cursor / WIDTH;    // 计算出光标所在的行(0开始)
}

/**
 * 将光标移动到 cursor 指向的位置上
 */
static void set_cursor() {
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    outb(CRT_DATA_REG, ((cursor - MEM_BASE) >> 9) & 0xFF);
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    outb(CRT_DATA_REG, ((cursor - MEM_BASE) >> 1) & 0xFF);
}

/*************************************** ASCII码操作 ****************************************/
/**
 * 退格操作
 */
static void com_bs() {
    if (x)
    {
        x--;
        // 光标地址前移两个字节
        cursor-=2;
        *(uint16_t*)cursor = erase;
    }
}

/**
 * 删除操作
 */
static void com_del() {
    *(uint16_t*)cursor = erase;
}

/**
 * 回车操作
 */
static void com_cr() {
    // 从当前光标位置向前移动 x * 2 个字节
    cursor -= (x << 1);
    x = 0;
}

/**
 * 滚动屏幕
 */
static void scroll_up() {
    // 如果当前显示区域加上一行就超出显存末尾，则将屏幕内容放在显存开始位置
    if (screen + SCR_SIZE + ROW_SIZE >= MEM_END) {
        kernel_memcpy((uint32_t*)MEM_BASE, (uint32_t*)screen, SCR_SIZE);    
        cursor -= (screen - MEM_BASE);
        screen = MEM_BASE;
    }
    // 清理即将展示的一行
    uint32_t *ptr = (uint32_t*)(screen + SCR_SIZE);
    for (int i = 0; i < WIDTH; i++)
    {
        *ptr++ = erase;
    }
    // 屏幕和游标都向下滚动一行
    screen += ROW_SIZE;
    cursor += ROW_SIZE;

    set_screen();
    set_cursor();
}

/**
 * 换行操作
 */
static void com_lf() {
    if (y < HEIGHT - 1)
    {
        y++;
        cursor += ROW_SIZE;
        return;
    }
    scroll_up();
}

/******************************************* tty 功能函数 **************************************/

/**
 * tty 打印功能初始化
 */
void tty_init() {
    tty_clear();
}

/**
 * 清屏函数
 */
void tty_clear() {
    screen = MEM_BASE;
    set_screen();
    cursor = MEM_BASE;
    set_cursor();

    // 两个字节表示一个字符，因此创建一个指向两字节数据的指针方便处理
    uint16_t *ptr = (uint16_t*)MEM_BASE;
    // 显存空间是 [MEM_BASE, MEM_END)
    while (ptr < (uint16_t*)MEM_END)
    {
        *ptr++ = erase;
    }
}

/**
 * 把给定缓冲区 buffer 中指定长度 count 的字符串输出到屏幕中当前光标指向的位置，如果屏幕显示不下则会滚屏显示。
 * 
 * tty_init之后，如果所有函数都通过调用 tty_write 来显示文字，运行了tty_write之后
 *      1.字符串所有字符依次正常输出
 *      2.光标位置一定在屏幕中
 *      3.写入的字符一定不会越界显存
 * 
 * @param   buf     给定的缓冲区
 * @param   count   缓冲区长度
 * @return  written 该函数最终输出的字符数
 */
uint32_t tty_write(char *buf, uint32_t count) {
    char c;
    uint32_t written = 0;       // 已输出的字符数量
    while (written++ < count)
    {
        c = *buf++;
        tty_write_char(c);      // cursor 范围在[screen, screen + SCR_SIZE + 2]
    }
    set_cursor();
    return written;
}

/**
 * 在当前光标位置输出一个 ASCII 码字符，如果屏幕显示不下会滚屏显示
 * 
 * 该函数是自保持的，只要输入是合法的，那么输出也是合法的，可以将该输出作为输入继续调用。
 * 因此只要第一次输入是合法的，且任意两次调用中间不改变输入的变量，那么连续调用n次该函数都合法。
 *                                  |
 *                                  |   不变量观点
 *                                  V
 * 这个函数执行前必须满足不变量I，如果它的输入状态满足不变量I，那么输出一定满足不变量I。
 * 因此只要最初状态不变量成立，且两次函数调用期间不破坏不变量，那么连续调用 n 次该函数之后不变量I依然成立。
 * 
 * 定义不变量 I(screen, cursor, x, y) : 
 *      cursor 属于 [screen, screen + SCR_SIZE + 2]
 *      cursor , screen 属于 [MEM_BASE, MEM_END]
 *      x, y 属于 [0, 24], [0, 79]
 * 
 * @param c 输出的字符内容
 */
void tty_write_char(char c) {
    switch (c)
    {
        case ASCII_NULL:    // 空字符
            break; 
        case ASCII_BS:      // 退格
            com_bs();
            break;
        case ASCII_DEL:     // 删除
            com_del();
            break;
        case ASCII_CR:      // 回车
            com_cr();
            break;
        case ASCII_FF:      // 换行
            com_lf();
            break;
        case ASCII_LF:      // 回车 + 换行
            com_lf();
            com_cr();
            break;
        default:
            // 输出非功能性字符

            // 当 x 超出屏幕宽度的处理逻辑
            if (x >= WIDTH) {
                x -= WIDTH;             // 计算出游标换行后的横坐标
                cursor -= ROW_SIZE;
                com_lf();               // 这里会复原 cursor
            }
            
            // 在光标位置写入字符
            char *ptr = (char*)cursor;  // 指向当前光标内存位置处的一字节数据
            *ptr = c;
            *(ptr + 1) = attr;

            // 更新指针与坐标
            cursor += 2;
            x += 1;
            break;
    }
}
 
#define BUFFER_SIZE 1024
static char buf[BUFFER_SIZE];

/**
 * 可变参数的 printf
 * @param   fmt   格式化字符串
 * @param   ...   可变参数列表
 * @return  i     打印出的字符串长度
 */
int tty_printf(const char *fmt, ...) {
    kernel_memset(buf, 0, BUFFER_SIZE);
    va_list args;
    int i;
    va_start(args, fmt);            // 开始访问可变参数列表
    i = vsprintf(buf, fmt, args);   // i 表示将fmt插入数据之后写入到缓冲区的长度
    va_end(args);                       // 结束访问可变参数列表
    tty_write(buf, (uint32_t)i);
    return i;                       // 返回打印的字符串长度
}