#include "commonheaders.h"

#define SAFE_PTR(a) a?(IsBadReadPtr(a,1)?a=NULL:a):a

int __cdecl MyStrCmp (const char *a, const char *b)
{
	SAFE_PTR(a);
	SAFE_PTR(b);
	if (!(a&&b)) return a!=b;
	return (strcmp(a,b));
}

DWORD exceptFunction(LPEXCEPTION_POINTERS EP)
{
    //printf("1 ");                     // printed first
	char buf[4096];


	sprintf(buf,"\r\nExceptCode: %x\r\nExceptFlags: %x\r\nExceptAddress: %p\r\n",
		EP->ExceptionRecord->ExceptionCode,
		EP->ExceptionRecord->ExceptionFlags,
		EP->ExceptionRecord->ExceptionAddress
		);
	OutputDebugStringA(buf);
	MessageBoxA(0,buf,"clist_mw Exception",0);


	return EXCEPTION_EXECUTE_HANDLER;
}
