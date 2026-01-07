#include <kernel.h>

#define SECTOR_SIZE 512
#define OS_ADDR     0x90000    // 操作系统内核放入内存的起始地址

/**
 * 将硬盘的内容读取到内存中
 * @param   sector  要读取的起始扇区
 * @param   count   扇区数量
 * @param   buffer  读入内存的首地址
 */
void read_disk(uint32_t sector, uint32_t count, uint16_t* buffer) {
    // 0xE0 = 11100000b 
    // 位7-5: 111 = 选择主硬盘，LBA模式
    // 位4: 0 = 主硬盘
    // 位3-0: LBA地址的24-27位
    outb(0x1F6, 0xE0);      

    outb(0x1F2, (uint8_t)(count >> 8));     // 扇区数高字节
    outb(0x1F3, (uint8_t)(sector >> 24));   // LBA 24-31位
    outb(0x1F4, 0);                         // LBA 32-39位
    outb(0x1F5, 0);                         // LBA 40-47位

    outb(0x1F2, (uint8_t)count);            // 扇区数低字节
    outb(0x1F3, (uint8_t)sector);           // LBA 0-7位
    outb(0x1F4, (uint8_t)(sector >> 8));    // LBA 8-15位  
    outb(0x1F5, (uint8_t)(sector >> 16));   // LBA 16-23位

    outb(0x1F7, 0x24);  // 0x24 = 读取DMA命令（或者可能是READ SECTORS EXT）

    uint16_t *ptr = buffer;
    while(count --) {
        // 0x08 = 00001000
        // 位7(BSY)必须为0 - 硬盘不忙
        // 位3(DRQ)必须为1 - 数据已准备好可以传输
        // 0x88 是掩码，只关注第 7 和 3 位
        while((inb(0x1F7) & 0x88) != 0x8);
        // uint8_t status;
        // do {
        //     status = inb(0x1F7);
        //     // if (status == 0xFF)
        //     // {
        //     //     show_string("Disk controller not found or port error\r\n");
        //     // }
            
        // } while((status & 0x80) || !(status & 0x08));
        for (int i = 0; i < SECTOR_SIZE / 2; i++) {
            *ptr++ = inw(0x1F0);    // 从数据寄存器0x1F0每次读取2字节
        }
    }
}

/**
 * 这个函数是ELF加载器的核心部分，负责解析ELF文件格式并将程序加载到内存中
 * @param   buffer 临时存储ELF文件内容的内存缓冲区首地址
 * @return  内核的入口地址，如果失败返回 0
 */
uint32_t read_elf_header(uint8_t *buffer) {
    // 把 buffer 装填成 elf 的格式
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*)buffer;
    // ELF文件头验证, 检查 ELF 魔数
    if (elf_header->e_ident[0] != 0x7F  ||
        elf_header->e_ident[1] != 'E'   ||
        elf_header->e_ident[2] != 'L'   ||
        elf_header->e_ident[3] != 'F'
    ) {
        return 0;
    }
    // 遍历遍历程序头表（Program Headers）
    // e_phnum: 程序头表中段的数量
    for (int i = 0; i < elf_header->e_phnum; i++) {
        // e_phoff: 程序头表在 ELF 文件中的偏移量
        // phdr:    当前遍历到的段应该被加载到的位置
        Elf32_Phdr *phdr = (Elf32_Phdr *)(buffer + elf_header->e_phoff) + i;

        // 筛选可加载段，这些是需要加载到内存中的段
        if (phdr->p_type != PT_LOAD) {
            continue;
        }

        uint8_t *src = buffer + phdr->p_offset;     // 找到段在文件里的起始位置
        uint8_t *dst = (uint8_t*)phdr->p_paddr;     // 找到段要加载到的内存物理地址
        // 把文件里的数据搬到内存（逐个字节）
        for (int j = 0; j < phdr->p_filesz; j++)
        {
            *dst++ = *src++;
        }
        
        // ELF 文件中 BSS 段是不占空间的，但是在内存中需要占用空间，初值为 0.
        // 链接过程会为 ELF 文件的每个段生成段首的物理地址 phdr->p_paddr, 
        // 这个物理地址是将上一个段的 BSS 占用的内存空间预估在内的,
        // 因此直接在该段的文件数据之后为 BSS 段赋 0 值就可以，不会和下一个段的数据发生地址冲突

        // 给 BSS 段清零
        dst = (uint8_t*)phdr->p_paddr + phdr->p_filesz;
        // p_memsz - p_filesz 这段多出来的内存就是BSS
        for (int j = 0; j < phdr->p_memsz - phdr->p_filesz; j++)
        {
            *dst++ = 0;
        }
    }

    // -Ttext=0x100000 在链接脚本中指定了代码段的加载地址
    // elf_header->e_entry 是 ELF 文件的入口地址, 这个地址等于:
    //      1. 基于-Ttext指定的基地址
    //      2. 加上入口符号 _start 在代码中的偏移量
    return elf_header->e_entry;
}

extern memory_info_t memory_info;           // 定义在 memory.c
extern gdt_table_t gdt_table[GDT_SIZE];     // 定义在 gdt.c

void kernel32_init()
{
    // 0号扇区: 引导扇区 MBR
    // 1~9: Kernel x16
    // 10~*: Kernel x86
    read_disk(10, 100, (uint16_t*)OS_ADDR);
    uint32_t addr = read_elf_header((uint8_t*)OS_ADDR);
    if (addr == 0)
    {
        while (TRUE);
    }

    // 跳转到 x86 内核
    ((void (*)(memory_info_t*, uint32_t))addr)(&memory_info, (uint32_t)&gdt_table);
}