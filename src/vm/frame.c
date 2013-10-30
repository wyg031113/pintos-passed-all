#include"frame.h"
#include"threads/palloc.h"
#include<list.h>
#include<hash.h>
#include"userprog/pagedir.h"
#include"swap.h"
#include"threads/interrupt.h"
struct list AllPage;
struct list PageUsed;
int Pages=0;
int IUsed=0;
int ICount=0;
void InitPageMan(void)
{
    list_init(&AllPage);
    list_init(&PageUsed);
}
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
   int error_code=0;
   while(phy_page==NULL)//||IUsed>380)
   {
       enum intr_level old_level=intr_disable();
        pc=FindMaxRecent();
       intr_set_level(old_level);
	if(pc==NULL)
	{
	    error_code=1;
	    break;
	}   
	pc->InMem=false;
	if(!(pc->is_code==0&&!pagedir_is_dirty(pc->t->pagedir,pc->vir_page)))
	{
	    
             pc->offs=SwapOutPage(pc->vir_page);
	     if(pc->offs==-1)
	     {
		 error_code=2;
                  printf("no Swap space\n");
		  break;
	     }	
	     pc->is_code=1;

	}
	palloc_free_page(pc->phy_page);
	pagedir_clear_page(pc->t->pagedir,pc->vir_page);
	pc->recent=0;
	IUsed--;
	pc->phy_page=NULL;
         old_level=intr_disable();
	list_remove(&pc->all_elem);
	list_push_back(&AllPage,&pc->all_elem);
	ICount--;
   intr_set_level(old_level);
	//printf("Swap out a Page\n");
  if(phy_page==NULL)
        phy_page=palloc_get_page(flags);
   }
   //if(phy_page==NULL)
  if(phy_page==NULL)
	    printf("No page in mem! error_code=%d\n",error_code);
  else IUsed++;
 //  printf("used pages=%d  but IUsed=%d\n",Pages,IUsed);   
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
void CountEveryPage(struct thread *t)
{
    struct list_elem *e;
    for(e=list_begin(&PageUsed);e!=list_end(&PageUsed);e=list_next(e))
    {
	struct PageCon *pc=list_entry(e,struct PageCon,all_elem);
	struct thread *t=pc->t;
	if(t->pagedir==NULL)
	    continue;
	if(!pagedir_is_accessed(t->pagedir,pc->vir_page))
	    pc->recent++;
	else
	    pagedir_set_accessed(t->pagedir,pc->vir_page,false);

    }
}
struct PageCon *FindMaxRecent(void)
{
    int recent=0;
    struct PageCon *pc=NULL,*MaxPC=NULL;
    struct list_elem *e;
    int n=0;
    for(e=list_begin(&PageUsed);e!=list_end(&PageUsed);e=list_next(e))
    {
	pc=list_entry(e,struct PageCon,all_elem);
	if(pc->recent>=recent)
	{
	    recent=pc->recent;
	    MaxPC=pc;
	}
	n++;
    }
//    printf("have %d phy_page in memroy,but ICount=%d\n",n,ICount);
    return MaxPC;
}

