#include<stdio.h>
#define index0 (12*512)
#define index1 (index0+128*512)
#define index2 (index1+128*128*512)
#define index3 (index2+128*128*128*512)
#define uint16_t unsigned short
#define  uint32_t  unsigned int
struct PosInfo
{
	uint16_t lev;
	uint32_t off;

	uint32_t sn[4];
	uint32_t np[4];
};
int  GetPos(struct PosInfo *pi,off_t off)
{
	if(off<0||off>=index3) return 1;
	if(off>=index2)
	{
		off-=index2;
		pi->lev=3;
		
		pi->sn[0]=0;
		pi->np[0]=14;

		pi->sn[1]=0;
		pi->np[1]=off/(128*128*512);
		off-=pi->np[1]*(128*128*512);

		pi->sn[2]=0;
		pi->np[2]=off/(128*512);
		off-=pi->np[2]*(128*512);

		pi->sn[3]=0;
		pi->np[3]=off/512;
		pi->off=off%512;
	}
	else if(off>=index1)
	{
		off-=index1;
		pi->lev=2;

		pi->sn[0]=0;
		pi->np[0]=13;

		pi->sn[1]=0;
		pi->np[1]=off/(512*128);

		off-=pi->np[1]*(512*128);

		pi->sn[2]=0;
		pi->np[2]=off/512;

		pi->off=off%512;
	}
	else if(off>=index0)
	{
		off-=index0;
		pi->lev=1;

		pi->sn[0]=0;
		pi->np[0]=12;

		pi->sn[1]=0;
		pi->np[1]=off/512;

		pi->off=off%512;
	}
	else
	{
		pi->lev=0;
		pi->sn[0]=0;
		pi->np[0]=off/512;	
		pi->off=off%512;
	}
	return 0;
}
int main()
{
	struct PosInfo pi;
	uint32_t off;
	while(1)
	{
		printf("please input start pos:\n");
		scanf("%d",&off);
		if(GetPos(&pi,off)!=0)
		{
			printf("error pos!\n");
			break;
		}
		printf("lev=%d\n",pi.lev);
		if(pi.lev>=0)
			printf("block index:%d\n",pi.np[0]);
		if(pi.lev>=1)
			printf("first index:%d\n",pi.np[1]);
		if(pi.lev>=2)
			printf("secon index:%d\n",pi.np[2]);
		if(pi.lev>=3)
			printf("third index:%d\n",pi.np[3]);
		printf("off=%d\n",pi.off);
		printf("\n");
	}
		return 0;;
}
