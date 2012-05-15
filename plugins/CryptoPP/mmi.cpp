#include "commonheaders.h"

/*
void m_check(void *ptr, const char *module) {
    if(ptr==NULL) {
    	char buffer[128];
    	strcpy(buffer,module); strcat(buffer,": NULL pointer detected !");
	MessageBoxA(0,buffer,szModuleName,MB_OK|MB_ICONSTOP);
	__asm{ int 3 };
	exit(1);
    }
}


void *m_alloc(size_t size) {
	void *ptr;
	ptr = malloc(size);
	m_check(ptr,"m_alloc");
   	ZeroMemory(ptr,size);
	return ptr;
}


void m_free(void *ptr) {
//    m_check(ptr,"m_free");
    if(ptr) {
	free(ptr);
    }
}


void *m_realloc(void *ptr,size_t size) {
    r = realloc(ptr,size);
    m_check(ptr,"m_realloc");
	return ptr;
}


#ifndef _DEBUG
void *operator new(size_t size) {
	return malloc(size);
}
#endif


void operator delete(void *p) {
	free(p);
}


void *operator new[](size_t size) {
	return operator new(size);
}


void operator delete[](void *p) {
	operator delete(p);
}


char *m_strdup(const char *str) {
    if(str==NULL) return NULL;
    int len = (int)strlen(str)+1;
	char *dup = (char*) m_alloc(len);
	MoveMemory((void*)dup,(void*)str,len);
	return dup;
}
*/

void __fastcall safe_free(void** p)
{
  if (*p) {
    free(*p);
    *p = NULL;
  }
}


void __fastcall safe_delete(void** p)
{
  if (*p) {
    delete(*p);
    *p = NULL;
  }
}


// EOF
