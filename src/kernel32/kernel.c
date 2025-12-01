#include <kernel.h>
#include <tty.h>
#include <stdio.h>

void test() {

    char *str = "123\b45\n678";

    // for (int i = 0; i < 26; i++)
    // {
    //     str[i*4] = 'h';
    //     str[i*4 + 1] = 'q'; 
    //     str[i*4 + 2] = (char)i + 'a';
    //     str[i*4 + 3] = '\n';
    // }
    
    
    tty_write(str, 10);


    // tty_write("\x7F", 1);

}

void hlos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();
    
    test();

    while(TRUE);
}