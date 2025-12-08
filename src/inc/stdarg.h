#ifndef HLOS_STDARG_H
#define HLOS_STDARG_H

/**
 * 使用 char * 定义 va_list，因此 va_list 访问数据时以字节为单位移动
 * 可使va_list类型的指针访问可变参数列表时更灵活的移动
 */
typedef char * va_list;     

/**
 * 开始访问: 使 ap 指向可变参数列表第一个元素。sizeof(char*) 在这里代表的是系统的标准参数槽大小
 * @param ap    va_list 类型的指针
 * @param v     最后一个固定参数
 */
#define va_start(arg_p, last) ((arg_p) = (va_list)((char*)&(last) + sizeof(last)))
/**
 * 访问列表中下一个变量
 * @param ap    va_list 类型的指针
 * @param t     将要访问的变量类型
 */
#define va_arg(ap, t)   (*(t*)((ap) += sizeof(char*)) - sizeof(char*))
/**
 * 结束访问: ap 赋值为 NULL，防止后续错误访问该列表
 * @param ap    当前的 va_list 指针
 */
#define va_end(ap)      ((ap) = (va_list)0)

#endif