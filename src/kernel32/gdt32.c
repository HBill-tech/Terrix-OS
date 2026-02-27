#include <kernel.h>

// 0: null, 1：code, 2：data, 3：usable
static uint32_t usable_index = 3;

// GDT 表的指针
static gdt_table_t *gdt;

/**
 * 初始化 GDT 表的指针
 * @param gdt_ptr GDT 表的首地址
 */
void gdt32_init(gdt_table_t *gdt_ptr)
{
    gdt = gdt_ptr + sizeof(gdt_table_t);

}