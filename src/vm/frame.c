#include"frame.h"
#include"threads/palloc.h"
#include<list.h>
#include<hash.h>
#include"userprog/pagedir.h"
struct list AllPage;
void InitPageCon(struct PageCon *pc)
{
    pc->vir_page=NULL;
    pc->phy_page=NULL;
    pc->offs=-1;
    pc->writable=true;
    pc->recent=0;
    pc->is_code=0;
}
void *PageAlloc(enum palloc_flags flags)
{
   void * phy_page=palloc_get_page(flags);
   return phy_page;
}
//LRU
void CountRecent(struct hash_elem *e,void *aux)
{
    struct thread *t=(struct thread *)aux;
    if(t->pagedir==NULL)
	return;
    struct PageCon *pc=hash_entry(e,struct PageCon,has_elem);
    if(!pagedir_is_accessed(t->pagedir,pc->vir_page))
	pc->recent++;
    else
	pagedir_set_accessed(t->pagedir,pc->vir_page,false);

}
