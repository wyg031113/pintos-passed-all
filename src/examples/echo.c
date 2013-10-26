#include <stdio.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
	int i;
	for(i=0;i<126;i++)
		if(open(argv[0])==-1)
			break;
/*  create("wygOk",0x123);
  int i;
  printf("Hello,world\n");
  for (i = 0; i < argc; i++)
    printf ("%s ", argv[i]);
  printf ("\n");*/
return i;
//  return EXIT_SUCCESS;
}
