#ifndef __DLLLOADER_H__
#define __DLLLOADER__H__

HMODULE MemLoadLibrary(PBYTE data);
BOOL MemFreeLibrary(HMODULE hDll);
FARPROC MemGetProcAddress(HMODULE hDll, LPCTSTR fname);

#endif
