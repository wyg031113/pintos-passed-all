#ifndef __HASHFUN_H__
#define __HASHFUN_H__
#include<stdio.h>
#include<hash.h>
unsigned page_hash(const struct hash_elem *p_,void *aux UNUSED);
bool page_less(const struct hash_elem *a_, const struct hash_elem *b_,void *aux UNUSED);
struct PageCon *page_lookup(const struct hash *h,const void *vir_page);
void destroy(struct hash_elem *e,void *aux);
#endif
