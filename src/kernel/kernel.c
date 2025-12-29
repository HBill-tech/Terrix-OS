__asm__(".code16gcc");
#include <kernel.h>

void kernel_init() {
    // 内存检测
    memory_check();
    // 开启 A20 地址线
    cli();                      // 关中断
    uint8_t v = inb(0x92);      // 开启 A20 地址线
    outb(0x92, v | 0x02);
    // 加载 GDT 
    init_gdt();
    // 开启保护模式 (cr0 第 0 位为 1)
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | (1 << 0));      // 设置PE位，启用保护模式
    
    // 远跳转刷新 CS 为 KERNEL_CODE_SEG, EIP加载protect_mode的32位地址
    // CPU 正式进入保护模式
    // 这里的 protect_mode 编码为什么是32位而不是16位呢?
    // 因为 protect_mode 在 kernel32/start.asm 中实现，被编译成 32 位的代码
    far_jump(KERNEL_CODE_SEG, (uint32_t)protect_mode);
}