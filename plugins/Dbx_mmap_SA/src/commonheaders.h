/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define _CRT_SECURE_NO_WARNINGS

#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <shlobj.h>
#include <time.h>
#include <process.h>
#include <memory>

#include <newpluginapi.h>
#include <win2k.h>
#include <m_system_cpp.h>
#include <m_database.h>
#include <m_genmenu.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_crypto.h>

#include <m_folders.h>

//own headers
#include "dbintf_sa.h"
#include "..\Db3x_mmap\src\database.h"
#include "resource.h"
#include "version.h"

extern HINSTANCE g_hInst;
extern HGENMENU hSetPwdMenu;

#ifdef __GNUC__
#define mir_i64(x) (x##LL)
#else
#define mir_i64(x) (x##i64)
#endif

//global procedures

struct DlgStdInProcParam
{
	CDbxMmapSA *p_Db;
	const TCHAR *pStr;
};
INT_PTR CALLBACK DlgStdInProc(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam);

struct DlgChangePassParam
{
	CDbxMmapSA *p_Db;
	char *pszNewPass;
};
INT_PTR CALLBACK DlgChangePass(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam);

INT_PTR CALLBACK DlgStdNewPass(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void xModifyMenu(HGENMENU hMenu,long flags,const TCHAR* name, HICON hIcon);

extern DBSignature dbSignatureSecured, dbSignatureNonSecured;
extern DBSignature dbSignatureU, dbSignatureE, dbSignatureIM, dbSignatureSA;

extern LIST<CDbxMmapSA> g_Dbs;

typedef struct{
	void* (__stdcall *GenerateKey)(char* pwd);
	void (__stdcall *FreeKey)(void* key);
	void (__stdcall *EncryptMem)(BYTE* data, int size, void* key);
	void (__stdcall *DecryptMem)(BYTE* data, int size, void* key);

    char* Name;
    char* Info;
    char* Author;
    char* Site;
    char* Email;

	DWORD Version;

	WORD uid;
} Cryptor;

typedef struct{
	TCHAR dllname[MAX_PATH];
	HMODULE hLib;
	Cryptor* cryptor;
} CryptoModule;

extern Cryptor* CryptoEngine;
extern void* key;
