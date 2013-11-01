#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "malloc.h"/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB pageŔ.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */

#include "../filesys/file.h"
#include"synch.h"
#include<hash.h>
#define MAX_SEMA   40                  /* numbers of sema that a thread can get */
#define MAX_DEEP   40                 /* Max deep of nest priority-donate */
#define MaxSons 20
//above two can't too big  Maybe we don't have enough memory, mlfqs-load-60 will failed
extern int load_avg;
void PrintThread(struct thread *t,void *aux);
void list_sort_ready(void);
struct GetedSema                        /*thread geted sema*/
{
   struct semaphore *s;
   int n;
};
struct PriStore
{
    int pri;
    struct semaphore *s;
};

struct file_node
{
    int fd;
    struct list_elem elem;
    struct file *f;
};
struct ret_data
{
    int tid;
    int ret;
    struct list_elem elem;
};
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */

	int64_t block_ticks;
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

    int nice;                           /*for advance schedualer*/
    int recent_cpu;                     /*recent cpu time*/
    struct thread *father;
    int sons;                           //numbers of sons
    struct list sons_ret;
    //struct ret_data sonret[MaxSons];   //record sons return value

    int SonTop;                        //for ret_data_sonret
    bool SaveData;                    //whether save data to father's array
    bool bWait;
    int FileNum;                      //nums of open files
    struct hash h;
    bool IsUser;
    void *esp;
#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
    int ret;                            /* save self exit code */
    struct list file_list;              //open file list
    int maxfd;                         //for alloc file handle
    struct semaphore SemaWait,SemaWaitSuccess;
    struct file *FileSelf;            //open self deny write
    //bool bSuccess;
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);
void block_check(struct thread *t,void *aux);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);


/*list compare priority*/
bool less_priority(const struct list_elem *a,
				   const struct list_elem *b,
				   void *aux);

int thread_get_priority (void);
void thread_set_priority (int);
void change_priority(struct thread*,int,struct semaphore *);  //A new priority seter for priority-donate
int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);
void change_recent_cpu(struct thread *t,void *aux);
void Get_ready_threads(struct thread*t,void *aux);
void calc_priority(struct thread *t,void *aux);
struct thread *GetThreadFromTid(tid_t tid);
#endif /* threads/thread.h */
