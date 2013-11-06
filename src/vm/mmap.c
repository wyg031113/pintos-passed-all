#include"mmap.h"
#include"threads/malloc.h"
#include"frame.h"
#include"page.h"
static int ID=1;
int IDAlloc(void)
{
  ASSERT(ID>0&&ID<0x7FFFFFFF);
    return ID++;
}
bool WriteBackFile(struct MmapNode *mn,struct PageCon *pc)
{
     if(pagedir_get_page(pc->t->pagedir,pc->vir_page)==NULL&&!pagedir_is_dirty(pc->t->pagedir,pc->vir_page))
	 return true;
     file_seek(mn->FilePtr,pc->offs);
     if(file_write(mn->FilePtr,pc->vir_page,pc->read_bytes)!=pc->read_bytes)
     {
	 printf("Write file back failed\n");
	 return false;
     }

     return true;
}
