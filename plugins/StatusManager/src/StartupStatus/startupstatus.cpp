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

#include "..\stdafx.h"

static UINT_PTR setStatusTimerId = 0;

static TSettingsList startupSettings(10, SSCompareSettings);

TSSSetting::TSSSetting(PROTOACCOUNT *pa)
{
	m_szName = pa->szModuleName;
	m_tszAccName = pa->tszAccountName;
	m_status = m_lastStatus = CallProtoService(pa->szModuleName, PS_GETSTATUS, 0, 0);
	m_szMsg = nullptr;
}

TSSSetting::TSSSetting(int profile, PROTOACCOUNT *pa)
{
	// copy name
	m_szName = pa->szModuleName;
	m_tszAccName = pa->tszAccountName;

	// load status
	char setting[80];
	mir_snprintf(setting, "%d_%s", profile, pa->szModuleName);
	int iStatus = db_get_w(0, SSMODULENAME, setting, 0);
	if (iStatus < MIN_STATUS || iStatus > MAX_STATUS)
		iStatus = DEFAULT_STATUS;
	m_status = iStatus;

	// load last status
	mir_snprintf(setting, "%s%s", PREFIX_LAST, m_szName);
	iStatus = db_get_w(0, SSMODULENAME, setting, 0);
	if (iStatus < MIN_STATUS || iStatus > MAX_STATUS)
		iStatus = DEFAULT_STATUS;
	m_lastStatus = iStatus;

	m_szMsg = GetStatusMessage(profile, m_szName);
	if (m_szMsg)
		m_szMsg = wcsdup(m_szMsg);
}

TSSSetting::~TSSSetting()
{
	free(m_szMsg);
}

/////////////////////////////////////////////////////////////////////////////////////////

static HANDLE hProtoAckHook, hCSStatusChangeHook, hStatusChangeHook;

static HWND hMessageWindow;

static BYTE showDialogOnStartup = 0;

/////////////////////////////////////////////////////////////////////////////////////////
// command line options

static PROTOCOLSETTINGEX* IsValidProtocol(TSettingsList& protoSettings, char* protoName)
{
	for (int i = 0; i < protoSettings.getCount(); i++)
		if (!strncmp(protoSettings[i].m_szName, protoName, mir_strlen(protoSettings[i].m_szName)))
			return &protoSettings[i];

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
	if (!strncmp("onthephone", statusDesc, 10))
		return ID_STATUS_ONTHEPHONE;
	if (!strncmp("outtolunch", statusDesc, 10))
		return ID_STATUS_OUTTOLUNCH;
	if (!strncmp("last", statusDesc, 4))
		return ID_STATUS_LAST;

	return 0;
}

static void ProcessCommandLineOptions(TSettingsList& protoSettings)
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

static void SetLastStatusMessages(TSettingsList &ps)
{
	for (int i = 0; i < ps.getCount(); i++) {
		if (ps[i].m_status != ID_STATUS_LAST)
			continue;

		char dbSetting[128];
		mir_snprintf(dbSetting, "%s%s", PREFIX_LASTMSG, ps[i].m_szName);

		DBVARIANT dbv;
		if (ps[i].m_szMsg == nullptr && !db_get_ws(0, SSMODULENAME, dbSetting, &dbv)) {
			ps[i].m_szMsg = wcsdup(dbv.ptszVal); // remember this won't be freed
			db_free(&dbv);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Account control event

int OnSSAccChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;
	switch (wParam) {
	case PRAC_ADDED:
		startupSettings.insert(new TSSSetting(-1, pa));
		break;

	case PRAC_REMOVED:
		for (int i = 0; i < startupSettings.getCount(); i++) {
			if (!mir_strcmp(startupSettings[i].m_szName, pa->szModuleName)) {
				startupSettings.remove(i);
				break;
			}
		}
		break;
	}

	return 0;
}

// 'allow override'
static int ProcessProtoAck(WPARAM, LPARAM lParam)
{
	// 'something' made a status change
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_STATUS && ack->result != ACKRESULT_FAILED)
		return 0;

	if (!db_get_b(0, SSMODULENAME, SETTING_OVERRIDE, 1) || startupSettings.getCount() == 0)
		return 0;

	for (int i = 0; i < startupSettings.getCount(); i++) {
		if (!mir_strcmp(ack->szModule, startupSettings[i].m_szName)) {
			startupSettings[i].m_szName = "";
			log_debugA("StartupStatus: %s overridden by ME_PROTO_ACK, status will not be set", ack->szModule);
		}
	}

	return 0;
}

static int StatusChange(WPARAM, LPARAM lParam)
{
	// change by menu
	if (!db_get_b(0, SSMODULENAME, SETTING_OVERRIDE, 1) || startupSettings.getCount() == 0)
		return 0;

	char *szProto = (char *)lParam;
	if (szProto == nullptr) { // global status change
		for (int i = 0; i < startupSettings.getCount(); i++) {
			startupSettings[i].m_szName = "";
			log_debugA("StartupStatus: all protos overridden by ME_CLIST_STATUSMODECHANGE, status will not be set");
		}
	}
	else {
		for (int i = 0; i < startupSettings.getCount(); i++) {
			if (!mir_strcmp(startupSettings[i].m_szName, szProto)) {
				startupSettings[i].m_szName = "";
				log_debugA("StartupStatus: %s overridden by ME_CLIST_STATUSMODECHANGE, status will not be set", szProto);
			}
		}
	}

	return 0;
}

static int CSStatusChangeEx(WPARAM wParam, LPARAM)
{
	// another status plugin made the change
	if (!db_get_b(0, SSMODULENAME, SETTING_OVERRIDE, 1) || startupSettings.getCount() == 0)
		return 0;

	if (wParam != 0) {
		PROTOCOLSETTINGEX** ps = *(PROTOCOLSETTINGEX***)wParam;
		if (ps == nullptr)
			return -1;

		for (int i = 0; i < startupSettings.getCount(); i++) {
			for (int j = 0; j < startupSettings.getCount(); j++) {
				if (ps[i]->m_szName == nullptr || startupSettings[j].m_szName == nullptr)
					continue;

				if (!mir_strcmp(ps[i]->m_szName, startupSettings[j].m_szName)) {
					log_debugA("StartupStatus: %s overridden by MS_CS_SETSTATUSEX, status will not be set", ps[i]->m_szName);
					// use a hack to disable this proto
					startupSettings[j].m_szName = "";
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
	CallService(MS_CS_SETSTATUSEX, (WPARAM)&startupSettings, 0);
}

static int OnOkToExit(WPARAM, LPARAM)
{
	// save last protocolstatus
	int count;
	PROTOACCOUNT** protos;
	Proto_EnumAccounts(&count, &protos);

	for (int i = 0; i < count; i++) {
		PROTOACCOUNT *pa = protos[i];
		if (!IsSuitableProto(pa))
			continue;

		if (!Proto_GetAccount(pa->szModuleName))
			continue;

		char lastName[128], lastMsg[128];
		mir_snprintf(lastName, "%s%s", PREFIX_LAST, pa->szModuleName);
		db_set_w(0, SSMODULENAME, lastName, (WORD)CallProtoService(pa->szModuleName, PS_GETSTATUS, 0, 0));
		mir_snprintf(lastMsg, "%s%s", PREFIX_LASTMSG, pa->szModuleName);
		db_unset(0, SSMODULENAME, lastMsg);

		if (!(CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG))
			continue;

		int status = CallProtoService(pa->szModuleName, PS_GETSTATUS, 0, 0);
		if (!(CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(status)))
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
				db_set_ws(0, SSMODULENAME, lastMsg, npi.tszMsg);
				mir_free(npi.tszMsg);
			}
		}
	}

	if (db_get_b(0, SSMODULENAME, SETTING_SETPROFILE, 1) || db_get_b(0, SSMODULENAME, SETTING_OFFLINECLOSE, 0))
		Clist_SetStatusMode(ID_STATUS_OFFLINE);

	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	// set windowstate and docked for next startup
	if (db_get_b(0, SSMODULENAME, SETTING_SETWINSTATE, 0)) {
		int state = db_get_b(0, SSMODULENAME, SETTING_WINSTATE, SETTING_STATE_NORMAL);
		HWND hClist = pcli->hwndContactList;
		BOOL isHidden = !IsWindowVisible(hClist);
		switch (state) {
		case SETTING_STATE_HIDDEN:
			// try to use services where possible
			if (!isHidden)
				pcli->pfnShowHide();
			break;

		case SETTING_STATE_MINIMIZED:
			if (!db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 0))
				ShowWindow(hClist, SW_SHOWMINIMIZED);
			break;

		case SETTING_STATE_NORMAL:
			// try to use services where possible (that's what they're for)
			if (isHidden)
				pcli->pfnShowHide();
			break;
		}
	}

	// hangup
	if (db_get_b(0, SSMODULENAME, SETTING_AUTOHANGUP, 0))
		InternetAutodialHangup(0);

	int state = db_get_b(0, SSMODULENAME, SETTING_WINSTATE, SETTING_STATE_NORMAL);
	// set windowstate and docked for next startup
	if (db_get_b(0, SSMODULENAME, SETTING_SETWINSTATE, 0))
		db_set_b(0, MODULE_CLIST, SETTING_WINSTATE, (BYTE)state);

	if (hMessageWindow)
		DestroyWindow(hMessageWindow);

	startupSettings.destroy();
	return 0;
}

/* Window proc for poweroff event */
static DWORD CALLBACK MessageWndProc(HWND, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_ENDSESSION:
		log_debugA("WM_ENDSESSION");
		if (wParam) {
			log_debugA("WM_ENDSESSION: calling exit");
			OnShutdown(0, 0);
			log_debugA("WM_ENDSESSION: exit called");
		}
		break;
	}

	return TRUE;
}

int SSModuleLoaded(WPARAM, LPARAM)
{
	protoList = (OBJLIST<PROTOCOLSETTINGEX>*)&startupSettings;

	InitProfileModule();

	HookEvent(ME_PROTO_ACCLISTCHANGED, OnSSAccChanged);
	HookEvent(ME_OPT_INITIALISE, StartupStatusOptionsInit);

	/* shutdown hook for normal shutdown */
	HookEvent(ME_SYSTEM_OKTOEXIT, OnOkToExit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);
	/* message window for poweroff */
	hMessageWindow = CreateWindowEx(0, L"STATIC", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
	SetWindowLongPtr(hMessageWindow, GWLP_WNDPROC, (LONG_PTR)MessageWndProc);

	GetProfile(-1, startupSettings);

	// override with cmdl
	ProcessCommandLineOptions(startupSettings);
	if (startupSettings.getCount() == 0)
		return 0;// no protocols are loaded

	SetLastStatusMessages(startupSettings);
	showDialogOnStartup = (showDialogOnStartup || db_get_b(0, SSMODULENAME, SETTING_SHOWDIALOG, 0));

	// dial
	if (showDialogOnStartup || db_get_b(0, SSMODULENAME, SETTING_SETPROFILE, 1))
		if (db_get_b(0, SSMODULENAME, SETTING_AUTODIAL, 0))
			InternetAutodial(0, nullptr);

	// set the status!
	if (showDialogOnStartup || db_get_b(0, SSMODULENAME, SETTING_SHOWDIALOG, 0))
		ShowConfirmDialogEx((TProtoSettings*)&startupSettings, db_get_dw(0, SSMODULENAME, SETTING_DLGTIMEOUT, 5));
	else if (db_get_b(0, SSMODULENAME, SETTING_SETPROFILE, 1)) {
		// set hooks for override
		if (db_get_b(0, SSMODULENAME, SETTING_OVERRIDE, 1)) {
			hProtoAckHook = HookEvent(ME_PROTO_ACK, ProcessProtoAck);
			hCSStatusChangeHook = HookEvent(ME_CS_STATUSCHANGEEX, CSStatusChangeEx);
			hStatusChangeHook = HookEvent(ME_CLIST_STATUSMODECHANGE, StatusChange);
		}
		setStatusTimerId = SetTimer(nullptr, 0, db_get_dw(0, SSMODULENAME, SETTING_SETPROFILEDELAY, 500), SetStatusTimed);
	}

	// win size and location
	if (db_get_b(0, SSMODULENAME, SETTING_SETWINLOCATION, 0) || db_get_b(0, SSMODULENAME, SETTING_SETWINSIZE, 0)) {
		HWND hClist = pcli->hwndContactList;

		// store in db
		if (db_get_b(0, SSMODULENAME, SETTING_SETWINLOCATION, 0)) {
			db_set_dw(0, MODULE_CLIST, SETTING_XPOS, db_get_dw(0, SSMODULENAME, SETTING_XPOS, 0));
			db_set_dw(0, MODULE_CLIST, SETTING_YPOS, db_get_dw(0, SSMODULENAME, SETTING_YPOS, 0));
		}
		if (db_get_b(0, SSMODULENAME, SETTING_SETWINSIZE, 0)) {
			db_set_dw(0, MODULE_CLIST, SETTING_WIDTH, db_get_dw(0, SSMODULENAME, SETTING_WIDTH, 0));
			if (!db_get_b(0, MODULE_CLUI, SETTING_AUTOSIZE, 0))
				db_set_dw(0, MODULE_CLIST, SETTING_HEIGHT, db_get_dw(0, SSMODULENAME, SETTING_HEIGHT, 0));
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
					if (db_get_b(0, SSMODULENAME, SETTING_SETWINLOCATION, 0)) {
						x = db_get_dw(0, SSMODULENAME, SETTING_XPOS, x);
						y = db_get_dw(0, SSMODULENAME, SETTING_YPOS, y);
					}
					if (db_get_b(0, SSMODULENAME, SETTING_SETWINSIZE, 0)) {
						width = db_get_dw(0, SSMODULENAME, SETTING_WIDTH, width);
						if (!db_get_b(0, MODULE_CLUI, SETTING_AUTOSIZE, 0))
							height = db_get_dw(0, SSMODULENAME, SETTING_HEIGHT, height);
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
	return GetProfile((int)wParam, *(TSettingsList*)lParam);
}

void StartupStatusLoad()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, SSModuleLoaded);

	if (db_get_b(0, SSMODULENAME, SETTING_SETPROFILE, 1) || db_get_b(0, SSMODULENAME, SETTING_OFFLINECLOSE, 0))
		db_set_w(0, "CList", "Status", (WORD)ID_STATUS_OFFLINE);

	// docking
	if (db_get_b(0, SSMODULENAME, SETTING_SETDOCKED, 0)) {
		int docked = db_get_b(0, SSMODULENAME, SETTING_DOCKED, DOCKED_NONE);
		if (docked == DOCKED_LEFT || docked == DOCKED_RIGHT)
			docked = -docked;

		db_set_b(0, MODULE_CLIST, SETTING_DOCKED, (BYTE)docked);
	}

	// Create service functions; the get functions are created here; they don't rely on commonstatus
	CreateServiceFunction(MS_SS_GETPROFILE, SrvGetProfile);
	CreateServiceFunction(MS_SS_GETPROFILECOUNT, GetProfileCount);
	CreateServiceFunction(MS_SS_GETPROFILENAME, GetProfileName);

	LoadProfileModule();
}

void StartupStatusUnload()
{
	DeinitProfilesModule();
}
