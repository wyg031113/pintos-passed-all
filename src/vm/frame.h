#ifndef __FRAME_H__
#define __FRAME_H__
#include<stdio.h>
#include"threads/thread.h"
#include"filesys/file.h"
#include<list.h>
#include"threads/palloc.h"
struct PageCon
{
    uint8_t *vir_page;   //虚拟页 作为 hash 表的主键
    uint8_t *phy_page;   //物理页
    int    offs;         //该页在代码文件中的偏移 或者在swap分区中的位置
    bool writable;      //是否是只读页 
    uint32_t read_bytes;
    uint32_t zero_bytes;
    uint8_t is_code;
    int recent;          //LRU 
    struct list_elem all_elem;
    struct hash_elem has_elem;
};
extern struct list AllPage;
void InitPageCon(struct PageCon *pc);
void *PageAlloc(enum palloc_flags flags);
#endif