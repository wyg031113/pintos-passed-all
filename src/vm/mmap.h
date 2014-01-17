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
    struct file *FilePtr;     //映射文件指针
    void *vaddr;	      //文件映射到的内存首地址
    int nPages;		     //这个文件需要占用多少个内存页
    struct list_elem elem;
    int  id;		     //返回的内存文件句柄。
};
int IDAlloc(void);
bool WriteBackFile(struct PageCon *pc);
struct MmapNode *GetMapNodeFromID(struct thread *t,int id);
bool UnMapFile(struct thread *cur,struct MmapNode *mn);
#endif
