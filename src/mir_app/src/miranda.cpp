/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "stdafx.h"
#include "clc.h"

#if defined(VLD_ENABLED)
#include "msapi/vld.h"
#endif

#pragma comment(lib, "version.lib")

int LoadDefaultModules(void);
void UnloadNewPluginsModule(void);
void UnloadDefaultModules(void);

HANDLE hOkToExitEvent, hModulesLoadedEvent;
uint32_t hMainThreadId;
bool g_bModulesLoadedFired = false;
int g_iIconX, g_iIconY, g_iIconSX, g_iIconSY;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	"Miranda NG",
	MIRANDA_VERSION_DWORD,
	"",
	"Miranda NG team",
	LEGAL_COPYRIGHT,
	"https://miranda-ng.org",
	UNICODE_AWARE,
	MIID_LAST
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(nullptr, pluginInfoEx)
{}

#ifdef _WINDOWS

/////////////////////////////////////////////////////////////////////////////////////////
// dll entry point

BOOL WINAPI DllMain(HINSTANCE hinstDLL, uint32_t dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH) {
		g_plugin.setInst(hinstDLL);
		RegisterModule(&g_plugin);

		g_iIconX = GetSystemMetrics(SM_CXICON);
		g_iIconY = GetSystemMetrics(SM_CYICON);
		g_iIconSX = GetSystemMetrics(SM_CXSMICON);
		g_iIconSY = GetSystemMetrics(SM_CYSMICON);

		#if defined(VLD_ENABLED)
			VLDSetOptions(VLD_OPT_SKIP_HEAPFREE_LEAKS, -1, -1);
			VLDSetReportOptions(VLD_OPT_REPORT_TO_FILE, L"C:\\temp\\vld.txt");
		#endif
	}
	return TRUE;
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////

static int SystemShutdownProc(WPARAM, LPARAM)
{
	UnloadDefaultModules();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

#define MIRANDA_PROCESS_WAIT_TIMEOUT        60000
#define MIRANDA_PROCESS_WAIT_RESOLUTION     1000
#define MIRANDA_PROCESS_WAIT_STEPS          (MIRANDA_PROCESS_WAIT_TIMEOUT/MIRANDA_PROCESS_WAIT_RESOLUTION)

class CWaitRestartDlg : public CDlgBase
{
	HANDLE m_hProcess;
	CTimer m_timer;
	CCtrlProgress m_progress;
	CCtrlButton m_cancel;

protected:
	bool OnInitDialog() override
	{
		m_progress.SetRange(MIRANDA_PROCESS_WAIT_STEPS);
		m_progress.SetStep(1);
		m_timer.Start(MIRANDA_PROCESS_WAIT_RESOLUTION);
		return true;
	}

	void Timer_OnEvent(CTimer*)
	{
		if (m_progress.Move() == MIRANDA_PROCESS_WAIT_STEPS)
			EndModal(0);

		#ifdef _WINDOWS
			if (WaitForSingleObject(m_hProcess, 1) != WAIT_TIMEOUT) {
				m_progress.SetPosition(MIRANDA_PROCESS_WAIT_STEPS);
				EndModal(0);
			}
		#endif
	}

	void Cancel_OnClick(CCtrlBase*)
	{
		m_progress.SetPosition(MIRANDA_PROCESS_WAIT_STEPS);
		EndModal(1);
	}

public:
	CWaitRestartDlg(HANDLE hProcess)
		: CDlgBase(g_plugin, IDD_WAITRESTART), m_timer(this, 1),
		m_progress(this, IDC_PROGRESSBAR), m_cancel(this, IDCANCEL)
	{
		m_autoClose = 0;
		m_hProcess = hProcess;
		m_timer.OnEvent = Callback(this, &CWaitRestartDlg::Timer_OnEvent);
		m_cancel.OnClick = Callback(this, &CWaitRestartDlg::Cancel_OnClick);
	}
};

INT_PTR CheckRestart()
{
	LPCTSTR tszPID = CmdLine_GetOption(L"restart");
	if (tszPID) {
		#ifdef _WINDOWS
			HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, _wtol(tszPID));
			if (hProcess) {
				INT_PTR result = CWaitRestartDlg(hProcess).DoModal();
				CloseHandle(hProcess);
				return result;
			}
		#endif
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class MSystemWindow : public CDlgBase
{

public:
	MSystemWindow() :
		CDlgBase(g_plugin, IDD_EMPTY)
	{
	}
};

static MSystemWindow *g_pSystemWindow;

/////////////////////////////////////////////////////////////////////////////////////////

int CALLBACK mir_main(LPTSTR cmdLine)
{
	hMainThreadId = GetCurrentThreadId();

	CmdLine_Parse(cmdLine);
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

	g_pSystemWindow = new MSystemWindow();
	g_pSystemWindow->Create();

	BeginMessageLoop();

	int result = 0;
	if (LoadDefaultModules()) {
		Miranda_SetTerminated();

		UnloadDefaultModules();

		result = 1;
	}
	else {
		InitPathVar();

		if (plugin_service == nullptr) {
			NotifyEventHooks(hModulesLoadedEvent, 0, 0);
			g_bModulesLoadedFired = true;
		}

		// ensure that the kernel hooks the SystemShutdownProc() after all plugins
		HookEvent(ME_SYSTEM_SHUTDOWN, SystemShutdownProc);

		EnterMessageLoop();
	}

	UnloadNewPluginsModule();
	UnloadCoreModule();

	UninitTray();

	delete g_pSystemWindow;

	LeaveMessageLoop();
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(bool) Miranda_OkToExit()
{
	return NotifyEventHooks(hOkToExitEvent, 0, 0) == 0;
}

MIR_APP_DLL(void) Miranda_Close()
{
	while (!Miranda_OkToExit()) {
		#ifdef _WINDOWS
			MSG msg;
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			Sleep(0);
		#endif
	}

	#ifdef _WINDOWS
		DestroyWindow(g_clistApi.hwndContactList);
	#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
// version functions

MIR_APP_DLL(uint32_t) Miranda_GetVersion()
{
	#ifdef _WINDOWS
		wchar_t filename[MAX_PATH];
		GetModuleFileName(g_plugin.getInst(), filename, _countof(filename));

		DWORD unused, verInfoSize = GetFileVersionInfoSize(filename, &unused);
		PVOID pVerInfo = _alloca(verInfoSize);
		GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);

		UINT blockSize;
		VS_FIXEDFILEINFO *vsffi;
		VerQueryValue(pVerInfo, L"\\", (PVOID*)&vsffi, &blockSize);
		return (((vsffi->dwProductVersionMS >> 16) & 0xFF) << 24) |
			((vsffi->dwProductVersionMS & 0xFF) << 16) |
			(((vsffi->dwProductVersionLS >> 16) & 0xFF) << 8) |
			(vsffi->dwProductVersionLS & 0xFF);
	#else
		return MIRANDA_VERSION_DWORD;
	#endif
}

MIR_APP_DLL(void) Miranda_GetFileVersion(MFileVersion *pVer)
{
	#ifdef _WINDOWS
		wchar_t filename[MAX_PATH];
		GetModuleFileName(g_plugin.getInst(), filename, _countof(filename));

		DWORD unused, verInfoSize = GetFileVersionInfoSize(filename, &unused);
		PVOID pVerInfo = _alloca(verInfoSize);
		GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);

		UINT blockSize;
		VS_FIXEDFILEINFO *vsffi;
		VerQueryValue(pVerInfo, L"\\", (PVOID*)&vsffi, &blockSize);

		(*pVer)[0] = HIWORD(vsffi->dwProductVersionMS);
		(*pVer)[1] = LOWORD(vsffi->dwProductVersionMS);
		(*pVer)[2] = HIWORD(vsffi->dwProductVersionLS);
		(*pVer)[3] = LOWORD(vsffi->dwProductVersionLS);
	#else
		uint16_t tmp[4] = { MIRANDA_VERSION_FILEVERSION };
		memcpy(pVer, tmp, sizeof(tmp));
	#endif
}

MIR_APP_DLL(void) Miranda_GetVersionText(char *pDest, size_t cbSize)
{
	#ifdef _WINDOWS
		wchar_t filename[MAX_PATH], *productVersion;
		GetModuleFileName(g_plugin.getInst(), filename, _countof(filename));

		DWORD unused, verInfoSize = GetFileVersionInfoSize(filename, &unused);
		PVOID pVerInfo = _alloca(verInfoSize);
		GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);

		UINT blockSize;
		VerQueryValue(pVerInfo, L"\\StringFileInfo\\000004b0\\ProductVersion", (LPVOID*)&productVersion, &blockSize);
		strncpy_s(pDest, cbSize, _T2A(productVersion), _TRUNCATE);
	#else
		strncpy_s(pDest, cbSize, "Miranda NG " MIRANDA_VERSION_DISPLAY, _TRUNCATE);
	#endif

	#if defined(_WIN64)
		strcat_s(pDest, cbSize, " x64");
	#endif
}

MIR_APP_DLL(CDlgBase *) Miranda_GetSystemWindow()
{
	return g_pSystemWindow;
}

///////////////////////////////////////////////////////////////////////////////

extern HANDLE hOptionsInitEvent;

int LoadSystemModule(void)
{
	hModulesLoadedEvent = CreateHookableEvent(ME_SYSTEM_MODULESLOADED);
	hOkToExitEvent = CreateHookableEvent(ME_SYSTEM_OKTOEXIT);
	hOptionsInitEvent = CreateHookableEvent(ME_OPT_INITIALISE);
	
	// this event is created inside the core not to lose its hooks when MIID_UIUSERINFO plugin reloads
	CreateHookableEvent(ME_USERINFO_INITIALISE);
	return 0;
}
