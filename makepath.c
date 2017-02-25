#include<stdio.h>
#include<string.h>
#include<malloc.h>
char pwd[100]="/";
const char *GetPwd(void)
{
	return pwd;
}
void xstrcpy(char *to,const char *from)
{
	while(*to++=*from++);
}
char * MakePath(const char *from)
{
	//ASSERT(to!=NULL);
	const char *pwd=GetPwd();
	char *to;
	int lf=strlen(from);
	int lp=strlen(pwd);
	int len=lf+lp;
	int pos=lp-1;
	to=(char *)malloc(len+1);
	if(to==NULL)
	{
		printf("error\n");
		return 0;
	}
	xstrcpy(to,pwd);	
	if(to[lp-1]!='/')
	{
		to[lp]='/';
		to[lp+1]=0;
	}
	if(from[0]=='/')
	{
		//xstrcpy(to,from);
		//return to;
		pos=0;
	}

	char pre=0;
	int i;
	for(i=0;i<lf;i++)
	{

		if(from[i]=='.'&&pre=='.')
		{
			if(pos>0)pos--;
			while(pos>0&&to[pos]!='/')
				pos--;
		}		
		else if(from[i]=='/')
		{
			if(to[pos]!='/')
				to[++pos]=from[i];
		}
		else if(from[i]=='.'&&pre==0);
			 
		else if(pre=='/'&&from[i]=='.');
		else
		to[++pos]=from[i];
		pre=from[i];
	}	
	to[++pos]=0;
	return to;
	
}
int main()
{
	char ph1[50]="home/os/pintos/file.c";
	scanf("%s",ph1);
	char *ans=NULL;
	ans=MakePath(ph1);
	printf("%s\n",ans);
	free(ans);
	return 0;
}
