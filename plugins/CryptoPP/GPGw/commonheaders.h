// Windows API

#pragma once
#define _WIN32_IE 0x0500
//#define WIN32_LEAN_AND_MEAN
//#pragma warning(disable: 4078)

#include <windows.h>
//#include <afxwin.h>
#include <commdlg.h>
#include <commctrl.h>
#include <stdio.h>
#include "..\version.h"

// gnupg plugin
#include "resource.h"
#include "size.h"
#include "language.h"
#include "pipeexec.h"
#include "gpg.h"
#include "tools.h"
#include "passphrases.h"
#include "passdialog.h"
#include "userdialog.h"
#include "keys.h"

// debug makro
#define debugout(mtext) MessageBox(NULL, (mtext), "GnuPG Plugin - Debug", MB_OK)

// passphrase typ
struct passphrase
{
  char keyuserid[keyuseridsize];
  char passphrase[passphrasesize];
};

extern struct passphrase *passphrases;
extern int passphrasecount;

extern char temporarydirectory[fullfilenamesize];
extern char logfile[fullfilenamesize];

extern char *txtbeginpgppublickeyblock;
extern char *txtendpgppublickeyblock;
extern char *txtbeginpgpmessage;
extern char *txtendpgpmessage;

#ifdef _MSC_VER
//#pragma comment(linker,"/merge:.rdata=.text")
#pragma comment(linker,"/entry:dllmain")
#pragma comment(linker,"/nodefaultlib")
#pragma comment(linker,"/subsystem:windows")
#pragma optimize("gsy", on)
#endif

#define MODULENAME "GnuPG"

extern LPCSTR szModuleName;
extern LPCSTR szVersionStr;
extern HINSTANCE g_hInst;

#define DLLEXPORT __declspec(dllexport)

extern DLLEXPORT int   __cdecl _gpg_init(void);
extern DLLEXPORT int   __cdecl _gpg_done(void);
extern DLLEXPORT int   __cdecl _gpg_open_keyrings(LPSTR,LPSTR);
extern DLLEXPORT int   __cdecl _gpg_close_keyrings(void);
extern DLLEXPORT void  __cdecl _gpg_set_log(LPCSTR);
extern DLLEXPORT void  __cdecl _gpg_set_tmp(LPCSTR);
extern DLLEXPORT LPSTR __cdecl _gpg_get_error(void);
extern DLLEXPORT int   __cdecl _gpg_size_keyid(void);
extern DLLEXPORT int   __cdecl _gpg_select_keyid(HWND,LPSTR);
extern DLLEXPORT LPSTR __cdecl _gpg_encrypt(LPCSTR,LPCSTR);
extern DLLEXPORT LPSTR __cdecl _gpg_decrypt(LPCSTR);
extern DLLEXPORT LPSTR __cdecl _gpg_get_passphrases();
extern DLLEXPORT void  __cdecl _gpg_set_passphrases(LPCSTR);

void __cdecl ErrorMessage(const char *alevel, const char *atext, const char *ahint);
void __cdecl LogMessage(const char *astart, const char *atext, const char *aend);
