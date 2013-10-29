#include"frame.h"
#include"threads/palloc.h"
#include<list.h>
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
