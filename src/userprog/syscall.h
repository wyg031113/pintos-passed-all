#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "../threads/thread.h"
int CloseFile(struct thread *t,int fd,int bAll);
void syscall_init (void);

#endif /* userprog/syscall.h */
