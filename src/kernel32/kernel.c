#include <kernel.h>
#include <tty.h>
#include <stdio.h>

void hlos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();

    tty_write("hello word!", 12);
    
    // tty_printf("hello, %d!", 3434);

    while (TRUE);
}