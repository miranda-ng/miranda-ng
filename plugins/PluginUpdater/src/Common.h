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
#include <stdio.h>
#include <malloc.h>
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
#include <m_utils.h>
#include <m_system.h>
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

#define MODNAME "PluginUpdater"
#define MODULEA "Plugin Updater"
#define MODULEW L"Plugin Updater"
#define DEFAULT_UPDATES_FOLDER L"Plugin Updates"
#define MODULE	MODULEW

struct FILEURL
{
	TCHAR tszDownloadURL[2048];
	TCHAR tszDiskPath[MAX_PATH];
};

struct FILEINFO
{
	TCHAR tszOldName[MAX_PATH], tszNewName[MAX_PATH];
	FILEURL File;
	BOOL enabled, bDeleteOnly;
	BYTE FileType;
	int  FileNum;
	BYTE Force;
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

#define DEFAULT_UPDATE_URL					"http://miranda-ng.org/distr/stable/x%platform%"

#define IDINFO				3
#define IDDOWNLOAD			4
#define IDDOWNLOADALL		5

using namespace std;

extern HINSTANCE hInst;

extern TCHAR tszRoot[MAX_PATH], tszDialogMsg[2048];
extern FILEINFO *pFileInfo;
extern HANDLE CheckThread, hPluginUpdaterFolder;
extern PlugOptions opts;
extern POPUP_OPTIONS PopupOptions;
extern aPopups PopupsList[POPUPS];
extern HANDLE Timer;
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

void DoCheck(int iFlag);
BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal);
void ShowPopup(HWND hDlg, LPCTSTR Title, LPCTSTR Text, int Number, int ActType);
void __stdcall RestartMe(void*);
BOOL AllowUpdateOnStartup();
void InitTimer();

INT_PTR MenuCommand(WPARAM wParam,LPARAM lParam);
INT_PTR EmptyFolder(WPARAM wParam,LPARAM lParam);

INT_PTR CALLBACK DlgUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgMsgPop(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool unzip(const TCHAR *ptszOldFileName, const TCHAR *ptszZipFile, TCHAR *ptszDestPath, TCHAR *ptszBackPath);
void strdel(TCHAR *parBuffer, int len);

#if MIRANDA_VER < 0x0A00

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
void CreatePathToFileT(TCHAR *szFilePath);

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