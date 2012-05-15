#ifndef __SVCS_RSA_H__
#define __SVCS_RSA_H__

#include <windows.h>

extern pRSA_EXPORT exp;
extern RSA_IMPORT imp;
extern BOOL rsa_4096;

int __cdecl rsa_inject(HANDLE,LPCSTR);
int __cdecl rsa_check_pub(HANDLE,PBYTE,int,PBYTE,int);
void __cdecl rsa_notify(HANDLE,int);

unsigned __stdcall sttGenerateRSA(LPVOID);
BYTE loadRSAkey(pUinKey);
void createRSAcntx(pUinKey);
void resetRSAcntx(pUinKey);
void deleteRSAcntx(pUinKey);

#endif
