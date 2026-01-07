#ifndef HLOS_LOGF_H
#define HLOS_LOGF_H

/**************************************** 定义串口的端口映射 *************************************/
#define COM1_PORT   0x3F8
#define COM2_PORT   0x2F8
#define COM3_PORT   0x3E8
#define COM4_PORT   0x2E8

void tty_logf_init();

void tty_logf(const char* fmt, ...);

#endif