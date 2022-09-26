/*
Miranda Crash Dumper Plugin
Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <windows.h>
#include <windowsx.h>
#include <richedit.h>

#include <delayimp.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_hotkeys.h>
#include <m_protocols.h>
#include <m_icolib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_netlib.h>
#include <m_db_int.h>
#include <m_gui.h>

#include "m_folders.h"
#include "m_toptoolbar.h"

#include "sdkstuff.h"
#include "version.h"
#include "resource.h"

#define crsi_u2a(dst, src) \
{ \
	int cbLen = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL); \
	dst = (char*)alloca(cbLen+1); \
	WideCharToMultiByte(CP_ACP, 0, src, -1, dst, cbLen, NULL, NULL); \
}

#define crsi_a2u(dst, src, alloc) \
{ \
	int cbLen = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0); \
	dst = (wchar_t*)alloc(sizeof(wchar_t) * (cbLen+1)); \
	MultiByteToWideChar(CP_ACP, 0, src, -1, dst, cbLen); \
}



#define crsi_t2a(d,s) crsi_u2a(d,s)
#define crsi_a2t(d,s) crsi_a2u(d,s,alloca)
#define crs_a2t(d,s) crsi_a2u(d,s,mir_alloc)



#define MS_CRASHDUMPER_STORETOFILE "CrashDmp/StoreVerInfoToFile"
#define MS_CRASHDUMPER_STORETOCLIP "CrashDmp/StoreVerInfoToClip"
#define MS_CRASHDUMPER_GETINFO     "CrashDmp/GetInfo"
#define MS_CRASHDUMPER_VIEWINFO    "CrashDmp/ViewInfo"
#define MS_CRASHDUMPER_URL         "CrashDmp/StartUrl"
#define MS_CRASHDUMPER_URLTOCLIP   "CrashDmp/CopyToClip"

#define MODULENAME "Crash Dumper"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMOption<uint8_t> bCatchCrashes, bClassicDates, bUseSubFolder, bSuccessPopups;

	bool bNeedRestart = false, bServiceMode = false;

	CMPlugin();

	int Load() override;
	int Unload() override;
};

#define VI_FLAG_FORMAT  1
#define VI_FLAG_PRNVAR  2
#define VI_FLAG_PRNDLL  4
#define VI_FLAG_WEATHER 8

struct VerTrnsfr
{
	char* buf;
	bool  autot;
};

extern LCID packlcid;

extern wchar_t CrashLogFolder[MAX_PATH];
extern wchar_t VersionInfoFolder[MAX_PATH];

void WriteBBFile(CMStringW& buffer, bool hdr);
void WriteUtfFile(HANDLE hDumpFile, char* bufu);

LONG WINAPI myfilter(PEXCEPTION_POINTERS exc_ptr);
LONG WINAPI myfilterv(PEXCEPTION_POINTERS exc_ptr);
uint32_t MirandaThreadFilter(uint32_t code, EXCEPTION_POINTERS* info);

void GetInternetExplorerVersion(CMStringW& buffer);
void GetProcessorString(CMStringW& buffer);
void GetFreeMemoryString(CMStringW& buffer);
void GetFreeDiskString(LPCTSTR dirname, CMStringW& buffer);
void GetAdminString(CMStringW& buffer);
void GetLanguageString(CMStringW& buffer);
void GetLanguagePackString(CMStringW& buffer);
void GetWow64String(CMStringW& buffer);
void GetVersionInfo(HMODULE hLib, CMStringW& buffer);

void GetISO8061Time(SYSTEMTIME* stLocal, LPTSTR lpszString, uint32_t dwSize);

void ReadableExceptionInfo(PEXCEPTION_RECORD excrec, CMStringW& buffer);

void GetLastWriteTime(LPCTSTR fileName, LPTSTR lpszString, uint32_t dwSize);
void GetLastWriteTime(FILETIME* ftime, LPTSTR lpszString, uint32_t dwSize);
void ShowMessage(int type, const wchar_t* format, ...);

const PLUGININFOEX* GetMirInfo(HMODULE hModule);

void CreateMiniDump(HANDLE hDumpFile, PEXCEPTION_POINTERS exc_ptr);
void CreateCrashReport(HANDLE hDumpFile, PEXCEPTION_POINTERS exc_ptr, const wchar_t* msg);
void PrintVersionInfo(CMStringW& buffer, unsigned flags = VI_FLAG_PRNVAR);

void InitExceptionHandler(void);
void DestroyExceptionHandler(void);
void SetExceptionHandler(void);
void RemoveExceptionHandler(void);

extern CDlgBase *pViewDialog;

void OpenAuthUrl(const char* url);

void InitIcons(void);

int OptionsInit(WPARAM wParam, LPARAM);
INT_PTR ViewVersionInfo(WPARAM wParam, LPARAM);
