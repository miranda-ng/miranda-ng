// Windows API

#define WIN32_LEAN_AND_MEAN
#define NETLIB_LOG

#ifdef _MSC_VER
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#if _MSC_VER >= 1300
//#pragma comment (compiler,"/GS-")
//#pragma comment (linker,"/NODEFAULTLIB:libcmt.lib") 
//#pragma comment (lib,"../../lib/msvcrt.lib")
//#pragma comment (lib,"../../lib/msvcrt71.lib")
#else
#ifndef _DEBUG
#pragma optimize("gsy", on)
#endif
#endif 
#endif 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif

#define MIRANDA_VER 0x0700
#include <m_stdhdr.h>

#if _MSC_VER > 1000
#include <windows.h>
#else
#include <afxwin.h>
#endif
#include <process.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#pragma comment(linker,"/filealign:16")
//#pragma comment(linker,"/align:16")
//#pragma optimize("gsy", on)
#ifdef _DEBUG
#if _MSC_VER >= 1300
#pragma comment(lib,"crypto/Debug9/cryptlib.lib")
#else
#pragma comment(lib,"crypto/Debug/cryptlib.lib")
#endif
#else
#if _MSC_VER >= 1300
#pragma comment(lib,"crypto/Release9/cryptlib.lib")
#else
#pragma comment(lib,"crypto/Release/cryptlib.lib")
#endif
#endif
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"user32.lib")

#ifndef M_API_H__
#define M_API_H__

// Miranda API
#include "newpluginapi.h"
#include "m_stdhdr.h"
#include "m_plugins.h"
#include "m_system.h"
#include "m_database.h"
#include "m_protomod.h"
#include "m_protosvc.h"
#include "m_utils.h"
#include "m_netlib.h"
#include "sdk/m_updater.h"

#endif

#include "cryptopp.h"
#include "version.h"
#include "resource.h"
#include "dllloader.h"
#include "mmi.h"
#include "utf8.h"
#include "base16.h"
#include "base64.h"
#include "gettime.h"
#include "cpp_rsam.h"
#include "cpp_rsau.h"

#define MODULENAME "Crypto++"

extern LPCSTR szModuleName;
extern LPCSTR szVersionStr;
extern char TEMP[MAX_PATH];
extern int  TEMP_SIZE;
extern BOOL isVista;

// shared vars
extern HINSTANCE g_hInst;
extern PLUGINLINK *pluginLink;
extern PLUGININFO pluginInfo;
extern PLUGININFOEX pluginInfoEx;
//extern MM_INTERFACE mmi;
extern MUUID interfaces[];

extern HANDLE hPGPPRIV;
extern HANDLE hRSA4096;

extern CRITICAL_SECTION localQueueMutex;
extern CRITICAL_SECTION localContextMutex;

void ExtractFile(char*,int,int);
int rtrim(LPCSTR);

#if defined(_DEBUG) || defined(NETLIB_LOG)
extern HANDLE hNetlibUser;
void InitNetlib();
void DeinitNetlib();
int Sent_NetLog(const char *,...);
#endif

#define MIID_CRYPTOPP	{0x3613F2D9, 0xC040, 0x4361, { 0xA4, 0x4F, 0xDF, 0x7B, 0x5A, 0xAA, 0xCF, 0x6E }} //3613F2D9-C040-4361-A44F-DF7B5AAACF6E

#define DLLEXPORT __declspec(dllexport)

PBYTE cpp_alloc_pdata(pCNTX);

extern "C" {

 DLLEXPORT int Load(PLUGINLINK *);
 DLLEXPORT PLUGININFO *MirandaPluginInfo(DWORD);
 DLLEXPORT PLUGININFOEX *MirandaPluginInfoEx(DWORD);
 DLLEXPORT MUUID* MirandaPluginInterfaces(void);
 DLLEXPORT int Unload();

 DLLEXPORT HANDLE __cdecl cpp_create_context(int); // create crypt-context
 DLLEXPORT void   __cdecl cpp_delete_context(HANDLE); // delete crypt-context
 DLLEXPORT void   __cdecl cpp_reset_context(HANDLE);  // reset crypt-context (free all data)
 DLLEXPORT LPSTR  __cdecl cpp_init_keya(HANDLE,int);    // make KeyA
 DLLEXPORT int    __cdecl cpp_init_keyb(HANDLE,LPCSTR); // load KeyB
 DLLEXPORT int    __cdecl cpp_calc_keyx(HANDLE);        // calculate KeyX
 DLLEXPORT int    __cdecl cpp_init_keyp(HANDLE,LPCSTR); // make KeyP from password
 DLLEXPORT LPSTR  __cdecl cpp_encodeA(HANDLE,LPCSTR); // encode ANSIz string
 DLLEXPORT LPSTR  __cdecl cpp_encodeW(HANDLE,LPWSTR); // encode USC2z string
 DLLEXPORT LPSTR  __cdecl cpp_encodeU(HANDLE,LPCSTR); // encode UTF8z string
 DLLEXPORT LPSTR  __cdecl cpp_decode(HANDLE,LPCSTR);  // decode as ANSIzUCS2z
 DLLEXPORT LPSTR  __cdecl cpp_decodeU(HANDLE,LPCSTR); // decode as UTF8z
 DLLEXPORT int    __cdecl cpp_encrypt_file(HANDLE,LPCSTR,LPCSTR); // encrypt file
 DLLEXPORT int    __cdecl cpp_decrypt_file(HANDLE,LPCSTR,LPCSTR); // decrypt file
 DLLEXPORT int    __cdecl cpp_get_features(HANDLE); // get features field from client
 DLLEXPORT int    __cdecl cpp_get_error(HANDLE);    // get last error code
 DLLEXPORT int    __cdecl cpp_get_version(void); // get dll version
 DLLEXPORT int    __cdecl cpp_size_keyx(void);
 DLLEXPORT void   __cdecl cpp_get_keyx(HANDLE,byte*); // get crypto key
 DLLEXPORT void   __cdecl cpp_set_keyx(HANDLE,byte*); // set crypto key
 DLLEXPORT int    __cdecl cpp_size_keyp(void);
 DLLEXPORT void   __cdecl cpp_get_keyp(HANDLE,byte*); // get pre-shared key
 DLLEXPORT void   __cdecl cpp_set_keyp(HANDLE,byte*); // set pre-shared key
 DLLEXPORT int    __cdecl cpp_keya(HANDLE); // KeyA exist ?
 DLLEXPORT int    __cdecl cpp_keyb(HANDLE); // KeyB exist ?
 DLLEXPORT int    __cdecl cpp_keyx(HANDLE); // KeyX exist ?
 DLLEXPORT int    __cdecl cpp_keyp(HANDLE); // KeyP exist ?

 DLLEXPORT int    __cdecl pgp_init(void);
 DLLEXPORT int    __cdecl pgp_done(void);
 DLLEXPORT int    __cdecl pgp_open_keyrings(LPSTR,LPSTR);
 DLLEXPORT int    __cdecl pgp_close_keyrings(void);
 DLLEXPORT int    __cdecl pgp_get_version(void);
 DLLEXPORT LPSTR  __cdecl pgp_get_error(void);
 DLLEXPORT int    __cdecl pgp_set_priv_key(LPCSTR);
 DLLEXPORT int    __cdecl pgp_set_key(HANDLE,LPCSTR);
 DLLEXPORT int    __cdecl pgp_set_keyid(HANDLE,PVOID);
 DLLEXPORT int    __cdecl pgp_size_keyid(void);
 DLLEXPORT PVOID  __cdecl pgp_select_keyid(HWND,LPSTR);
 DLLEXPORT LPSTR  __cdecl pgp_encode(HANDLE,LPCSTR);
 DLLEXPORT LPSTR  __cdecl pgp_decode(HANDLE,LPCSTR);

 DLLEXPORT int    __cdecl gpg_init(void);
 DLLEXPORT int    __cdecl gpg_done(void);
 DLLEXPORT int    __cdecl gpg_open_keyrings(LPSTR,LPSTR);
 DLLEXPORT int    __cdecl gpg_close_keyrings(void);
 DLLEXPORT void   __cdecl gpg_set_log(LPCSTR);
 DLLEXPORT void   __cdecl gpg_set_tmp(LPCSTR);
 DLLEXPORT LPSTR  __cdecl gpg_get_error(void);
 DLLEXPORT int    __cdecl gpg_set_key(HANDLE,LPCSTR);
 DLLEXPORT int    __cdecl gpg_set_keyid(HANDLE,LPCSTR);
 DLLEXPORT int    __cdecl gpg_size_keyid(void);
 DLLEXPORT int    __cdecl gpg_select_keyid(HWND,LPSTR);
 DLLEXPORT LPSTR  __cdecl gpg_encode(HANDLE,LPCSTR);
 DLLEXPORT LPSTR  __cdecl gpg_decode(HANDLE,LPCSTR);
 DLLEXPORT LPSTR  __cdecl gpg_get_passphrases();
 DLLEXPORT void   __cdecl gpg_set_passphrases(LPCSTR);

 DLLEXPORT int    __cdecl rsa_init(pRSA_EXPORT*,pRSA_IMPORT);
 DLLEXPORT int    __cdecl rsa_done(void);
}

// EOF
