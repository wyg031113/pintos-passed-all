#include"hashfun.h"
#include<stdio.h>
#include<hash.h>
#include"frame.h"
#include"threads/synch.h"
#include"page.h"
#include"threads/interrupt.h"
//#include"threads/malloc.h"
unsigned page_hash(const struct hash_elem *p_,void *aux)
{
    const struct PageCon *p = hash_entry(p_,struct PageCon,has_elem);
    return hash_bytes(&p->vir_page,sizeof p->vir_page);
}
bool page_less(const struct hash_elem *a_, const struct hash_elem *b_,void *aux UNUSED)
{
    const struct PageCon *a=hash_entry(a_,struct PageCon,has_elem);
    const struct PageCon *b=hash_entry(b_,struct PageCon,has_elem);
    return a->vir_page < b->vir_page;
}
struct PageCon *page_lookup(const struct hash *h,const void *vir_page)
{
    struct PageCon p;
    struct hash_elem *e;
    p.vir_page=vir_page;
    e=hash_find(h,&p.has_elem);
    return e!=NULL?hash_entry(e,struct PageCon,has_elem):NULL;
}
void destroy(struct hash_elem *e,void *aux)
{

    struct PageCon *pc=hash_entry(e,struct PageCon,has_elem);
    //enum intr_level old_level=intr_disable();
    list_remove(&pc->all_elem);
   // intr_set_level(old_level);

   free(pc);

}
