#ifndef HLOS_TTY_H
#define HLOS_TTY_H

#include <types.h>

void tty_init();

uint32_t tty_write(char *buf, uint32_t count);

void tty_clear();

#endif