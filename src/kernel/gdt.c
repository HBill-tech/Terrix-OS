__asm__(".code16gcc");
#include <kernel.h>

// 定义 gdt 
gdt_table_t gdt_table[GDT_SIZE] = {
    {0, 0, 0, 0, 0}
};

// 设置 gdt 的表项
// 这里的 base 指在保护模式下段可以访问的起始地址, 因此该段实际能够访问的地址范围是 (base, base + limit)
void set_gdt_table_entry(int selector, uint32_t base, uint32_t limit, uint16_t attr) {

    // 如果段界限超过1MB（0xFFFFF），需要设置粒度位
    if (limit > 0xFFFFF)
    {
        attr |= 0x8000;         // 设置粒度位为1（4KB粒度）
        limit >>= 12;           // 将界限值转换为4KB页为单位
    }
    
    gdt_table_t *entry = &gdt_table[selector >> 3];                 // 获取描述符所引用的表项
    entry->limit_l = limit & 0xFFFF;                                // 设置界限的低 16 位
    entry->base_l = base & 0xFFFF;                                  // 设置段基地址的低 16 位
    entry->base_m = (base >> 16) & 0xFF;                            // 设置段基地址的中 8 位
    entry->attr = (attr & 0xF0FF) | (((limit >> 16) & 0xF) << 8);   // 设置属性字段
    entry->base_h = (base >> 24) & 0xFF;                            // 设置段基地址的高 8 位
}

void init_gdt() {
    // 初始化所有 GDT 表项
    for (int i = 0; i < GDT_SIZE; i++) {
        // i 左移三位, 变成 selector 格式
        set_gdt_table_entry(i << 3, 0, 0, 0);
    }
    // 平坦模式
    // 设置内核(x32)的数据段描述符
    set_gdt_table_entry(KERNEL_DATA_SEG, 0, 0xFFFFFFFF, 
        SEG_ATTR_P | SEG_ATTR_DPL0 | SEG_NORMAL | SEG_TYPE_DATA  | SEG_TYPE_RW | SEG_ATTR_D | SEG_ATTR_G);

    // 设置内核(x32)的代码段描述符
    set_gdt_table_entry(KERNEL_CODE_SEG, 0, 0xFFFFFFFF, 
        SEG_ATTR_P | SEG_ATTR_DPL0 | SEG_NORMAL | SEG_TYPE_CODE  | SEG_TYPE_RW | SEG_ATTR_D | SEG_ATTR_G);
    // 初始化 lgdt 寄存器
    lgdt((uint32_t)gdt_table, (uint16_t)sizeof(gdt_table));
    show_string("GDT init success!\r\n");
}