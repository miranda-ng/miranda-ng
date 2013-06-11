// Windows API

#pragma once

#include "..\version.h"

// PGP API
#ifndef PGP_WIN32
#error Define PGP_WIN32 to SDK version (e.g. 0x658)
#endif

#undef MODULENAME
#define MODULENAME "PGPsdk"

BOOL ShowSelectKeyringsDlg(HWND,LPSTR,LPSTR);

extern LPCSTR szModuleName;
extern LPCSTR szVersionStr;
extern HINSTANCE g_hInst;

extern "C"  int   __cdecl _pgp_init(void);
extern "C"  int   __cdecl _pgp_done(void);
extern "C"  int   __cdecl _pgp_open_keyrings(LPSTR,LPSTR);
extern "C"  int   __cdecl _pgp_close_keyrings(void);
extern "C"  int   __cdecl _pgp_get_version(void);
extern "C"  LPSTR __cdecl _pgp_get_error(void);
extern "C"  int   __cdecl _pgp_size_keyid(void);
extern "C"  PVOID __cdecl _pgp_select_keyid(HWND,LPSTR);
extern "C"  LPSTR __cdecl _pgp_encrypt_keydb(LPCSTR,PVOID);
extern "C"  LPSTR __cdecl _pgp_decrypt_keydb(LPCSTR);
extern "C"  int   __cdecl _pgp_check_key(LPCSTR);
extern "C"  LPSTR __cdecl _pgp_encrypt_key(LPCSTR,LPCSTR);
extern "C"  LPSTR __cdecl _pgp_decrypt_key(LPCSTR,LPCSTR);
