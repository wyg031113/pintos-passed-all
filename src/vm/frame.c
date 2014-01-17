#include"frame.h"
#include"threads/palloc.h"
#include<list.h>
#include<hash.h>
#include"userprog/pagedir.h"
#include"swap.h"
#include"threads/interrupt.h"
#include"devices/timer.h"
//#define DBG
/*      所有不在内存中的页链入AllPage链表.这些页包括了lazy_load 的代码；内存映射文件中lazy_load的文件；
 *  被换出到交换分区上的页。
 *      所有已经装入到内存的页都链入PageUsed链表。
 */
struct list AllPage; 
struct list PageUsed;
int Pages=0;
int IUsed=0;
int ICount=0;
bool InitOk=false;   //虚拟内存机制是否初始化完毕。

/*这个函数在init.c:main()函数中被调用，初始化了AllPage, PageUsed两个链表。*/
void InitPageMan(void)
{
    list_init(&AllPage);
    list_init(&PageUsed);
    InitOk=true;
}

/* 初始化一个新生的PageCon结构*/
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
    pc->FilePtr=NULL;
}

/*---------------------内存分配器--------------------------------------------------------------------
 * 此内存分配器封装了原有的palloc_get_page()；
 * 在内存不足时，会淘汰最久未使用的页面
 * 对不同的页面的淘汰是不同的。
 * 1.对于代码，因为是只读的，淘汰出内存时不用写到交换分区(pc->is_code==2&&pc->writable==false)
 *   调入内存时，只需要从原来的可执行文件载入就行。
 * 2.对于已经改过的数据页和栈页，淘汰要写入交换分区，载入时从交换分区载入。(pc->is_code==1或3)
 * 3.对于映射文件的页，淘汰出时如果此页已经被修改，直接写回文件，如果未被修改则不用写回(pc->is_code==
 *   4)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void *PageAlloc(enum palloc_flags flags)
{
   void * phy_page=palloc_get_page(flags);
   struct PageCon *pc=NULL;
   int error_code=0;
   //如果申请页失败，则一直把内存中的页往出淘汰。
   //如果没有可以淘汰的页，则一直等待到有页可换出
   //以下关中断是为了同步使用PageUsed链表和AllPage链表。
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
       //将置换出去的页从PageUsed链表中取出，插入AllPage链表。
	list_remove(&pc->all_elem);
	list_push_back(&AllPage,&pc->all_elem);
	ICount--;
        intr_set_level(old_level);
	if(pc->is_code==4)
	{
	    //printf("run FrameAlloc\n");
	    if(pagedir_is_dirty(pc->t->pagedir,pc->vir_page))
		WriteBackFile(pc);
	    else
		printf("file not change page=%x\n",pc->vir_page);
	}
	else
        if( !(pc->is_code==2&&pc->writable==false))  //对于代码本身是只读的，不用写入交换分区
	{					    //其他情况写入交换分区
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
       if(phy_page==NULL)
           phy_page=palloc_get_page(flags);
   }

  /*以下代码是在出错是调试使用*/
  if(phy_page==NULL)
	    printf("No page in mem! error_code=%d\n",error_code);
  else IUsed++;
  if(((unsigned)phy_page&0xFFF)!=0)
  {
      printf("get a wrong page\n");
      while(1);
  }
#ifdef DBG
  printf("used pages=%d  but IUsed=%d\n",Pages,IUsed);   
 #endif
   return phy_page;
}
//LRU
/*---------------------------统计页面使用情况------------------------------------------------------*
 *   据LRU算法，此函数在timer_interrupt()中每秒会被调用一次，用于统计页面使用情况。
 *   对于每个在内存中的页如果被访问过，则recent清0,如果没被访问过则recent++
 *   recent 最大的页面就是最久不使用的页面。
 *   注：已经在内存中的页都被链入到了PageUsed链表。
 *-------------------------------------------------------------------------------------------------*/
void CountEveryPage()
{
   if(!InitOk)
       return;
    struct list_elem *e;
    for(e=list_begin(&PageUsed);e!=list_end(&PageUsed);e=list_next(e))
    {
	struct PageCon *pc=list_entry(e,struct PageCon,all_elem);
	struct thread *t=pc->t;
	if(t->pagedir==NULL)
	    continue;
	if(!pagedir_is_accessed(t->pagedir,pc->vir_page)) //检验pc->vir_page这个页是否被访问过。
	    pc->recent++;
	else
	{
	    pagedir_set_accessed(t->pagedir,pc->vir_page,false);
	    pc->recent=0;
	} 

    }
   
}

/*-------------------找出最久未使用的页-----------------------------------------------------------*
 * 淘汰页时调用这个函数找出最久未使用的页。
 * 淘汰页时不能淘汰已经被锁定到内存里的页。
 */
struct PageCon *FindMaxRecent(void)
{
    int recent=0;
    struct PageCon *pc=NULL,*MaxPC=NULL;
    struct list_elem *e;
    int n=0;
    for(e=list_begin(&PageUsed);e!=list_end(&PageUsed);e=list_next(e))
    {
	pc=list_entry(e,struct PageCon,all_elem);
	if(pc->recent>recent&&pc->LockTimes==0)   //如果LockTimes>0则些页已经被锁定到内存，不能换出。
	{
	    recent=pc->recent;
	    MaxPC=pc;
	}
	n++;
    }
  #ifdef DBG
    printf("have %d phy_page in memroy,but ICount=%d\n",n,ICount);
    //printf("max recent is %d\n",recent);
  #endif
    return MaxPC;
}

