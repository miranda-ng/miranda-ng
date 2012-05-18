/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

#define MIRANDA_VER 0x0A00

#define _WIN32_WINNT 0x0501
#include "m_stdhdr.h"

//windows headers

#include <windows.h>

#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <io.h>
#include <string.h>
#include <direct.h>
#include <crtdbg.h>

//miranda headers
#include <newpluginapi.h>
#include <win2k.h>
#include <m_plugins.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_options.h>

//non-official miranda-plugins sdk
#include "m_folders.h"

//own headers
#include "database.h"
#include "resource.h"
#include "version.h"

extern PLUGINLINK *pluginLink;

extern struct MM_INTERFACE memoryManagerInterface;
extern struct LIST_INTERFACE li;

extern CRITICAL_SECTION csDbAccess;
extern struct DBHeader dbHeader;
extern HANDLE hDbFile;
extern BOOL bEncoding;

#ifdef __GNUC__
#define mir_i64(x) (x##LL)
#else
#define mir_i64(x) (x##i64)
#endif

//global procedures
//int InitSkin();
void EncodeCopyMemory(void * dst, void * src, size_t size );
void DecodeCopyMemory(void * dst, void * src, size_t size );
void EncodeDBWrite(DWORD ofs, void * src, size_t size);
void DecodeDBWrite(DWORD ofs, void * src, size_t size);
int CheckPassword(WORD checkWord, char * szDBName);

void EncryptDB();
void DecryptDB();
void RecryptDB();
void ChangePwd();

void EncodeEvent(HANDLE hEvent);
void EncodeContactEvents(HANDLE hContact);
void DecodeEvent(HANDLE hEvent);
void DecodeContactEvents(HANDLE hContact);
void EncodeContactSettings(HANDLE hContact);
void DecodeContactSettings(HANDLE hContact);

BOOL CALLBACK DlgStdInProc(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK DlgStdNewPass(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK DlgChangePass(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam);
void xModifyMenu(HANDLE hMenu,long flags,const TCHAR* name, HICON hIcon);

#define NEWSTR_ALLOCA(A) (A==NULL)?NULL:strcpy((char*)alloca(strlen(A)+1),A)

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
	char dllname[255];
	HMODULE hLib;
	Cryptor* cryptor;
} CryptoModule;