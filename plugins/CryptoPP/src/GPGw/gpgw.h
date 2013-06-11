// Windows API

#pragma once

#include "..\version.h"

// gnupg plugin
#include "gpg_res.h"
#include "size.h"
#include "language.h"
#include "pipeexec.h"
#include "gpg.h"
#include "tools.h"
#include "passphrases.h"
#include "passdialog.h"
#include "userdialog.h"
#include "keys.h"

// passphrase typ
struct passphrase_t
{
	char keyuserid[keyuseridsize];
	char passphrase[passphrasesize];
};

extern struct passphrase_t *passphrases;
extern int passphrasecount;

extern char temporarydirectory[fullfilenamesize];
extern char logfile[fullfilenamesize];

extern char *txtbeginpgppublickeyblock;
extern char *txtendpgppublickeyblock;
extern char *txtbeginpgpmessage;
extern char *txtendpgpmessage;

#undef  MODULENAME
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
