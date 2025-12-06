#include <kernel.h>
#include <tty.h>
#include <stdio.h>

void test() {



    char buffer[1024];
    char* ptr = buffer;

    ptr = number(ptr, -530, 10, 10, 5, ZEROPAD | SIGN);

    // for (int i = 0; i < 26; i++) 
    // {
    //     str[i*4] = 'h';
    //     str[i*4 + 1] = 'q';
    //     str[i*4 + 2] = (char)i + 'a';
    //     str[i*4 + 3] = '\n';
    // }
    tty_write("hlos/$HOME/usr> ", 16);
    // tty_write(buffer, 12);


    // tty_write("\x7F", 1);

}

void hlos_init(memory_info_t* mem_info, uint32_t gdt_info)
{
    tty_init();

    test();
    
    // tty_printf("hello world! %d", 1254);

    while(TRUE);
}