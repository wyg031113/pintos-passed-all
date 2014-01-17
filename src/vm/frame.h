#ifndef __FRAME_H__
#define __FRAME_H__
#include<stdio.h>
#include"threads/thread.h"
#include"filesys/file.h"
#include<list.h>
#include"threads/palloc.h"
struct PageCon
{
    struct file *FilePtr;//此页在文件中，保存文件指针
    uint8_t *vir_page;   //虚拟页 作为 hash 表的主键
    uint8_t *phy_page;   //物理页
    int    offs;         //该页在代码文件中的偏移 或者在swap分区中的位置
    bool writable;      //是否是只读页 
    uint32_t read_bytes; //需要读入的字节数
    uint32_t zero_bytes; //需要清0的字节数
    uint8_t is_code;    //is_code=0:lazy_load 的代码。1－此页从交换分区换入内存，换出是也换出到交换分区
                        //2－此页是代码，已经装入过内存一次了3－此页是分配到栈空间的
			//4－此页是分配给内存映射文件的
    int recent;          //LRU 
    struct thread *t;   //此页属于t进程
    int LockTimes;     //此页被锁定到内存的次数
    struct list_elem all_elem;    //链入AllPage 和UsedPage两个链表是用
    struct hash_elem has_elem;    //链入本进程hash 表用
};
extern struct list AllPage;      //所有不在内存中的页链入这个表。 
extern struct list PageUsed;     //所有已经在内存中的页链入这个表。
extern int Pages;
extern int ICount;
void PageInit(void);
void InitPageCon(struct PageCon *pc);
void *PageAlloc(enum palloc_flags flags);
void CountRecent(struct hash_elem *e,void *aux);
struct PageCon *FindMaxRecent(void);
void CountEveryPage(void);
#endif
