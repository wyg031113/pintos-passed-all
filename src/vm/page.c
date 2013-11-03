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
bool lazy_load (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  struct thread *t=thread_current();
  file_seek (file, ofs);
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
    //  printf("add 1 page!\n");     
       InitPageCon(pc);
       pc->offs=every_offs;
       pc->read_bytes=page_read_bytes;
       pc->zero_bytes=page_zero_bytes;
       pc->writable=writable;
       pc->is_code=0;
       pc->vir_page=upage; 
       pc->t=t;
       hash_insert(&t->h,&pc->has_elem);
      // printf("add page %x\n",pc->vir_page);
#ifdef DBGPAGE
static int x=0;
      printf("lazy load page:%d\n",x++);
#endif
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      upage += PGSIZE;
      every_offs+=page_read_bytes;
    }
//       lock_release(&LockAllPageList);
  return true;
}
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
    if(pc->is_code==0||(pc->is_code==2&&pc->writable==false))
    {    
        /* Load this page. */
        file_seek(t->FileSelf,pc->offs);
        if(file_read (t->FileSelf,pc->phy_page, pc->read_bytes) != (int)pc->read_bytes)
        {
           palloc_free_page (pc->phy_page);
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
	pc->is_code=2;
	enum intr_level old_level=intr_disable();
	list_remove(&pc->all_elem);
	list_push_back(&PageUsed,&pc->all_elem);
	ICount++;
	intr_set_level(old_level);
#ifdef DBGPAGE
	printf("reload page %d\n",x++);
#endif
	return true;
    }
    else if(pc->is_code==1)
    {
        if (!install_page (pc->vir_page, pc->phy_page, pc->writable))
        {
           palloc_free_page (pc->phy_page);
	   printf("install page error is_code=1\n");
	   return false;
	}
	/*int i;
	for(i=0;i<100;i++)
	    *(char *)pc->phy_page=i;
	*/
	//printf("run here!\n");
         SwapReadPage(pc->offs,pc->phy_page);
	 SwapPageFree(pc->offs);
	enum intr_level old_level=intr_disable();
        list_remove(&pc->all_elem);
	list_push_back(&PageUsed,&pc->all_elem);
	ICount++;
	intr_set_level(old_level);
#ifdef DBGPAGE
	printf("reload page %d\n",x++);
#endif
	return true;
    }
    return false;
}

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
      pc->vir_page=(int)(fault_addr)&0xFFFFF000;
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
     // printf("add 1 page\n");
      pc->is_code=1;
      enum intr_level old_level=intr_disable(); //关中断
      list_push_back(&PageUsed,&pc->all_elem);
      ICount++;
      intr_set_level(old_level);
 
      hash_insert(&t->h,&pc->has_elem);	 
      return true;

end:
      free(pc);
      
      return false;
}

bool LockPage(void *vir_page)
{
    vir_page=(void*)((unsigned int)vir_page&0xFFFFF000);
    struct thread *t=thread_current();
    if(!t->IsUser)
	return false;
    struct PageCon *pc=page_lookup(&t->h,vir_page);
    if(pc==NULL)
	return false;
    pc->LockTimes++;
    if(pagedir_get_page (t->pagedir, vir_page) == NULL)
	ASSERT(reload(pc));
    return true;
}

bool FreeLockPage(void *vir_page)
{
    vir_page=(void*)((unsigned int)vir_page&0xFFFFF000);
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
