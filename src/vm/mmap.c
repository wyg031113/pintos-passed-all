#include"mmap.h"
#include"threads/malloc.h"
#include"frame.h"
#include"page.h"
#include"threads/vaddr.h"
#include"hashfun.h"
static int ID=1;
int IDAlloc(void)
{
  ASSERT(ID>0&&ID<0x7FFFFFFF);
    return ID++;
}
bool WriteBackFile(struct PageCon *pc)
{
    // if(pagedir_get_page(pc->t->pagedir,pc->vir_page)==NULL&&!pagedir_is_dirty(pc->t->pagedir,pc->vir_page))
	 //return true;
     file_seek(pc->FilePtr,pc->offs);
     if(file_write(pc->FilePtr,pc->phy_page,pc->read_bytes)!=pc->read_bytes)
     {
	 printf("Write file back failed\n");
	 return false;
     }

     return true;
}
bool UnMapFile(struct thread *cur,struct MmapNode *mn)
{
    struct PageCon *pc=NULL;
    int i;
    for(i=0;i<mn->nPages;i++)
    {
	enum intr_level old_level=intr_disable();
	pc=page_lookup(&cur->h,mn->vaddr+i*PGSIZE);
	if(pc==NULL)
	    printf("write back file failed\n");
	hash_delete(&cur->h,&pc->has_elem);
	list_remove(&pc->all_elem);
	intr_set_level(old_level);
	if(pc->phy_page!=NULL&&pagedir_get_page(pc->t->pagedir,pc->vir_page)!=NULL&&pagedir_is_dirty(pc->t->pagedir,pc->vir_page))
	{
	      WriteBackFile(pc);
	      pagedir_clear_page(pc->t->pagedir,pc->vir_page);
	      palloc_free_page(pc->phy_page);
	}   
	free(pc);
    }
    list_remove(&mn->elem);
    file_close(mn->FilePtr);
    free(mn);
    return true;
} 
struct MmapNode *GetMapNodeFromID(struct thread *t,int id)
{
     struct list_elem *e;
     for(e=list_begin(&t->MmapFile);e!=list_end(&t->MmapFile);e=list_next(e))
     {
	 struct MmapNode *mn=list_entry(e,struct MmapNode,elem);
	 if(mn->id==id)
	     return mn;
     }
     return NULL;
}
void UnMapAllFile(struct thread *t)
{
    struct MmapNode *mn;
    while(!list_empty(&t->MmapFile))
    {
//	printf("run here\n");
	mn=list_entry(list_pop_front(&t->MmapFile),struct MmapNode,elem);
	UnMapFile(t,mn);
    }
}
