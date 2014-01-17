#include"page.h"
#include "hashfun.h"
#include "threads/vaddr.h"
#include <string.h>
#include "devices/timer.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"
#include "frame.h"
#include <inttypes.h>
#include <stdio.h>
#include "threads/interrupt.h"
#include "threads/vaddr.h"
#include "swap.h"
//#define DBGPAGE
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}
/*----------------lazy_load elf and mem map file-----------------------------------
 *   这个函数在加载可执行文件和内存映射文件时使用
 *   函数中创建了一个PageCon结构，把他加入到进程hash表和AllPage链表中。
 *   
 *--------------------------------------------------------------------------------*/
bool lazy_load (struct file *file,off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable,int is_code)
{
    int loadpages=0;
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

       static int x=0;
  struct thread *t=thread_current();
  //file_seek (file, ofs);
  off_t every_offs=ofs;
  while (read_bytes > 0 || zero_bytes > 0)
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
       size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
       size_t page_zero_bytes = PGSIZE - page_read_bytes;

       struct PageCon *pc=(struct PageCon *)malloc(sizeof(struct PageCon));
       if(pc==NULL)
       {
          printf("Error occur in malloc in lazy load\n");
          return false;
       }
       enum intr_level old_level=intr_disable(); //关中断
       list_push_back(&AllPage,&pc->all_elem);
       intr_set_level(old_level);
       //以下设置pc各项内容
       InitPageCon(pc);
       pc->offs=every_offs;
       pc->read_bytes=page_read_bytes;
       pc->zero_bytes=page_zero_bytes;
       pc->writable=writable;
       pc->is_code=is_code;
       pc->vir_page=upage; 
       pc->t=t;
       pc->FilePtr=file;
       hash_insert(&t->h,&pc->has_elem);
#ifdef DBGPAGE
       printf("add page %x\n",pc->vir_page);
       printf("lazy load page:%d\n",x++);
#endif
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
    //  if(is_code==4)
//	  printf("Add a page %x, offset=%x\n",pc->vir_page,every_offs);
      upage += PGSIZE;
      every_offs+=page_read_bytes;
      loadpages++;
    }
  //if(is_code==4)
    //  printf("real load pages %d\n",loadpages);
  return true;
}
/*------------------重新从文件或者交换分区载入页------------------------------
 * 对于被置换出去的页或者是lazy_load还未载入过内存的页，这个函数则根据
 * struct PageCon结构载入页。
 * */
bool reload(struct PageCon *pc)
{
        static int x=0;
    struct thread *t=thread_current();
    pc->phy_page=PageAlloc(PAL_USER);
    if(pc->phy_page==NULL)
    {
	printf("reload failed because no phy_page\n");
        return false;
    }   
    if(pc->is_code==0||(pc->is_code==2&&pc->writable==false)||pc->is_code==4)
	/*这个表示要载入的页是代码文件或者映射到内存的文件，这是首次载入，要从源文件载入  */
    {    
        /* Load this page. */
        file_seek(pc->FilePtr,pc->offs);
        if(file_read (pc->FilePtr,pc->phy_page, pc->read_bytes) != (int)pc->read_bytes)
        {
           palloc_free_page (pc->phy_page);
	   printf("reload read file fail!\n");
           return false;
        }
        memset (pc->phy_page+ pc->read_bytes, 0, pc->zero_bytes);

        /* Add the page to the process's address space. */
        if (!install_page (pc->vir_page, pc->phy_page, pc->writable))
        {
	   printf("install page error is_code=0 or 2\n");
           palloc_free_page (pc->phy_page);
           return false;
        }   
	if(pc->is_code==0)
	    pc->is_code=2;
	enum intr_level old_level=intr_disable();
	list_remove(&pc->all_elem);
	list_push_back(&PageUsed,&pc->all_elem);
	ICount++;
	intr_set_level(old_level);
//	if(pc->is_code==4)
//	    printf("load mem map page %x\n",pc->vir_page);
#ifdef DBGPAGE
	printf("reload page %d\n",x++);
#endif
	pagedir_set_dirty(pc->t->pagedir,pc->vir_page,false);
	return true;
    }
    else if(pc->is_code==1)            //pc->is_code==1意味着这个页在交换分区上
    {
        if (!install_page (pc->vir_page, pc->phy_page, pc->writable))
        {
           palloc_free_page (pc->phy_page);
	   printf("install page error is_code=1\n");
	   return false;
	}
        SwapReadPage(pc->offs,pc->phy_page);
	SwapPageFree(pc->offs);
	enum intr_level old_level=intr_disable();
        list_remove(&pc->all_elem);
	list_push_back(&PageUsed,&pc->all_elem);
	intr_set_level(old_level);
#ifdef DBGPAGE
	ICount++;
	printf("reload page %d\n",x++);
#endif
	return true;
    }
    else if(pc->is_code==3)           //3 表示这是lazy_load的stack 页,而且之前未使用过，
				      //只需要安装申请到页就可以。
    {
        if (!install_page (pc->vir_page, pc->phy_page, pc->writable))
        {
           palloc_free_page (pc->phy_page);
	   printf("first install stack page error is_code=1\n");
	   return false;
	}
	pc->is_code=1;               //这个页被安装上后，以后无论换入换出都在交换分区中。
	return true;
    }
    return false;
}

/*--------------------------------动态增长栈----------------------------------------------
 *    对于page_fault发生的这个页，立即装入一个物理页.
 *    如果地址大于fault_addr的页依然有没装入的，采取lazy_load方式
 *---------------------------------------------------------------------------------------*/
bool StackFault(struct intr_frame *f,bool not_present,bool wirte,bool user,void *fault_addr)
{
    struct thread *t=thread_current();
     if(user)
	 t->esp=f->esp;
     else if(t->esp>fault_addr)
	 return false;
     if(PHYS_BASE-(unsigned int)fault_addr>STACK_LIMIT || t->esp-32>fault_addr)
         return false;
     struct PageCon *pc=(struct PageCon *)malloc(sizeof(struct PageCon));
     if(pc==NULL)
     {
       printf("Error occur in malloc in install stack page\n");
       return false;
     }
      InitPageCon(pc);
      pc->vir_page=(int)(fault_addr)&~PGMASK;
      pc->phy_page=PageAlloc(PAL_USER);
      pc->t=t;
      if(pc->phy_page==NULL)
	  goto end;
     if (!(pagedir_get_page (t->pagedir, pc->vir_page) == NULL
          && pagedir_set_page (t->pagedir,pc->vir_page, pc->phy_page, pc->writable)))
      {
	  palloc_free_page(pc->phy_page);
	  free(pc);
	  return false;
      }
      pc->is_code=1;
      enum intr_level old_level=intr_disable(); //关中断
      list_push_back(&PageUsed,&pc->all_elem);
      ICount++;
      intr_set_level(old_level);
      hash_insert(&t->h,&pc->has_elem);	 
      /*  上边装入了发生page_fault的页，下面lazy_load其他页*/
      if(!LazyLoadStack(pc->vir_page+PGSIZE))
        return false;
      return true;

end:
      free(pc);
      
      return false;
}
/*-----------------------lazy_load 栈页---------------------------------
 * 从vir_page到0xBffff000如果有没有装入的页，立即lazy_load
 */
bool LazyLoadStack(void *vir_page)
{
    struct thread *t=thread_current();
    struct PageCon *pc;
    while(vir_page<0xBFFFF000&&page_lookup(&t->h,vir_page)==NULL)
    {
	pc=(struct PageCon*)malloc(sizeof(struct PageCon));
	if(pc==NULL)
	    return false;
	InitPageCon(pc);
	pc->vir_page=vir_page;
	pc->is_code=3;
	pc->t=t;
        enum intr_level old_level=intr_disable(); //关中断
        list_push_back(&AllPage,&pc->all_elem);
        intr_set_level(old_level);
        hash_insert(&t->h,&pc->has_elem);	 
#ifdef DBGPAGE
	printf("Lazy load stack %x\n",pc->vir_page);
#endif
        vir_page+=PGSIZE;
    }
    return true;
}
/*---------------------锁定一个页到内存中，使这个页不能被淘汰出去---------------
 *    如果要锁定的页已不在内存，reload这个页。
 *    LockTimes++锁定次数自增
 */
bool LockPage(void *vir_page)
{
    vir_page=(void*)((unsigned int)vir_page&~PGMASK);
    if(vir_page==0xBFFFF000)
	return true;
    struct thread *t=thread_current();
    if(!t->IsUser)
	return false;
    struct PageCon *pc=page_lookup(&t->h,vir_page);
    if(pc==NULL)
	return false;
    pc->LockTimes++;
    if(pagedir_get_page (t->pagedir, vir_page) == NULL)
   {
#ifdef DBGPAGE
       printf("Lock page %x",pc->vir_page);
#endif
        ASSERT(reload(pc));
   }
    return true;
}
/*---------解锁一个页-----------------------
 * 并没有把LockTimes赋值为0，只是--LockTimes
 * LockTimes==0时此页就解锁了。
 *----------------------------------------- */
bool FreeLockPage(void *vir_page)
{
    
    vir_page=(void*)((unsigned int)vir_page&~PGMASK);
    if(vir_page==0xBFFFF000)
	return true;
    struct thread *t=thread_current();
    if(!t->IsUser)
	return false;
    struct PageCon *pc=page_lookup(&t->h,vir_page);
    if(pc==NULL)
	return false;
    if(pc->LockTimes>=0)
	pc->LockTimes--;
    return true;
	
}
