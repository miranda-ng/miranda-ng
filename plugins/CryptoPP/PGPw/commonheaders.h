// Windows API

#pragma once
//#define WIN32_LEAN_AND_MEAN
//#pragma warning(disable: 4078)

#include <windows.h>
//#include <afxwin.h>
#include <commdlg.h>
#include "..\version.h"

#pragma comment(linker,"/merge:.rdata=.text")
#pragma comment(linker,"/entry:dllmain")
#pragma comment(linker,"/nodefaultlib")
#pragma comment(linker,"/subsystem:windows")
#pragma optimize("gsy", on)

// PGP API
#ifndef PGP_WIN32
#error Define PGP_WIN32 to SDK version (e.g. 0x658)
#endif

#include <pgpEncode.h>
#include <pgpErrors.h>
#include <pgpKeys.h>
#include <pgpOptionList.h>
#include <pgpUserInterface.h>
#include <pgpUtilities.h>
#include <pgpFeatures.h>


#define MODULENAME "PGPsdk"
BOOL ShowSelectKeyringsDlg(HWND,LPSTR,LPSTR);

extern LPCSTR szModuleName;
extern LPCSTR szVersionStr;
extern HINSTANCE g_hInst;


#define DLLEXPORT __declspec(dllexport)

extern "C" DLLEXPORT int   __cdecl _pgp_init(void);
extern "C" DLLEXPORT int   __cdecl _pgp_done(void);
extern "C" DLLEXPORT int   __cdecl _pgp_open_keyrings(LPSTR,LPSTR);
extern "C" DLLEXPORT int   __cdecl _pgp_close_keyrings(void);
extern "C" DLLEXPORT int   __cdecl _pgp_get_version(void);
extern "C" DLLEXPORT LPSTR __cdecl _pgp_get_error(void);
extern "C" DLLEXPORT int   __cdecl _pgp_size_keyid(void);
extern "C" DLLEXPORT PVOID __cdecl _pgp_select_keyid(HWND,LPSTR);
extern "C" DLLEXPORT LPSTR __cdecl _pgp_encrypt_keydb(LPCSTR,PVOID);
extern "C" DLLEXPORT LPSTR __cdecl _pgp_decrypt_keydb(LPCSTR);
//extern "C" DLLEXPORT int   __cdecl _pgp_check_key(LPCSTR);
extern "C" DLLEXPORT LPSTR __cdecl _pgp_encrypt_key(LPCSTR,LPCSTR);
extern "C" DLLEXPORT LPSTR __cdecl _pgp_decrypt_key(LPCSTR,LPCSTR);

/*
#undef RtlMoveMemory
#undef RtlFillMemory
#undef RtlZeroMemory

NTSYSAPI VOID NTAPI RtlMoveMemory(VOID UNALIGNED *Destination, CONST VOID UNALIGNED *Source, SIZE_T Length);
NTSYSAPI VOID NTAPI RtlFillMemory(VOID UNALIGNED *Destination, SIZE_T Length, BYTE  Fill);
NTSYSAPI VOID NTAPI RtlZeroMemory(VOID UNALIGNED *Destination, SIZE_T Length);
*/
