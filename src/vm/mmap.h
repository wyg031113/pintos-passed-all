#ifndef __MMAP_H__
#define __MMAP_H__
#include<stdio.h>
#include<list.h>
#include"filesys/filesys.h"
#include"userprog/syscall.h"
#include"frame.h"
#include"page.h"
#include"userprog/pagedir.h"
struct MmapNode
{
    struct file *FilePtr;
    void *vaddr;
    int nPages;
    struct list_elem elem;
    int  id;
};
int IDAlloc(void);
bool WriteBackFile(struct MmapNode *mn,struct PageCon *pc);

#endif
