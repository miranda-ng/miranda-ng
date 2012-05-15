#ifndef __RTFCONV_H__
#define __RTFCONV_H__

#include <windows.h>

#ifndef _INTPTR_T_DEFINED
#define intptr_t int
#endif

#define CONVMODE_USE_SYSTEM_TABLE         0x800000    /* Use OS's table only */

typedef intptr_t (WINAPI *RTFCONVSTRING) ( const void *pSrcBuffer, void *pDstBuffer,
    int nSrcCodePage, int nDstCodePage, unsigned long dwFlags,
    size_t nMaxLen );

extern RTFCONVSTRING pRtfconvString;

BOOL load_rtfconv ();
void free_rtfconv ();
void rtfconvA(LPCSTR rtf, LPWSTR plain);
void rtfconvW(LPCWSTR rtf, LPWSTR plain);

#endif
