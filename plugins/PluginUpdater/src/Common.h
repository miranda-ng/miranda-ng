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

#define MIRANDA_VER    0x0A00

// Windows Header Files:
#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <Windowsx.h>
#include <vector>       // stl vector header
#include <map>
#include <string>
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
	char  curhash[32+1];
	char  newhash[32+1];
	TCHAR tszDescr[256];
	FILEURL File;
	BOOL enabled;
	BYTE FileType;
	INT FileNum;
	BYTE Force;
};

struct PopupDataText
{
	TCHAR*  Title;
	TCHAR*  Text;
};

struct PlugOptions
{
	BYTE bReminder, bUpdateOnStartup, bUpdateOnPeriod, bOnlyOnceADay, bUpdateIcons;
	BOOL bSilent, bDlgDld;

	BYTE bPeriodMeasure;
	INT  Period;
};

#define DEFAULT_REMINDER          1
#define DEFAULT_UPDATEICONS       0
#define DEFAULT_UPDATEONSTARTUP   1
#define DEFAULT_ONLYONCEADAY      0
#define DEFAULT_UPDATEONPERIOD    0
#define DEFAULT_PERIOD            1
#define DEFAULT_PERIODMEASURE     1

#ifdef WIN64
	#define DEFAULT_UPDATE_URL					"http://nightly.miranda.im/x64"
#else
	#define DEFAULT_UPDATE_URL					"http://nightly.miranda.im/x32"
#endif

#define IDINFO				3
#define IDDOWNLOAD			4
#define IDDOWNLOADALL		5

using namespace std;

extern HINSTANCE hInst;

extern TCHAR tszRoot[MAX_PATH], tszDialogMsg[2048];
extern FILEINFO* pFileInfo;
extern HANDLE CheckThread;
extern PlugOptions opts;
extern POPUP_OPTIONS PopupOptions;
extern aPopups PopupsList[POPUPS];
extern HANDLE Timer;
extern HWND hwndDialog;

VOID InitPopupList();
VOID LoadOptions();
BOOL NetlibInit();
VOID IcoLibInit();
VOID NetlibUnInit();
INT ModulesLoaded(WPARAM wParam, LPARAM lParam);
INT_PTR MenuCommand(WPARAM wParam,LPARAM lParam);
INT_PTR EmptyFolder(WPARAM wParam,LPARAM lParam);
INT OnPreShutdown(WPARAM wParam, LPARAM lParam);
INT OptInit(WPARAM wParam, LPARAM lParam);
VOID DoCheck(INT iFlag);
BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal);
VOID ShowPopup(HWND hDlg, LPCTSTR Title, LPCTSTR Text, INT Number, INT ActType);
INT_PTR CALLBACK DlgUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgMsgPop(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void __stdcall ExitMe(void*);
void __stdcall RestartMe(void*);
BOOL AllowUpdateOnStartup();
VOID InitTimer();