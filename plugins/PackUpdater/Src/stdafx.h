/*
Copyright (C) 2011-22 Mataes

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

#pragma once

// Windows Header Files:
#include <windows.h>
#include <Windowsx.h>
#include <time.h>
#include <Shlobj.h>

#include <string>
#include <vector>

// Miranda header files
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_popup.h>
#include <m_hotkeys.h>
#include <m_netlib.h>
#include <m_icolib.h>

#include <m_folders.h>

#include "version.h"
#include "resource.h"
#include "Notifications.h"

#define MODULENAME "PackUpdater"
#define MODULEA "Pack Updater"
#define MODULE L"Pack Updater"
#define DEFAULT_UPDATES_FOLDER	L"Pack Updates"
typedef std::wstring tString;

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

struct FILEURL
{
	wchar_t tszDownloadURL[2048];
	wchar_t tszDiskPath[MAX_PATH];
};

struct FILEINFO
{
	wchar_t tszCurVer[16];
	wchar_t tszNewVer[16];
	wchar_t tszLastVer[16];
	wchar_t tszAdvFolder[256];
	wchar_t tszInfoURL[2048];
	wchar_t tszMessage[5000];
	wchar_t tszDescr[256];
	FILEURL File;
	BOOL enabled;
	uint8_t FileType;
	INT FileNum;
	uint8_t Force;
};

#define DEFAULT_REMINDER					1
#define DEFAULT_UPDATEONSTARTUP				1
#define DEFAULT_ONLYONCEADAY				0
#define DEFAULT_UPDATEONPERIOD				0
#define DEFAULT_PERIOD						1
#define DEFAULT_PERIODMEASURE				1
#define DEFAULT_FILECOUNT					0
#define DEFAULT_FILETYPE					0 //0 - not defined, 1 - pack, 2 - plugin, 3 - icon, 4 - files in miranda root (e.g. langpack, dbtool), 5 - same as 4 without restart

#define IDINFO				3
#define IDDOWNLOAD			4
#define IDDOWNLOADALL		5

using std::wstring;
using namespace std;

extern INT FileCount, CurrentFile, Number, UpdatesCount, Period;
extern BOOL Silent, DlgDld;
extern uint8_t Reminder, UpdateOnStartup, UpdateOnPeriod, OnlyOnceADay, PeriodMeasure;
extern wchar_t tszRoot[MAX_PATH], tszDialogMsg[2048];
extern FILEINFO* pFileInfo;
extern FILEURL* pFileUrl;
extern HANDLE hCheckThread;
extern MYOPTIONS MyOptions;
extern aPopups PopupsList[POPUPS];
extern LPCTSTR Title, Text;
extern HANDLE Timer;

void InitPopupList();
void LoadOptions();
BOOL NetlibInit();
void IcoLibInit();
void NetlibUnInit();
INT ModulesLoaded(WPARAM wParam, LPARAM lParam);
INT_PTR MenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR EmptyFolder(WPARAM wParam, LPARAM lParam);
INT OnPreShutdown(WPARAM wParam, LPARAM lParam);
INT OptInit(WPARAM wParam, LPARAM lParam);
void DoCheck(INT iFlag);
BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal);
void show_popup(HWND hDlg, LPCTSTR Title, LPCTSTR Text, INT Number, INT ActType);
void DlgDownloadProc();
INT_PTR CALLBACK DlgUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgMsgPop(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void __stdcall ExitMe(void*);
void __stdcall RestartMe(void*);
BOOL AllowUpdateOnStartup();
void InitTimer();
