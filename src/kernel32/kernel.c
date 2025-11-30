#include <kernel.h>
#include <tty.h>
#include <stdio.h>

void test() {

    char *str;

    for (int i = 0; i < 26; i++)
    {
        str[i*4] = 'h';
        str[i*4 + 1] = 'q'; 
        str[i*4 + 2] = (char)i + 'a';
        str[i*4 + 3] = '\n';
    }
    
    
    tty_write(str, 104);
}

void hlos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();
    
    test();

    while (TRUE);
}