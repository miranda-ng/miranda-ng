/*
	StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
	Copyright 2003-2006 P. Boon

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

CFakePlugin SSPlugin(SSMODULENAME);

static HANDLE hServices[3], hEvents[3];
static UINT_PTR setStatusTimerId = 0;

/////////////////////////////////////////////////////////////////////////////////////////

static HANDLE hProtoAckHook, hCSStatusChangeHook, hStatusChangeHook;

static HWND hMessageWindow;

static uint8_t showDialogOnStartup = 0;

/////////////////////////////////////////////////////////////////////////////////////////
// command line options

static PROTOCOLSETTINGEX* IsValidProtocol(TProtoSettings &protoSettings, const char *protoName)
{
	for (auto &it : protoSettings)
		if (!it->ssDisabled && !strncmp(it->m_szName, protoName, mir_strlen(it->m_szName)))
			return it;

	return nullptr;
}

static int IsValidStatusDesc(char* statusDesc)
{
	if (!strncmp("away", statusDesc, 4))
		return ID_STATUS_AWAY;
	if (!strncmp("na", statusDesc, 2))
		return ID_STATUS_NA;
	if (!strncmp("dnd", statusDesc, 3))
		return ID_STATUS_DND;
	if (!strncmp("occupied", statusDesc, 8))
		return ID_STATUS_OCCUPIED;
	if (!strncmp("freechat", statusDesc, 8))
		return ID_STATUS_FREECHAT;
	if (!strncmp("online", statusDesc, 6))
		return ID_STATUS_ONLINE;
	if (!strncmp("offline", statusDesc, 7))
		return ID_STATUS_OFFLINE;
	if (!strncmp("invisible", statusDesc, 9))
		return ID_STATUS_INVISIBLE;
	if (!strncmp("last", statusDesc, 4))
		return ID_STATUS_LAST;

	return 0;
}

static void ProcessCommandLineOptions(TProtoSettings &protoSettings)
{
	if (protoSettings.getCount() == 0)
		return;

	char *cmdl = GetCommandLineA();
	while (*cmdl != '\0') {
		while (*cmdl != '/') {
			if (*cmdl == '\0')
				return;

			cmdl++;
		}
		if (*cmdl == '\0')
			return;

		cmdl++;
		if (!strncmp(cmdl, "showdialog", 10)) {
			showDialogOnStartup = TRUE;
			continue;
		}
		char *protoName = cmdl; // first protocol ?
		PROTOCOLSETTINGEX* protoSetting = IsValidProtocol(protoSettings, protoName);
		if (protoSetting != nullptr) {
			while (*cmdl != '=') {
				if (*cmdl == '\0')
					return;

				cmdl++; // skip to status
			}

			if (*cmdl == '\0')
				return;

			cmdl++;
			char *statusDesc = cmdl;
			int status = IsValidStatusDesc(statusDesc);
			if (status != 0)
				protoSetting->m_status = status;
		}
	}
}

static void SetLastStatusMessages(TProtoSettings &ps)
{
	for (auto &it : ps) {
		if (it->m_status != ID_STATUS_LAST)
			continue;

		char dbSetting[128];
		mir_snprintf(dbSetting, "%s%s", PREFIX_LASTMSG, it->m_szName);
		it->m_szMsg = SSPlugin.getWStringA(dbSetting);
	}
}

void SS_LoadDynamic(SMProto *setting)
{
	if (!SSPlugin.getByte(SETTING_SETPROFILE, 1))
		return;

	TProtoSettings ps;
	ps.insert(new SMProto(*setting));
	SetStatusEx(ps);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Account control event

// 'allow override'
static int ProcessProtoAck(WPARAM, LPARAM lParam)
{
	// 'something' made a status change
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_STATUS && ack->result != ACKRESULT_FAILED)
		return 0;

	if (!SSPlugin.getByte(SETTING_OVERRIDE, 1) || protoList.getCount() == 0)
		return 0;

	for (auto &it : protoList) {
		if (!mir_strcmp(ack->szModule, it->m_szName)) {
			it->ssDisabled = true;
			log_debug(0, "StartupStatus: %s overridden by ME_PROTO_ACK, status will not be set", ack->szModule);
		}
	}

	return 0;
}

static int StatusChange(WPARAM, LPARAM lParam)
{
	// change by menu
	if (!SSPlugin.getByte(SETTING_OVERRIDE, 1) || protoList.getCount() == 0)
		return 0;

	char *szProto = (char *)lParam;
	if (szProto == nullptr) { // global status change
		for (auto &it : protoList) {
			it->ssDisabled = true;
			log_debug(0, "StartupStatus: all protos overridden by ME_CLIST_STATUSMODECHANGE, status will not be set");
		}
	}
	else {
		for (auto &it : protoList) {
			if (!mir_strcmp(it->m_szName, szProto)) {
				it->ssDisabled = true;
				log_debug(0, "StartupStatus: %s overridden by ME_CLIST_STATUSMODECHANGE, status will not be set", szProto);
			}
		}
	}

	return 0;
}

static int CSStatusChangeEx(WPARAM wParam, LPARAM)
{
	// another status plugin made the change
	if (!SSPlugin.getByte(SETTING_OVERRIDE, 1) || protoList.getCount() == 0)
		return 0;

	if (wParam != 0) {
		PROTOCOLSETTINGEX **ps = *(PROTOCOLSETTINGEX***)wParam;
		if (ps == nullptr)
			return -1;

		for (int i = 0; i < protoList.getCount(); i++) {
			for (auto &it : protoList) {
				if (ps[i]->m_szName == nullptr || it->m_szName == nullptr)
					continue;

				if (!mir_strcmp(ps[i]->m_szName, it->m_szName)) {
					log_debug(0, "StartupStatus: %s overridden by MS_CS_SETSTATUSEX, status will not be set", ps[i]->m_szName);
					it->ssDisabled = true;
				}
			}
		}
	}

	return 0;
}

static void CALLBACK SetStatusTimed(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(nullptr, setStatusTimerId);
	UnhookEvent(hProtoAckHook);
	UnhookEvent(hCSStatusChangeHook);
	UnhookEvent(hStatusChangeHook);

	TProtoSettings ps(protoList);
	for (auto &it : ps)
		if (it->ssDisabled)
			it->m_status = ID_STATUS_DISABLED;

	SetStatusEx(ps);
}

static int OnOkToExit(WPARAM, LPARAM)
{
	// save last protocolstatus
	for (auto &pa : Accounts()) {
		if (!IsSuitableProto(pa))
			continue;

		if (!Proto_GetAccount(pa->szModuleName))
			continue;

		char lastName[128], lastMsg[128];
		mir_snprintf(lastName, "%s%s", PREFIX_LAST, pa->szModuleName);
		SSPlugin.setWord(lastName, pa->iRealStatus);
		mir_snprintf(lastMsg, "%s%s", PREFIX_LASTMSG, pa->szModuleName);
		SSPlugin.delSetting(lastMsg);

		if (!(CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG))
			continue;

		if (!(CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(pa->iRealStatus)))
			continue;

		// NewAwaySys
		if (ServiceExists(MS_NAS_GETSTATE)) {
			NAS_PROTOINFO npi = { sizeof(npi) };
			npi.szProto = pa->szModuleName;
			CallService(MS_NAS_GETSTATE, (WPARAM)&npi, 1);
			if (npi.szMsg == nullptr) {
				npi.status = 0;
				npi.szProto = nullptr;
				CallService(MS_NAS_GETSTATE, (WPARAM)&npi, 1);
			}
			if (npi.szMsg != nullptr) {
				SSPlugin.setWString(lastMsg, npi.tszMsg);
				mir_free(npi.tszMsg);
			}
		}
	}

	if (SSPlugin.getByte(SETTING_SETPROFILE, 1) || SSPlugin.getByte(SETTING_OFFLINECLOSE, 0))
		Clist_SetStatusMode(ID_STATUS_OFFLINE);

	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	// set windowstate and docked for next startup
	if (SSPlugin.getByte(SETTING_SETWINSTATE, 0)) {
		int state = SSPlugin.getByte(SETTING_WINSTATE, SETTING_STATE_NORMAL);
		HWND hClist = g_clistApi.hwndContactList;
		BOOL isHidden = !IsWindowVisible(hClist);
		switch (state) {
		case SETTING_STATE_HIDDEN:
			// try to use services where possible
			if (!isHidden)
				g_clistApi.pfnShowHide();
			break;

		case SETTING_STATE_MINIMIZED:
			if (!db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 0))
				ShowWindow(hClist, SW_SHOWMINIMIZED);
			break;

		case SETTING_STATE_NORMAL:
			// try to use services where possible (that's what they're for)
			if (isHidden)
				g_clistApi.pfnShowHide();
			break;
		}
	}

	// hangup
	if (SSPlugin.getByte(SETTING_AUTOHANGUP, 0))
		InternetAutodialHangup(0);

	int state = SSPlugin.getByte(SETTING_WINSTATE, SETTING_STATE_NORMAL);
	// set windowstate and docked for next startup
	if (SSPlugin.getByte(SETTING_SETWINSTATE, 0))
		db_set_b(0, MODULE_CLIST, SETTING_WINSTATE, (uint8_t)state);

	if (hMessageWindow)
		DestroyWindow(hMessageWindow);

	ShutdownConfirmDialog();
	protoList.destroy();
	return 0;
}

/* Window proc for poweroff event */
static uint32_t CALLBACK MessageWndProc(HWND, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_ENDSESSION:
		log_debug(0, "WM_ENDSESSION");
		if (wParam) {
			log_debug(0, "WM_ENDSESSION: calling exit");
			OnShutdown(0, 0);
			log_debug(0, "WM_ENDSESSION: exit called");
		}
		break;
	}

	return TRUE;
}

int SSModuleLoaded(WPARAM, LPARAM)
{
	InitProfileModule();

	hEvents[0] = HookEvent(ME_OPT_INITIALISE, StartupStatusOptionsInit);

	/* shutdown hook for normal shutdown */
	hEvents[1] = HookEvent(ME_SYSTEM_OKTOEXIT, OnOkToExit);
	hEvents[2] = HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);
	/* message window for poweroff */
	hMessageWindow = CreateWindowEx(0, L"STATIC", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
	SetWindowLongPtr(hMessageWindow, GWLP_WNDPROC, (LONG_PTR)MessageWndProc);

	GetProfile(-1, protoList);

	// override with cmdl
	ProcessCommandLineOptions(protoList);
	if (protoList.getCount() == 0)
		return 0;// no protocols are loaded

	SetLastStatusMessages(protoList);
	showDialogOnStartup = (showDialogOnStartup || SSPlugin.getByte(SETTING_SHOWDIALOG, 0));

	// dial
	if (showDialogOnStartup || SSPlugin.getByte(SETTING_SETPROFILE, 1))
		if (SSPlugin.getByte(SETTING_AUTODIAL, 0))
			InternetAutodial(0, nullptr);

	// set the status!
	if (showDialogOnStartup || SSPlugin.getByte(SETTING_SHOWDIALOG, 0))
		ShowConfirmDialogEx((TProtoSettings*)&protoList, SSPlugin.getDword(SETTING_DLGTIMEOUT, 5));
	else if (SSPlugin.getByte(SETTING_SETPROFILE, 1)) {
		// set hooks for override
		if (SSPlugin.getByte(SETTING_OVERRIDE, 1)) {
			hProtoAckHook = HookEvent(ME_PROTO_ACK, ProcessProtoAck);
			hCSStatusChangeHook = HookEvent(ME_CS_STATUSCHANGEEX, CSStatusChangeEx);
			hStatusChangeHook = HookEvent(ME_CLIST_STATUSMODECHANGE, StatusChange);
		}
		setStatusTimerId = SetTimer(nullptr, 0, SSPlugin.getDword(SETTING_SETPROFILEDELAY, 500), SetStatusTimed);
	}

	// win size and location
	if (SSPlugin.getByte(SETTING_SETWINLOCATION, 0) || SSPlugin.getByte(SETTING_SETWINSIZE, 0)) {
		HWND hClist = g_clistApi.hwndContactList;

		// store in db
		if (SSPlugin.getByte(SETTING_SETWINLOCATION, 0)) {
			db_set_dw(0, MODULE_CLIST, SETTING_XPOS, SSPlugin.getDword(SETTING_XPOS, 0));
			db_set_dw(0, MODULE_CLIST, SETTING_YPOS, SSPlugin.getDword(SETTING_YPOS, 0));
		}
		if (SSPlugin.getByte(SETTING_SETWINSIZE, 0)) {
			db_set_dw(0, MODULE_CLIST, SETTING_WIDTH, SSPlugin.getDword(SETTING_WIDTH, 0));
			if (!db_get_b(0, MODULE_CLUI, SETTING_AUTOSIZE, 0))
				db_set_dw(0, MODULE_CLIST, SETTING_HEIGHT, SSPlugin.getDword(SETTING_HEIGHT, 0));
		}

		WINDOWPLACEMENT wndpl = { sizeof(wndpl) };
		if (GetWindowPlacement(hClist, &wndpl)) {
			if (wndpl.showCmd == SW_SHOWNORMAL && !Clist_IsDocked()) {
				RECT rc;
				if (GetWindowRect(hClist, &rc)) {
					int x = rc.left;
					int y = rc.top;
					int width = rc.right - rc.left;
					int height = rc.bottom - rc.top;
					if (SSPlugin.getByte(SETTING_SETWINLOCATION, 0)) {
						x = SSPlugin.getDword(SETTING_XPOS, x);
						y = SSPlugin.getDword(SETTING_YPOS, y);
					}
					if (SSPlugin.getByte(SETTING_SETWINSIZE, 0)) {
						width = SSPlugin.getDword(SETTING_WIDTH, width);
						if (!db_get_b(0, MODULE_CLUI, SETTING_AUTOSIZE, 0))
							height = SSPlugin.getDword(SETTING_HEIGHT, height);
					}
					MoveWindow(hClist, x, y, width, height, TRUE);
				}
			}
		}
	}

	return 0;
}

static INT_PTR SrvGetProfile(WPARAM wParam, LPARAM lParam)
{
	return GetProfile((int)wParam, *(TProtoSettings*)lParam);
}

void StartupStatusLoad()
{
	if (g_plugin.bMirandaLoaded)
		SSModuleLoaded(0, 0);
	else
		HookEvent(ME_SYSTEM_MODULESLOADED, SSModuleLoaded);

	if (SSPlugin.getByte(SETTING_SETPROFILE, 1) || SSPlugin.getByte(SETTING_OFFLINECLOSE, 0))
		db_set_w(0, "CList", "Status", (uint16_t)ID_STATUS_OFFLINE);

	// docking
	if (SSPlugin.getByte(SETTING_SETDOCKED, 0)) {
		int docked = SSPlugin.getByte(SETTING_DOCKED, DOCKED_NONE);
		if (docked == DOCKED_LEFT || docked == DOCKED_RIGHT)
			docked = -docked;

		db_set_b(0, MODULE_CLIST, SETTING_DOCKED, (uint8_t)docked);
	}

	// Create service functions; the get functions are created here; they don't rely on commonstatus
	hServices[0] = CreateServiceFunction(MS_SS_GETPROFILE, SrvGetProfile);
	hServices[1] = CreateServiceFunction(MS_SS_GETPROFILECOUNT, GetProfileCount);
	hServices[2] = CreateServiceFunction(MS_SS_GETPROFILENAME, GetProfileName);

	LoadProfileModule();
}

void StartupStatusUnload()
{
	if (g_plugin.bMirandaLoaded)
		OnShutdown(0, 0);

	KillModuleOptions(&SSPlugin);

	for (auto &it : hServices) {
		DestroyServiceFunction(it);
		it = nullptr;
	}

	for (auto &it : hEvents) {
		UnhookEvent(it);
		it = nullptr;
	}

	DeinitProfilesModule();
}
