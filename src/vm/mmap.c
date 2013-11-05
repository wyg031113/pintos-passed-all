#include"mmap.h"
#include"threads/malloc.h"
static int ID=1;
int IDAlloc(void)
{
  ASSERT(ID>0<0x7FFFFFFF);
    return ID++;
}

