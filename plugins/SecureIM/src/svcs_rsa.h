#ifndef __SVCS_RSA_H__
#define __SVCS_RSA_H__

extern pRSA_EXPORT mir_exp;
extern RSA_IMPORT imp;
extern BOOL rsa_4096;

int __cdecl rsa_inject(HANDLE, LPCSTR);
int __cdecl rsa_check_pub(HANDLE, uint8_t*, int, uint8_t*, int);
void __cdecl rsa_notify(HANDLE, int);

void sttGenerateRSA(LPVOID);
uint8_t loadRSAkey(pUinKey);
void createRSAcntx(pUinKey);
void resetRSAcntx(pUinKey);
void deleteRSAcntx(pUinKey);

#endif
