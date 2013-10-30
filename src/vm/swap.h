#ifndef __swap_h__
#define __swap_h__
#include<stdio.h>
#define SlotSize 1025
void InitSwap(void);
int SwapPageAlloc(void);
void SwapPageFree(int nSector);
int SwapOutPage(uint8_t *virpage);
void SwapReadPage(int nSector,uint8_t *virpage);
#endif
