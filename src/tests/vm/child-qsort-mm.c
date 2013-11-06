/* Mmaps a 128 kB file "sorts" the bytes in it, using quick sort,
   a multi-pass divide and conquer algorithm.  */

#include <debug.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"
#include "tests/vm/qsort.h"
#include<stdio.h>
const char *test_name = "child-qsort-mm";

int
main (int argc UNUSED, char *argv[]) 
{
  int handle;
  unsigned char *p = (unsigned char *) 0x10000000;

  quiet = true;

  CHECK ((handle = open (argv[1])) > 1, "open \"%s\"", argv[1]);
  CHECK (mmap (handle, p) != MAP_FAILED, "mmap \"%s\"", argv[1]);
  qsort_bytes (p, 1024 * 128);
//  write(handle,p,1024*128);
// int i;
//for(i=0;i<1024*128;i++)
  //  printf("%x ",p[i]);
//printf("\n\n");
  return 80;
}
