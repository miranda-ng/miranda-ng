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
	char  curhash[32+1];
	char  newhash[32+1];
	TCHAR tszDescr[256];
	FILEURL File;
	BOOL enabled;
	BYTE FileType;
	int  FileNum;
	BYTE Force;
};

typedef OBJLIST<FILEINFO> FILELIST;

struct PopupDataText
{
	TCHAR*  Title;
	TCHAR*  Text;
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

#ifdef WIN64
	#define DEFAULT_UPDATE_URL					"http://miranda-ng.org/x64"
#else
	#define DEFAULT_UPDATE_URL					"http://miranda-ng.org/x32"
#endif

#define IDINFO				3
#define IDDOWNLOAD			4
#define IDDOWNLOADALL		5

using namespace std;

extern HINSTANCE hInst;

extern TCHAR tszRoot[MAX_PATH], tszDialogMsg[2048];
extern FILEINFO* pFileInfo;
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
void __stdcall ExitMe(void*);
void __stdcall RestartMe(void*);
BOOL AllowUpdateOnStartup();
void InitTimer();

INT_PTR MenuCommand(WPARAM wParam,LPARAM lParam);
INT_PTR EmptyFolder(WPARAM wParam,LPARAM lParam);

INT_PTR CALLBACK DlgUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgMsgPop(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool unzip(const TCHAR* ptszZipFile, TCHAR* ptszDestPath, TCHAR* ptszBackPath);
