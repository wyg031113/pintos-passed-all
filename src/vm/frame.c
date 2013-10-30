#include"frame.h"
#include"threads/palloc.h"
#include<list.h>
#include<hash.h>
#include"userprog/pagedir.h"
#include"swap.h"
#include"threads/interrupt.h"
struct list AllPage;
void InitPageCon(struct PageCon *pc)
{
    pc->vir_page=NULL;
    pc->phy_page=NULL;
    pc->offs=-1;
    pc->writable=true;
    pc->recent=0;
    pc->is_code=0;
    pc->t=NULL;
    pc->InMem=true;
}
void *PageAlloc(enum palloc_flags flags)
{
   void * phy_page=palloc_get_page(flags);
   struct PageCon *pc;
   enum intr_level old_level=intr_disable();
   while(phy_page==NULL)
   {
        pc=FindMaxRecent();
	if(pc==NULL)
	    break;
	pc->InMem=false;
	if(!(pc->is_code==0&&!pagedir_is_dirty(pc->t->pagedir,pc->vir_page)))
	{
             pc->offs=SwapOutPage(pc->vir_page);
	     pc->is_code=1;

	}
	pagedir_clear_page(pc->t->pagedir,pc->vir_page);
	phy_page=palloc_get_page(flags);
   }
   intr_set_level(old_level);
   return phy_page;
}
//LRU
void CountRecent(struct hash_elem *e,void *aux)
{
    struct PageCon *pc=hash_entry(e,struct PageCon,has_elem);
    struct thread *t=pc->t;
    if(t->pagedir==NULL)
	return;
    if(!pagedir_is_accessed(t->pagedir,pc->vir_page))
	pc->recent++;
    else
	pagedir_set_accessed(t->pagedir,pc->vir_page,false);

}
struct PageCon *FindMaxRecent(void)
{
    int recent=0;
    struct PageCon *pc=NULL,*MaxPC=NULL;
    struct list_elem *e;
    for(e=list_begin(&AllPage);e!=list_end(&AllPage);e=list_next(e))
    {
	pc=list_entry(e,struct PageCon,all_elem);
	if(pc->recent>recent&&pc->InMem)
	{
	    recent=pc->recent;
	    MaxPC=pc;
	}
    }
    return MaxPC;
}

