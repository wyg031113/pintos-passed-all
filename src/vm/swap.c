#include"swap.h"
#include"filesys/directory.h"
#include"threads/interrupt.h"
#include"threads/synch.h"
//==============================================================
//use swap

int swapslot[SlotSize];
int SwapHead;
struct block *SwapPar;
struct semaphore RWSema;
void InitSwap(void)
{
//    list_init(&SwapPage);
    sema_init(&RWSema,1);
    SwapPar=block_get_role(BLOCK_SWAP);
    if(SwapPar==NULL)
    {
	printf("get swap partition failed!\n");
	return;
    }
    int i=0;
    SwapHead=0;
    for(i=0;i<SlotSize-1;i++)
	swapslot[i]=i+1;
    swapslot[SlotSize-1]=-1;
}

//=============================================
//Alloc a page 4 KB in swap disk  return the start sector number.
int SwapPageAlloc(void)
{
    if(SwapHead==-1)
	return -1;
    enum intr_level old_level=intr_disable();
    int nPage=SwapHead;
    SwapHead=swapslot[SwapHead];
    int nSector=8*nPage;
    intr_set_level(old_level);
    return nSector;
}


//=================================================
//page recycle 
void SwapPageFree(int nSector)
{
    if(nSector==-1)
	return; 
    ASSERT(nSector%8==0);
    enum intr_level old_level=intr_disable();
    int nSlot=nSector/8;
    swapslot[nSlot]=SwapHead;
    SwapHead=nSlot;
    intr_set_level(old_level);
}

int SwapOutPage(uint8_t *phy_page)
{
    ASSERT(SwapPar!=NULL);
    int nPage=SwapPageAlloc();
    if(nPage==-1)
	return -1;
    int i;
    //sema_down(&RWSema);
    for(i=0;i<8;i++)
       block_write(SwapPar,nPage+i,phy_page+i*512);
    //sema_up(&RWSema);
    return nPage;
}
void SwapReadPage(int nSector,uint8_t *phy_page)
{
    //printf("nSector=%d\n",nSector);
    ASSERT(SwapPar!=NULL);
    int i;
   // enum intr_level old_level=intr_disable();
   // sema_down(&RWSema);
    for(i=0;i<8;i++) 
        block_read(SwapPar,nSector+i,phy_page+i*512);
   // sema_up(&RWSema);
   // intr_set_level(old_level);
}

