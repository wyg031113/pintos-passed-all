#ifndef __PAGE_H__
#define __PAGE_H__
#include<stdio.h>
#include"filesys/file.h"
#include"vm/frame.h"
#include"threads/synch.h"
#include"threads/interrupt.h"
#define STACK_LIMIT 0x400000
bool reload(struct PageCon *pc);
bool lazy_load (off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable,int is_code);
void InitPageMan(void);
extern struct lock LockAllPageList;
bool StackFault(struct intr_frame *f,bool not_present,bool write,bool user,void *fault_addr);
bool LockPage(void *vir_page);
bool FreeLockPage(void *vir_page);
bool LazyLoadStack(void *vir_page);
#endif
