#include"swap.h"
#include"filesys/directory.h"
//==============================================================
//use swap

int swapslot[SlotSize];
int SwapHead;
struct block *SwapPar;
void InitSwap(void)
{
//    list_init(&SwapPage);
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
    int nPage=SwapHead;
    SwapHead=swapslot[SwapHead];
    int nSector=8*nPage;
    return nSector;
}


//=================================================
//page recycle 
void SwapPageFree(int nSector)
{
    if(nSector==-1)
	return; 
    ASSERT(nSector%8==0);
    int nSlot=nSector/8;
    swapslot[nSlot]=SwapHead;
    SwapHead=nSlot;
}

int SwapOutPage(uint8_t *virpage)
{
    ASSERT(SwapPar!=NULL);
    int nPage=SwapPageAlloc();
    if(nPage==-1)
	return -1;
    int i;
    for(i=0;i<8;i++)
       block_write(SwapPar,nPage+i,virpage+i*512);
    return nPage;
}
void SwapReadPage(int nSector,uint8_t *virpage)
{
    //printf("nSector=%d\n",nSector);
    ASSERT(SwapPar!=NULL);
    int i;
    for(i=0;i<8;i++) 
        block_read(SwapPar,nSector+i,virpage+i*512);
}
