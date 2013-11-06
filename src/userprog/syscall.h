#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "threads/thread.h"
#include "threads/interrupt.h"
int CloseFile(struct thread *t,int fd,int bAll);
void syscall_init (void);
void IWrite(struct intr_frame*);
void IExit(struct intr_frame *f);
void ExitStatus(int status);
void ICreate(struct intr_frame *f);
void IOpen(struct intr_frame *f);
void IClose(struct intr_frame *f);
void IRead(struct intr_frame *f);
void IFileSize(struct intr_frame *f);
void IExec(struct intr_frame *f);
void IWait(struct intr_frame *f);
void ISeek(struct intr_frame *f);
void IRemove(struct intr_frame *f);
void ITell(struct intr_frame *f);
void IHalt(struct intr_frame *f);
void IMmap(struct intr_frame *f);
void IMunmap(struct intr_frame *f);
struct file_node *GetFile(struct thread *t,int fd);
#endif /* userprog/syscall.h */
