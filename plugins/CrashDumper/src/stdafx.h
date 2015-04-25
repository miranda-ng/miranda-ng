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

#include <windows.h>
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
#include <m_string.h>

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
#define MS_CRASHDUMPER_UPLOAD      "CrashDmp/UploadInfo"
#define MS_CRASHDUMPER_URL         "CrashDmp/StartUrl"
#define MS_CRASHDUMPER_URLTOCLIP   "CrashDmp/CopyToClip"

#define PluginName "Crash Dumper"

#define VI_FLAG_FORMAT  1
#define VI_FLAG_PRNVAR  2
#define VI_FLAG_PRNDLL  4
#define VI_FLAG_WEATHER 8

struct VerTrnsfr
{
	char* buf;
	bool  autot;
};

extern HMODULE hInst;
extern DWORD mirandaVersion;
extern LCID packlcid;
extern bool servicemode, clsdates, dtsubfldr, catchcrashes, needrestart;

extern TCHAR CrashLogFolder[MAX_PATH];
extern TCHAR VersionInfoFolder[MAX_PATH];

void WriteBBFile(CMString& buffer, bool hdr);
void WriteUtfFile(HANDLE hDumpFile, char* bufu);

LONG WINAPI myfilter(PEXCEPTION_POINTERS exc_ptr);
LONG WINAPI myfilterv(PEXCEPTION_POINTERS exc_ptr);
DWORD MirandaThreadFilter(DWORD code, EXCEPTION_POINTERS* info);

void GetOSDisplayString(CMString& buffer);
void GetInternetExplorerVersion(CMString& buffer);
void GetProcessorString(CMString& buffer);
void GetFreeMemoryString(CMString& buffer);
void GetFreeDiskString(LPCTSTR dirname, CMString& buffer);
void GetAdminString(CMString& buffer);
void GetLanguageString(CMString& buffer);
void GetLanguagePackString(CMString& buffer);
void GetWow64String(CMString& buffer);
void GetVersionInfo(HMODULE hLib, CMString& buffer);

void GetISO8061Time(SYSTEMTIME* stLocal, LPTSTR lpszString, DWORD dwSize);

void ReadableExceptionInfo(PEXCEPTION_RECORD excrec, CMString& buffer);

void GetLastWriteTime(LPCTSTR fileName, LPTSTR lpszString, DWORD dwSize);
void GetLastWriteTime(FILETIME* ftime, LPTSTR lpszString, DWORD dwSize);
bool CreateDirectoryTree(LPTSTR szDir);
void StoreStringToClip(CMString& buffer);
void ShowMessage(int type, const TCHAR* format, ...);
bool IsPluginEnabled(TCHAR* filename);

PLUGININFOEX* GetMirInfo(HMODULE hModule);
const PLUGININFOEX* GetPluginInfoEx(void);

void CreateMiniDump(HANDLE hDumpFile, PEXCEPTION_POINTERS exc_ptr);
void CreateCrashReport(HANDLE hDumpFile, PEXCEPTION_POINTERS exc_ptr, const TCHAR* msg);
void PrintVersionInfo(CMString& buffer, unsigned flags = VI_FLAG_PRNVAR);
bool ProcessVIHash(bool store);

void InitExceptionHandler(void);
void DestroyExceptionHandler(void);
void SetExceptionHandler(void);
void RemoveExceptionHandler(void);

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcView(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void DestroyAllWindows(void);

void UploadInit(void);
void UploadClose(void);
void OpenAuthUrl(const char* url);
void __cdecl VersionInfoUploadThread(void* arg);

void InitIcons(void);
HICON LoadIconEx(int iconId, bool big = false);
void  ReleaseIconEx(int iconId);
HANDLE GetIconHandle(int iconId);
