/*
Miranda Database Tool
Copyright 2000-2011 Miranda ICQ/IM project, 
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

#if defined( UNICODE ) && !defined( _UNICODE )
	#define _UNICODE
#endif

#include <tchar.h>

#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <io.h>
#include <stdarg.h>
#include <process.h>
#include <direct.h>
#include <malloc.h>
#include <commctrl.h>
#include <time.h>

//#include <newpluginapi.h> // Only needed to keep m_database.h happy
#define CallService(a,b,c) 1

#include "../../miranda/include/m_database.h"
#include "database.h" // Note: This is a copy of database.h from the Miranda IM v0.3 tree.
                      //       Remember to update this when releasing new dbtool versions.
#include "resource.h"


#define WinVerMajor()      LOBYTE(LOWORD(GetVersion()))
#define IsWinVerXPPlus()   (WinVerMajor()>=5 && LOWORD(GetVersion())!=5)

#define WZM_GOTOPAGE   (WM_USER+1)
#define WZN_PAGECHANGING  (WM_USER+1221)
#define WZN_CANCELCLICKED (WM_USER+1222)

struct DbToolOptions {
	TCHAR filename[MAX_PATH];
	TCHAR workingFilename[MAX_PATH];
	TCHAR outputFilename[MAX_PATH];
	TCHAR backupFilename[MAX_PATH];
	HANDLE hFile;
	HANDLE hOutFile;
	HANDLE hMap;
	BYTE *pFile;
	DWORD error;
	int bCheckOnly,bBackup,bAggressive;
	int bEraseHistory,bMarkRead,bConvertUtf;
};

extern HINSTANCE hInst;
extern DbToolOptions opts;
extern DBHeader dbhdr;

int DoMyControlProcessing(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam,INT_PTR *bReturn);

INT_PTR CALLBACK SelectDbDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK CleaningDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK ProgressDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK FileAccessDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK WizardDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK FinishedDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK WelcomeDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK OpenErrorDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);

struct DBSignature {
  char name[15];
  BYTE eof;
};
static struct DBSignature dbSignature={"Miranda ICQ DB",0x1A};

#define SIZEOF(X) (sizeof(X)/sizeof(X[0]))

#define STATUS_MESSAGE    0
#define STATUS_WARNING    1
#define STATUS_ERROR      2
#define STATUS_FATAL      3
#define STATUS_SUCCESS    4
#define STATUS_CLASSMASK  0x0f
int AddToStatus(DWORD flags, TCHAR* fmt,...);
void SetProgressBar(int perThou);

int PeekSegment(DWORD ofs,PVOID buf,int cbBytes);
int ReadSegment(DWORD ofs,PVOID buf,int cbBytes);
#define WSOFS_END   0xFFFFFFFF
#define WS_ERROR    0xFFFFFFFF
DWORD WriteSegment(DWORD ofs,PVOID buf,int cbBytes);
int ReadWrittenSegment(DWORD ofs,PVOID buf,int cbBytes);
int SignatureValid(DWORD ofs,DWORD signature);
DWORD ConvertModuleNameOfs(DWORD ofsOld);
void FreeModuleChain();

int TranslateDialog(HWND hwndDlg);
void LoadLangPackModule(void);
void UnloadLangPackModule(void);

char* LangPackTranslateString(const char *szEnglish, const int W);
__inline LPSTR Translate(LPSTR source)
{	return ( LPSTR )LangPackTranslateString( source, 0 );
}

#if defined( _UNICODE )
	#define TranslateT(s) (TCHAR*)LangPackTranslateString((LPCSTR)_T(s),1)
	#define TranslateTS(s) (TCHAR*)LangPackTranslateString((LPCSTR)s,1)
#else
	#define TranslateT(s) LangPackTranslateString(s,0)
	#define TranslateTS(s) LangPackTranslateString(s,0)
#endif

char* Utf8DecodeCP(char* str, int codepage, wchar_t** ucs2);
char* Utf8EncodeUcs2(const wchar_t* src);
bool is_utf8_string(const char* str);

