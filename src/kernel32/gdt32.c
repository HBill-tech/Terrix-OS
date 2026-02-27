#include <kernel.h>

// 0: null, 1：code, 2：data, 3：usable
static uint32_t usable_index = 3;

// GDT 表的指针
static gdt_table_t *gdt;

/**
 * 重载 GDT 表的指针
 * BTW：重载GDT是确保内核稳定运行的必要步骤, 这样做肯定没问题，而不重载则存在多种潜在风险. 
 *      后续在 32 位内核中对于GDT的操作通过对重载后的指针进行操作.
 * @param gdt_ptr GDT 表的首地址
 */
void gdt32_init(gdt_table_t *gdt_ptr)
{
    gdt = gdt_ptr;
    lgdt(gdt, GDT_SIZE * sizeof(gdt_table_t));
}

uint32_t alloc_gdt_table_entry() {
    // 从可用索引开始，寻找一个 attr == 0 的描述符
    for (uint32_t i = usable_index; i < GDT_SIZE; i++) {
        // attr == 0, 因此 P 位为 0，表示该段不可用
        if ((gdt + i)->attr == 0)
        {
            usable_index = i + 1; // 更新下一个可用索引
            return i << 3; // 返回选择子（索引 * 描述符大小）
        }
    }
    return -1; // 没有可用的描述符
}

/**
 * 释放 GDT 表中的一个描述符
 * @param selector 要释放的描述符的选择子
 */
void free_gdt_table_entry(uint32_t selector) {
    gdt[selector >> 3].attr = 0; // 将 attr 设置为 0，表示该段不可用
}

void set_gdt_table_entry(uint32_t selector, uint32_t base, uint32_t limit, uint16_t attr) {
    if(limit > 0xFFFFF) {
        // 如果 limit 超过 20 位的最大值，则需要启用 granularity 位，并将 limit 以 4KB 为单位进行调整
        attr |= 0x8000; // 设置 granularity 位
        limit >>= 12; // 将 limit 以 4KB 为单位进行调整
    }

    gdt_table_t *entry = &gdt[selector >> 3]; // 获取描述符的地址
    entry->base_l = base & 0xFFFF; // 设置基地址的低 16 位
    entry->base_m = (base >> 16) & 0xFF; // 设置基地址的中 8 位
    entry->base_h = (base >> 24) & 0xFF; // 设置基地址的高 8 位
    entry->limit_l = limit & 0xFFFF; // 设置界限的低 16 位
    entry->attr = attr | (((limit >> 16) & 0xF) << 8); // 设置属性
}