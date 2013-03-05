/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#define _CRT_SECURE_NO_WARNINGS

// Windows Header Files:
#include <time.h>
#include <stddef.h>
#include <windows.h>
#include <Windowsx.h>
#include <Shlobj.h>

// Miranda header files
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_database.h>
#include <m_system_cpp.h>
#include <m_popup.h>
#include <m_hotkeys.h>
#include <m_netlib.h>
#include <m_icolib.h>
#include <win2k.h>

#include <m_folders.h>
#include <m_popup2.h>

#include "version.h"
#include "resource.h"
#include "Notifications.h"

// Enable Visual Style
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define MODNAME "PluginUpdater"
#define MODULEA "Plugin Updater"
#define MODULE L"Plugin Updater"
#define DEFAULT_UPDATES_FOLDER L"Plugin Updates"

struct FILEURL
{
	TCHAR tszDownloadURL[2048];
	TCHAR tszDiskPath[MAX_PATH];
	int CRCsum;
};

struct FILEINFO
{
	TCHAR tszOldName[MAX_PATH], tszNewName[MAX_PATH];
	FILEURL File;
	BOOL bEnabled, bDeleteOnly;
};

typedef OBJLIST<FILEINFO> FILELIST;

struct PopupDataText
{
	TCHAR *Title;
	TCHAR *Text;
};

struct PlugOptions
{
	BYTE bUpdateOnStartup, bUpdateOnPeriod, bOnlyOnceADay, bUpdateIcons;
	BOOL bSilent, bDlgDld;

	BYTE bPeriodMeasure;
	int  Period;
};

#define DEFAULT_UPDATEICONS       0
#define DEFAULT_UPDATEONSTARTUP   1
#define DEFAULT_ONLYONCEADAY      1
#define DEFAULT_UPDATEONPERIOD    0
#define DEFAULT_PERIOD            1
#define DEFAULT_PERIODMEASURE     1

#define DEFAULT_UPDATE_URL        "http://miranda-ng.org/distr/stable/x%platform%"
#define DEFAULT_UPDATE_URL_TRUNK  "http://miranda-ng.org/distr/x%platform%"

#define IDINFO				3
#define IDDOWNLOAD			4
#define IDDOWNLOADALL		5

using namespace std;

extern HINSTANCE hInst;

extern TCHAR tszRoot[MAX_PATH], tszDialogMsg[2048], tszTempPath[MAX_PATH];
extern FILEINFO *pFileInfo;
extern HANDLE CheckThread, hPluginUpdaterFolder;
extern PlugOptions opts;
extern POPUP_OPTIONS PopupOptions;
extern aPopups PopupsList[POPUPS];
extern HANDLE Timer, hPipe;
extern HWND hwndDialog;

void InitPopupList();
void LoadOptions();
BOOL NetlibInit();
void IcoLibInit();
void NetlibUnInit();
int  ModulesLoaded(WPARAM wParam, LPARAM lParam);

int  OnFoldersChanged(WPARAM, LPARAM);
int  OnPreShutdown(WPARAM, LPARAM);
int  OptInit(WPARAM, LPARAM);

void BackupFile(TCHAR *ptszSrcFileName, TCHAR *ptszBackFileName);

void DoCheck(int iFlag);
BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal, int CRCsum);
void ShowPopup(HWND hDlg, LPCTSTR Title, LPCTSTR Text, int Number, int ActType);
void __stdcall RestartMe(void*);
BOOL AllowUpdateOnStartup();
void InitTimer();

INT_PTR MenuCommand(WPARAM wParam,LPARAM lParam);
INT_PTR EmptyFolder(WPARAM wParam,LPARAM lParam);

INT_PTR CALLBACK DlgUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgMsgPop(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool unzip(const TCHAR *ptszZipFile, TCHAR *ptszDestPath, TCHAR *ptszBackPath);
void strdel(TCHAR *parBuffer, int len);

//////////////////////////////////////////////////////////

BOOL IsRunAsAdmin();
BOOL IsProcessElevated();

int SafeCreateDirectory(const TCHAR *ptszDirName);
int SafeCopyFile(const TCHAR *ptszSrc, const TCHAR *ptszDst);
int SafeMoveFile(const TCHAR *ptszSrc, const TCHAR *ptszDst);
int SafeDeleteFile(const TCHAR *ptszSrc);
int SafeCreateFilePath(TCHAR *pFolder);

#if MIRANDA_VER < 0x0A00

#define db_free(A) DBFreeVariant(A)
#define db_set_s(A,B,C,D) DBWriteContactSettingString(A,B,C,D)

struct VARST : public mir_ptr<TCHAR>
{
	__forceinline VARST(const TCHAR *str) :
		mir_ptr<TCHAR>( Utils_ReplaceVarsT(str))
		{}
};

template<class T> class mir_ptr
{
	T *data;

public:
	__inline mir_ptr() : data((T*)mir_calloc(sizeof(T))) {}
	__inline mir_ptr(T *_p) : data(_p) {}
	__inline ~mir_ptr() { mir_free(data); }
	__inline T *operator = (T *_p) { if (data) mir_free(data); data = _p; return data; }
	__inline T *operator->() const { return data; }
	__inline operator T*() const { return data; }
	__inline operator INT_PTR() const { return (INT_PTR)data; }
};

class _A2T
{
	TCHAR *buf;

public:
	__forceinline _A2T(const char *s) : buf(mir_a2t(s)) {}
	__forceinline _A2T(const char *s, int cp) : buf(mir_a2t_cp(s, cp)) {}
	~_A2T() { mir_free(buf); }

	__forceinline operator TCHAR*() const
	{	return buf;
	}
};

class _T2A
{
	char *buf;

public:
	__forceinline _T2A(const TCHAR *s) : buf(mir_t2a(s)) {}
	__forceinline _T2A(const TCHAR *s, int cp) : buf(mir_t2a_cp(s, cp)) {}
	__forceinline ~_T2A() { mir_free(buf); }

	__forceinline operator char*() const
	{	return buf;
	}
};

__forceinline INT_PTR Options_Open(OPENOPTIONSDIALOG *ood)
{
	return CallService("Opt/OpenOptions", 0, (LPARAM)ood);
}

__forceinline INT_PTR Options_AddPage(WPARAM wParam, OPTIONSDIALOGPAGE *odp)
{
	return CallService("Opt/AddPage", wParam, (LPARAM)odp);
}

char *rtrim(char *str);

#define NEWTSTR_ALLOCA(A) (A == NULL)?NULL:_tcscpy((TCHAR*)alloca((_tcslen(A)+1) *sizeof(TCHAR)), A)

__forceinline HANDLE Skin_GetIconHandle(const char *szIconName)
{	return (HANDLE)CallService(MS_SKIN2_GETICONHANDLE, 0, (LPARAM)szIconName);
}

__forceinline HICON Skin_GetIcon(const char *szIconName, int size=0)
{	return (HICON)CallService(MS_SKIN2_GETICON, size, (LPARAM)szIconName);
}

__forceinline HGENMENU Menu_AddMainMenuItem(CLISTMENUITEM *mi)
{	return (HGENMENU)CallService("CList/AddMainMenuItem", 0, (LPARAM)mi);
}

__forceinline INT_PTR Hotkey_Register(HOTKEYDESC *hk)
{	return CallService("CoreHotkeys/Register", 0, (LPARAM)hk);
}

__forceinline INT_PTR CreateDirectoryTreeT(const TCHAR *ptszPath)
{	return CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)ptszPath);
}

__forceinline HANDLE Skin_AddIcon(SKINICONDESC *si)
{	return (HANDLE)CallService("Skin2/Icons/AddIcon", 0, (LPARAM)si);
}

#endif