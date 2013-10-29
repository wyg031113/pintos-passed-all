#ifndef __PAGE_H__
#define __PAGE_H__
#include<stdio.h>
#include"filesys/file.h"
#include"vm/frame.h"
#include"threads/synch.h"
bool reload(struct PageCon *pc);
bool lazy_load (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable);
void InitPageMan(void);
extern struct lock LockAllPageList;
#endif
