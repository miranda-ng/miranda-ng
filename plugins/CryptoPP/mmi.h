#ifndef __MMI_H__
#define __MMI_H__
/*
void *m_alloc(size_t);
void m_free(void *);
void *m_realloc(void *,size_t);
char *m_strdup(const char *);

void *operator new(size_t sz);
void operator delete(void *p);
void *operator new[](size_t size);
void operator delete[](void * p);
*/

#define SAFE_INIT(t,p)			t p=NULL;
#define SAFE_FREE(p)			safe_free((void **)&(p));
#define SAFE_DELETE(p)			safe_delete((void **)&(p));

void __fastcall safe_free(void** p);
void __fastcall safe_delete(void** p);

#endif
