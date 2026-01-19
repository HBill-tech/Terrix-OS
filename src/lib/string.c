#include <cstd/string.h>
#include <kernel.h>

/**
 * 将 src 指向的字符串复制到 dst 为首地址的内存中
 * @param char *dst         目标地址
 * @param const char *src   源地址
 */
void kernel_strcpy(char *dst, const char *src) {
    // 若 dst 或 src 是空指针，则直接退出
    if (!dst || !src) {
        return;
    }

    // 冷知识: '\0' 的 ASCII 码是 0
    // 当 src 指向的字符不等于 '\0' 时，将 src 逐字节搬运到 dst 所指向的内存空间
    while (*src)
    {
        *dst++ = *src++;
    }
    // dst 指向的字符串需要以 '\0' 结尾
    *dst = '\0';
    
}

/**
 * 将 src 指向的字符串中的前 n 个字符复制到 dst 为首地址的内存中，如果 src 指向的
 * 字符串长度不够 n，那么久吧 src 全部复制到 dst
 * @param char *dst         目标地址
 * @param const char *src   源地址
 * @param uint32_t size     窗口大小
 */
void kernel_strncpy(char *dst, const char *src, uint32_t size) {
    // 目的地址 dst、源地址 dst 不为空
    // size 不能为 0
    // 否则直接退出
    if (!dst || !src || (size == 0)) {
        return;
    }

    char *d = dst;
    const char *s = src;

    // 先判断 size 是否大于 0, 之后 size 减 1
    // size要大于 0保证窗口没用完
    // 同时 src 指向的字符不是 '\0'
    while ((size > 0) && (*s))
    {
        *d++ = *s++;
        size--;
    }
    
    if (size == 0) {
        *(d - 1) = '\0';    // 如果窗口用完，就把窗口内最后一位换成 '\0'
    } else {
        *d = '\0';          // 如果窗口没用完但是字符串先复制完成，那么直接在字符串后加 '\0'
    }
}

int kernel_strncmp(const char *str1, const char * str2, uint32_t size) {
    // 若 str1 是字符串或 str2 是空字符串或 size = 0
    if (!str1 || !str2 || !size)
    {
        return -1;
    }

    const char *str1_copy = str1;
    const char *str2_copy = str2;
    
    // str1 和 str2 都没有遇到终止符 '\0' 
    // str1 与 str2 指向的字符相同
    // 窗口没有用完
    while (*str1_copy && *str2_copy && (*str1_copy == *str2_copy) && size > 0)
    {
        str1_copy++;
        str2_copy++;
        size--;
    }

    // 如果窗口用完了，说明两个字符串相同
    if (size == 0) {
        return 0;
    }
    // 否则返回最后一位的差
    return *(uint8_t*)str1_copy - *(uint8_t*)str2_copy;
    
}

uint32_t kernel_strlen(const char *str) {
    if (!str)
    {
        return 0;
    }

    const char *c = str;
    uint32_t length = 0;
    while (*c++) {
        length++;
    }
    
    return length;
}

/**
 * 将 src 指向的长度为 size 的内存区域复制到 dst 指向的内存区域
 * @param dst   目标地址
 * @param src   源地址
 * @param size  缓冲区长度(窗口大小)
 */
void kernel_memcpy(void *dst, void *src, uint32_t size) {
    // 特殊情况排查
    if (!dst || !src || size == 0) {
        return;
    }

    // 定义备份指针，之后的操作使用备份指针操作
    uint8_t *d = (uint8_t*)dst;
    uint8_t *s = (uint8_t*)src;
    while (size)
    {
        *d++ = *s++;
        size--;
    }
}

/**
 * 将内存中 dst 指向的一段长度为 size 的内存区域的值全部设置为 value
 * @param dst   目标地址
 * @param value 要设置的值
 * @param size  缓冲区大小
 */
void kernel_memset(void *dst, uint8_t value, uint32_t size) {
    // 特殊情况排查
    if (!dst || size == 0)
    {
        return;
    }
    uint8_t *d = (uint8_t*)dst;
    while (size)
    {
        *d++ = value;
        size--;
    }
}

int kernel_memcmp(void *v1, void *v2, uint32_t size) {
    if (!v1 || !v2 || size == 0)
    {
        return -1;
    }
    // 以一个字节为单位比较，因为 ASCII 码就是一个字节，如果指针指向的数据是两个字节将无法比较
    uint8_t *v1_copy = (uint8_t*)v1;
    uint8_t *v2_copy = (uint8_t*)v2;
    
    while (size)
    {
        if (*v1_copy != *v2_copy)
        {
            return 1;
        }
        
        v1_copy++;
        v2_copy++;

        size--;
    }

    return 0;
    
}