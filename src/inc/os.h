#ifndef HLOS_OS_H
#define HLOS_OS_H

// 内核(x32)代码段和数据段的段选择子
#define KERNEL_CODE_SEG     (1 * 8)     // 二进制: 0000 0000 0000 1000
#define KERNEL_DATA_SEG     (2 * 8)     // 二进制: 0000 0000 0001 0000


#define KERNEL_VERSION      "1.0.0"     // 内核版本
#define OP_SYS_VERSION      "1.0.0"     // 操作系统版本

#endif