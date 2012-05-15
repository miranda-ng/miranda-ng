#ifndef __SVCS_MENU_H__
#define __SVCS_MENU_H__

#include <windows.h>

INT_PTR __cdecl Service_IsContactSecured(WPARAM,LPARAM);

INT_PTR __cdecl Service_CreateIM(WPARAM,LPARAM);
INT_PTR __cdecl Service_DisableIM(WPARAM,LPARAM);

INT_PTR __cdecl Service_StatusDis(WPARAM,LPARAM);
INT_PTR __cdecl Service_StatusEna(WPARAM,LPARAM);
INT_PTR __cdecl Service_StatusTry(WPARAM,LPARAM);

INT_PTR __cdecl Service_PGPdelKey(WPARAM,LPARAM);
INT_PTR __cdecl Service_PGPsetKey(WPARAM,LPARAM);

INT_PTR __cdecl Service_GPGdelKey(WPARAM,LPARAM);
INT_PTR __cdecl Service_GPGsetKey(WPARAM,LPARAM);

INT_PTR __cdecl Service_ModeNative(WPARAM,LPARAM);
INT_PTR __cdecl Service_ModePGP(WPARAM,LPARAM);
INT_PTR __cdecl Service_ModeGPG(WPARAM,LPARAM);
INT_PTR __cdecl Service_ModeRSAAES(WPARAM,LPARAM);

#endif
