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
#include <string.h>

// Miranda header files
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_system_cpp.h>
#include <m_popup.h>
#include <m_hotkeys.h>
#include <m_netlib.h>
#include <m_icolib.h>
#include <win2k.h>
#include <m_pluginupdater.h>

#include <m_folders.h>

#include "version.h"
#include "resource.h"

#if MIRANDA_VER < 0x0A00
#include "Compat\compat.h"
#endif

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
	TCHAR   tszOldName[MAX_PATH], tszNewName[MAX_PATH];
	FILEURL File;
	BOOL    bEnabled, bDeleteOnly;
};

typedef OBJLIST<FILEINFO> FILELIST;

extern struct PlugOptions
{
	BYTE bUpdateOnStartup, bUpdateOnPeriod, bOnlyOnceADay, bForceRedownload, bSilentMode;
	BOOL bSilent, bDlgDld;

	BYTE bPeriodMeasure;
	int  Period;
} opts;

#define DEFAULT_UPDATEONSTARTUP   1
#define DEFAULT_UPDATEONPERIOD    0
#define DEFAULT_PERIOD            1
#define DEFAULT_PERIODMEASURE     1

#if MIRANDA_VER < 0x0A00
	#define DEFAULT_ONLYONCEADAY      0
#else
	#define DEFAULT_ONLYONCEADAY      1
#endif

#define DEFAULT_UPDATE_URL                "http://miranda-ng.org/distr/stable/x%platform%"
#define DEFAULT_UPDATE_URL_TRUNK          "http://miranda-ng.org/distr/x%platform%"
#define DEFAULT_UPDATE_URL_TRUNK_SYMBOLS  "http://miranda-ng.org/distr/pdb_x%platform%"
#define PLUGIN_INFO_URL	_T("http://miranda-ng.org/p/%s")

#define UPDATE_MODE_CUSTOM			0
#define UPDATE_MODE_STABLE			1
#define UPDATE_MODE_TRUNK			2
#define UPDATE_MODE_TRUNK_SYMBOLS	3
#define UPDATE_MODE_MAX_VALUE		3 // when adding new mode, increment this number

#define DB_SETTING_UPDATE_MODE		"UpdateMode"
#define DB_SETTING_UPDATE_URL		"UpdateURL"
#define DB_SETTING_REDOWNLOAD		"ForceRedownload"
#define DB_SETTING_NEED_RESTART		"NeedRestart"
#define DB_SETTING_RESTART_COUNT	"RestartCount"
#define DB_SETTING_LAST_UPDATE		"LastUpdate"
#define DB_SETTING_DONT_SWITCH_TO_STABLE		"DontSwitchToStable"
#define DB_MODULE_FILES				MODNAME "Files"

#define MAX_RETRIES			3

#define IDINFO				3
#define IDDOWNLOAD			4
#define IDDOWNLOADALL		5

using namespace std;

extern HINSTANCE hInst;

extern TCHAR tszRoot[MAX_PATH], tszTempPath[MAX_PATH];
extern aPopups PopupsList[POPUPS];
extern HANDLE hPipe, hNetlibUser;
#if MIRANDA_VER >= 0x0A00
extern IconItemT iconList[];
#endif
void UninitCheck(void);
void UninitListNew(void);

class AutoHandle
{
	HANDLE &m_handle;
	AutoHandle& operator=(const AutoHandle&) { return *this; }

public:
	AutoHandle(HANDLE &_handle) : m_handle(_handle) {}
	~AutoHandle()
	{
		if (m_handle) {
			::CloseHandle(m_handle);
			m_handle = 0;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////

struct ServListEntry
{
	ServListEntry(const char* _name, const char* _hash, int _crc) :
		m_name( mir_a2t(_name)),
		m_crc(_crc)
	{
		strncpy(m_szHash, _hash, sizeof(m_szHash));
	}

	~ServListEntry()
	{
		mir_free(m_name);
	}

	TCHAR *m_name;
	DWORD  m_crc;
	char   m_szHash[32+1];
};

typedef OBJLIST<ServListEntry> SERVLIST;

///////////////////////////////////////////////////////////////////////////////

void  InitPopupList();
void  LoadOptions();
void  InitNetlib();
void  InitIcoLib();
void  InitServices();
void  InitEvents();
void  InitOptions();
void  InitListNew();
void  InitCheck();
void  CreateTimer();

void  UnloadCheck();
void  UnloadListNew();
void  UnloadNetlib();

void  BackupFile(TCHAR *ptszSrcFileName, TCHAR *ptszBackFileName);

bool  ParseHashes(const TCHAR *ptszUrl, ptrT &baseUrl, SERVLIST &arHashes);
int   CompareHashes(const ServListEntry *p1, const ServListEntry *p2);

int   GetUpdateMode();
TCHAR* GetDefaultUrl();
bool   DownloadFile(FILEURL *pFileURL, HANDLE &nlc);

void  ShowPopup(LPCTSTR Title, LPCTSTR Text, int Number);
void  __stdcall RestartMe(void*);
void  __stdcall OpenPluginOptions(void*);
void  CheckUpdateOnStartup();
void  InitTimer(void *type);

bool unzip(const TCHAR *ptszZipFile, TCHAR *ptszDestPath, TCHAR *ptszBackPath,bool ch);
void strdel(TCHAR *parBuffer, int len);

///////////////////////////////////////////////////////////////////////////////

int CalculateModuleHash(const TCHAR *tszFileName, char *dest);

BOOL IsProcessElevated();
bool PrepareEscalation();

int SafeCreateDirectory(const TCHAR *ptszDirName);
int SafeCopyFile(const TCHAR *ptszSrc, const TCHAR *ptszDst);
int SafeMoveFile(const TCHAR *ptszSrc, const TCHAR *ptszDst);
int SafeDeleteFile(const TCHAR *ptszSrc);
int SafeCreateFilePath(TCHAR *pFolder);

char *StrToLower(char *str);
