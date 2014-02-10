#ifndef UTILITIES_H
#define UTILITIES_H

HANDLE CreateThreadEx(pThreadFuncEx AFunc, void* arg, DWORD* pThreadID);
TCHAR* __stdcall UniGetContactSettingUtf(MCONTACT hContact, const char *szModule,const char* szSetting, TCHAR* szDef);

#endif
