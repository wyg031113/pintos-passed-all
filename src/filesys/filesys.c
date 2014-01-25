#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "filesys/cache.h"
#include "threads/thread.h"
#include "devices/timer.h"
/* Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);
void WriteBackThread(void *aux UNUSED)
{
	while(Inited)
	{
		//printf("WriteBackRun...\n");
		WriteAllBack();
		timer_sleep(500);
	}
//	printf("WriteBackStop...\n");

}
/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  InitCacheMan();
  thread_create("WriteBack",31,WriteBackThread,NULL);
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");
  inode_init ();
  free_map_init ();

  if (format) 
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
	DestroyCacheMan();
  free_map_close ();
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
	to=(char *)malloc(len+5);//！注意这里，没有释放内存
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

struct dir *OpenDir(char *path,int *pos)
{
	struct dir *dir=dir_open_root();
	struct inode *inode=NULL;
	int cur=1,i,n;
	n=strlen(path);
	for(i=1;i<n;i++)
		if(path[i]=='/')
		{
			path[i]=0;
			dir_lookup(dir,path+cur,&inode);
			dir_close(dir);
			if(inode==NULL)
				return NULL;
			if(inode->data.isdir!=1)
				return NULL;
			dir=dir_open(inode);
			inode=NULL;
			cur=i+1;
		}
	*pos=cur;
	return dir;
}
/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size) 
{
  block_sector_t inode_sector = 0;
//  struct dir *dir = dir_open_root ();
 char *path=MakePath(name);
 ASSERT(path!=0);
 int cur;
 struct dir *dir=OpenDir(path,&cur);
  bool success = (dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size)
                  && dir_add (dir,path+cur, inode_sector));
  if (!success && inode_sector != 0) 
    free_map_release (inode_sector, 1);
  dir_close (dir);
 free(path);
  return success;
}


/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name)
{
 // struct dir *dir = dir_open_root ();
 char *path=MakePath(name);
 ASSERT(path!=0);
 int cur;
 struct dir *dir=OpenDir(path,&cur);
  struct inode *inode = NULL;

  if (dir != NULL)
    dir_lookup (dir, path+cur, &inode);
  dir_close (dir);
  free(path);
 // if(inode->data.isdir!=0)
//	  return NULL;
  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name) 
{
 char *path=MakePath(name);
 ASSERT(path!=0);
 int cur;
 struct dir *dir=OpenDir(path,&cur);
  bool success = dir != NULL && dir_remove (dir, path+cur);
  dir_close (dir); 
  free(path);
  return success;
}

/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}
