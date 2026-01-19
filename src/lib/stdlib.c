#include <cstd/stdlib.h>

/**
 * BCD码是一种用4位二进制数表示1位十进制数的编码方式.
 * BCD码能表示的十进制整数范围是 0 ~ 99.
 * 
 * BCD码（Binary-Coded Decimal）和二进制数的区别
 * 对于 uint8_t 类型的数字 0x58:
 *      1. 如果按照BCD格式解码，得到的十进制数值是 58
 *      2. 如果按照二进制（即16进制）格式解码，得到的数值是 88
 */

/**
 * BCD 码转化为二进制格式
 *      - 计算机中，2进制、10进制、16进制都是通过同样的二进制代码存储的，
 *      - 因此只需要计算出 BCD 所表示的 10 进制数字即可，会自动转储为 2 进制.
 * @param value BCD 格式的数据
 */
uint8_t bcd_to_bin(uint8_t value) {
    return (value & 0xF) + ((value >> 4) & 0xF) * 10;
}

/**
 * 二进制格式转化为 BCD 码.
 *      - 这里的 BCD 码格式仍然是以二进制格式存储的，
 *      - 因此我们需要将一个二进制数通过运算将该数字在形式上转化为 BCD 格式.
 * @param value 二进制格式的数据
 */
uint8_t bin_to_bcd(uint8_t value) {
    return (value / 10) << 4 + (value % 10);
}