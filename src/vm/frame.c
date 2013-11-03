#include"frame.h"
#include"threads/palloc.h"
#include<list.h>
#include<hash.h>
#include"userprog/pagedir.h"
#include"swap.h"
#include"threads/interrupt.h"
#include"devices/timer.h"
//#define DBG
struct list AllPage;
struct list PageUsed;
int Pages=0;
int IUsed=0;
int ICount=0;
bool InitOk=false;
void InitPageMan(void)
{
    list_init(&AllPage);
    list_init(&PageUsed);
    InitOk=true;
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
    pc->LockTimes=0;
}
void *PageAlloc(enum palloc_flags flags)
{
   void * phy_page=palloc_get_page(flags);
   struct PageCon *pc=NULL;
   int error_code=0;
   while(phy_page==NULL)
   {
       enum intr_level old_level;
       pc=NULL;
       while(pc==NULL)
       {
          old_level=intr_disable();
	   //CountEveryPage();
           pc=FindMaxRecent();
	  if(pc==NULL)
	  {
	   intr_set_level(old_level);
	   timer_sleep(5);
	  }    //thread_yield();
	  else
	      pagedir_clear_page(pc->t->pagedir,pc->vir_page);
       }
/*	if(pc==NULL)
	{
	   // error_code=1;
	    intr_set_level(old_level);
	   // break;
	   thread_yield();
	   continue;
	}   
	*/
	list_remove(&pc->all_elem);
	list_push_back(&AllPage,&pc->all_elem);
	ICount--;
   intr_set_level(old_level);
//	if(!(pc->is_code==0&&!pagedir_is_dirty(pc->t->pagedir,pc->vir_page)))
        if( !(pc->is_code==2&&pc->writable==false))
	{
	    
             pc->offs=SwapOutPage(pc->phy_page);
	     if(pc->offs==-1)
	     {
		 error_code=2;
                  printf("no Swap space\n");
		  break;
	     }	
	     pc->is_code=1;

	}
	palloc_free_page(pc->phy_page);
	pc->recent=0;
	IUsed--;
	pc->phy_page=NULL;
	//printf("Swap out a Page\n");
  if(phy_page==NULL)
        phy_page=palloc_get_page(flags);
   }
   //if(phy_page==NULL)
  if(phy_page==NULL)
	    printf("No page in mem! error_code=%d\n",error_code);
  else IUsed++;
#ifdef DBG
  printf("used pages=%d  but IUsed=%d\n",Pages,IUsed);   
 #endif
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
void CountEveryPage()
{
   if(!InitOk)
       return;
   // printf("I am run!\n");
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
	{
	    pagedir_set_accessed(t->pagedir,pc->vir_page,false);
	    pc->recent=0;
	} 

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
	if(pc->recent>recent&&pc->LockTimes==0)
	{
	    recent=pc->recent;
	    MaxPC=pc;
	}
	n++;
    }
  #ifdef DBG
    printf("have %d phy_page in memroy,but ICount=%d\n",n,ICount);
  #endif
    //printf("max recent is %d\n",recent);
    return MaxPC;
}

