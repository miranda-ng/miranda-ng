/*
	KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
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

struct TimerInfo
{
	int timer;
	int timeout;
	BOOL restart;
	int result;
	HANDLE hEvent;
};

HANDLE hMainThread = 0;
unsigned long mainThreadId = 0;

HANDLE hConnectionEvent = nullptr;

static mir_cs GenTimerCS, GenStatusCS, CheckContinueslyCS;

static HANDLE hProtoAckHook = nullptr;
static HANDLE hStatusChangeHook = nullptr;
static HANDLE hCSStatusChangeHook = nullptr;
static HANDLE hCSStatusChangeExHook = nullptr;

static HWND hMessageWindow = nullptr;

static int CompareConnections(const TKSSettings *p1, const TKSSettings *p2)
{
	return mir_strcmp(p1->m_szName, p2->m_szName);
}

static OBJLIST<TKSSettings> connectionSettings(10, CompareConnections);

static UINT_PTR checkConnectionTimerId = 0;
static UINT_PTR afterCheckTimerId = 0;
static UINT_PTR processAckTimerId = 0;
static UINT_PTR checkContinTimerId = 0;
static UINT_PTR checkConnectingTimerId = 0;
static int retryCount = 0;
static BOOL bLastPingResult = TRUE;
// variables (options)
static int maxRetries = 0;
static int initDelay = 0;
static int currentDelay = 0;
static int maxDelay = 0;
static int ackDelay = 500;
static int increaseExponential = 0;
static int showConnectionPopups = 0;
// prototypes
static int StartTimer(int timer, int timeout, BOOL restart);
static int StopTimer(int timer);
static void GetCurrentConnectionSettings();
static int ProcessProtoAck(WPARAM wParam, LPARAM lParam);
static VOID CALLBACK CheckConnectingTimer(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime);
static VOID CALLBACK CheckAckStatusTimer(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime);
static int StatusChange(WPARAM wParam, LPARAM lParam);
static int CSStatusChange(WPARAM wParam, LPARAM lParam);
static int CSStatusChangeEx(WPARAM wParam, LPARAM lParam);
static VOID CALLBACK CheckConnectionTimer(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime);
static int StopChecking();
static VOID CALLBACK AfterCheckTimer(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime);
static VOID CALLBACK CheckContinueslyTimer(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime);
INT_PTR IsProtocolEnabledService(WPARAM wParam, LPARAM lParam);

static int ProcessPopup(int reason, LPARAM lParam);
static INT_PTR ShowPopup(wchar_t *msg, HICON hIcon);
LRESULT CALLBACK KSPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static DWORD CALLBACK MessageWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// options.c
extern int KeepStatusOptionsInit(WPARAM wparam, LPARAM);

TKSSettings::TKSSettings(PROTOACCOUNT *pa)
{
	m_szName = pa->szModuleName;
	m_tszAccName = pa->tszAccountName;
	m_szMsg = nullptr;
	m_status = m_lastStatus = CallProtoService(pa->szModuleName, PS_GETSTATUS, 0, 0);
}

TKSSettings::~TKSSettings()
{
	if (m_szMsg != nullptr)
		free(m_szMsg);
}

int KSLoadOptions()
{
	UnhookEvent(hProtoAckHook);
	UnhookEvent(hStatusChangeHook);
	UnhookEvent(hCSStatusChangeHook);
	UnhookEvent(hCSStatusChangeExHook);
	hProtoAckHook = hStatusChangeHook = hCSStatusChangeHook = hCSStatusChangeExHook = 0;

	if (IsWindow(hMessageWindow))
		DestroyWindow(hMessageWindow);

	if (StartTimer(IDT_CHECKCONTIN, -1, FALSE))
		WSACleanup();

	StopTimer(IDT_CHECKCONN | IDT_PROCESSACK | IDT_AFTERCHECK | IDT_CHECKCONTIN | IDT_CHECKCONNECTING);

	GetCurrentConnectionSettings();

	if (db_get_b(0, KSMODULENAME, SETTING_CHECKCONNECTION, FALSE)) {
		if (db_get_b(0, KSMODULENAME, SETTING_CONTCHECK, FALSE)) {
			if (db_get_b(0, KSMODULENAME, SETTING_BYPING, FALSE)) {
				WSADATA wsaData;
				WSAStartup(MAKEWORD(2, 2), &wsaData);
			}
			StartTimer(IDT_CHECKCONTIN, 0, FALSE);
		}
		increaseExponential = db_get_b(0, KSMODULENAME, SETTING_INCREASEEXPONENTIAL, FALSE);
		currentDelay = initDelay = 1000 * db_get_dw(0, KSMODULENAME, SETTING_INITDELAY, DEFAULT_INITDELAY);
		maxDelay = 1000 * db_get_dw(0, KSMODULENAME, SETTING_MAXDELAY, DEFAULT_MAXDELAY);
		maxRetries = db_get_b(0, KSMODULENAME, SETTING_MAXRETRIES, 0);
		if (maxRetries == 0)
			maxRetries = -1;
		hProtoAckHook = HookEvent(ME_PROTO_ACK, ProcessProtoAck);
		hStatusChangeHook = HookEvent(ME_CLIST_STATUSMODECHANGE, StatusChange);
		if (ServiceExists(ME_CS_STATUSCHANGE))
			hCSStatusChangeHook = HookEvent(ME_CS_STATUSCHANGE, CSStatusChange);
		hCSStatusChangeExHook = HookEvent(ME_CS_STATUSCHANGEEX, CSStatusChangeEx);
		if (db_get_b(0, KSMODULENAME, SETTING_CHECKAPMRESUME, 0)) {
			if (!IsWindow(hMessageWindow)) {
				hMessageWindow = CreateWindowEx(0, L"STATIC", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
				SetWindowLongPtr(hMessageWindow, GWLP_WNDPROC, (LONG_PTR)MessageWndProc);
			}
		}
		retryCount = 0;
	}

	return 0;
}

static void GetCurrentConnectionSettings()
{
	connectionSettings.destroy();

	int count;
	PROTOACCOUNT** protos;
	Proto_EnumAccounts(&count, &protos);

	for (int i = 0; i < count; i++)
		if (IsSuitableProto(protos[i]))
			connectionSettings.insert(new TKSSettings(protos[i]));
}

static PROTOCOLSETTINGEX** GetCurrentProtoSettingsCopy()
{
	mir_cslock lck(GenStatusCS);
	PROTOCOLSETTINGEX **ps = (PROTOCOLSETTINGEX**)malloc(connectionSettings.getCount() * sizeof(PROTOCOLSETTINGEX *));
	if (ps == nullptr) {
		return nullptr;
	}
	for (int i = 0; i < connectionSettings.getCount(); i++) {
		ps[i] = (PROTOCOLSETTINGEX*)calloc(1, sizeof(PROTOCOLSETTINGEX));
		if (ps[i] == nullptr) {
			free(ps);
			return nullptr;
		}

		TKSSettings &cs = connectionSettings[i];
		ps[i]->m_lastStatus = cs.m_lastStatus;
		ps[i]->m_status = cs.m_status;
		ps[i]->m_szMsg = nullptr;
		ps[i]->m_szName = cs.m_szName;
		ps[i]->m_tszAccName = cs.m_tszAccName;
	}

	return ps;
}

static void FreeProtoSettings(PROTOCOLSETTINGEX** ps)
{
	for (int i = 0; i < connectionSettings.getCount(); i++) {
		if (ps[i]->m_szMsg != nullptr)
			free(ps[i]->m_szMsg);
		free(ps[i]);
	}
	free(ps);
}

int TKSSettings::AssignStatus(int iStatus, int iLastStatus, wchar_t *pwszMsg)
{
	if (iStatus < MIN_STATUS || iStatus > MAX_STATUS)
		return -1;
	if (iStatus != ID_STATUS_OFFLINE && m_status == ID_STATUS_DISABLED)
		return -2;
	if (!db_get_b(0, KSMODULENAME, SETTING_NOLOCKED, 0) && db_get_b(0, m_szName, "LockMainStatus", 0))
		return -3;

	mir_cslock lck(GenStatusCS);

	char dbSetting[128];
	mir_snprintf(dbSetting, "%s_enabled", m_szName);
	m_lastStatus = (iLastStatus == 0) ? m_status : iLastStatus;
	if (!db_get_b(0, KSMODULENAME, dbSetting, 1))
		m_status = ID_STATUS_DISABLED;
	else if (iStatus == ID_STATUS_LAST)
		m_status = m_lastStatus;
	else
		m_status = iStatus;

	log_infoA("KeepStatus: assigning status %d (%d, %d) to %s", m_status, m_lastStatus, iLastStatus, m_szName);

	if (pwszMsg != nullptr && mir_wstrcmp(pwszMsg, m_szMsg)) {
		if (m_szMsg != nullptr)
			free(m_szMsg);

		m_szMsg = wcsdup(pwszMsg);
	}
	else if (pwszMsg != m_szMsg) {
		if (m_szMsg != nullptr)
			free(m_szMsg);

		m_szMsg = nullptr;
	}
	return 0;
}

int TKSSettings::GetStatus() const 
{
	switch (m_status) {
	case ID_STATUS_CURRENT:
		return CallProtoService(m_szName, PS_GETSTATUS, 0, 0);
	case ID_STATUS_LAST:
		return m_lastStatus;
	default:
		return m_status;
	}
}

static int SetCurrentStatus()
{
	PROTOCOLSETTINGEX **ps = GetCurrentProtoSettingsCopy();
	for (int i = 0; i < connectionSettings.getCount(); i++) {
		auto p = ps[i];
		int realStatus = CallProtoService(p->m_szName, PS_GETSTATUS, 0, 0);
		int curStatus = connectionSettings[i].GetStatus();
		if (curStatus == ID_STATUS_DISABLED || curStatus == realStatus) { // ignore this proto by removing it's name (not so nice)
			p->m_szName = "";
		}
		else {
			log_infoA("KeepStatus: status for %s differs: stored = %d, real = %d", p->m_szName, curStatus, realStatus);

			// force offline before reconnecting?
			if (realStatus != ID_STATUS_OFFLINE && db_get_b(0, KSMODULENAME, SETTING_FIRSTOFFLINE, FALSE)) {
				log_infoA("KeepStatus: Setting %s offline before making a new connection attempt", p->m_szName);
				CallProtoService(p->m_szName, PS_SETSTATUS, (WPARAM)ID_STATUS_OFFLINE, 0);
			}
		}
	}
	ProcessPopup(KS_CONN_STATE_RETRY, (LPARAM)ps);
	INT_PTR ret = CallService(MS_CS_SETSTATUSEX, (WPARAM)&ps, 0);
	FreeProtoSettings(ps);

	return ret;
}

static int StatusChange(WPARAM wParam, LPARAM lParam)
{
	char *szProto = (char *)lParam;
	if (szProto == nullptr) { // global status change
		for (int i = 0; i < connectionSettings.getCount(); i++) {
			TKSSettings &cs = connectionSettings[i];
			cs.AssignStatus(wParam, 0, cs.m_szMsg);
		}
	}
	else {
		for (int i = 0; i < connectionSettings.getCount(); i++) {
			TKSSettings &cs = connectionSettings[i];
			if (!mir_strcmp(cs.m_szName, szProto))
				cs.AssignStatus(wParam, 0, cs.m_szMsg);
		}
	}

	return 0;
}

static int CSStatusChange(WPARAM wParam, LPARAM)
{
	// the status was changed by commonstatus (old)
	if (wParam != 0) {
		PROTOCOLSETTING **protoSettings = *(PROTOCOLSETTING***)wParam;
		if (protoSettings == nullptr)
			return -1;

		for (int i = 0; i < connectionSettings.getCount(); i++) {
			auto psi = protoSettings[i];
			if (psi->szName == nullptr)
				continue;

			for (int j = 0; j < connectionSettings.getCount(); j++) {
				TKSSettings &cs = connectionSettings[i];
				if (cs.m_szName == nullptr)
					continue;

				if (!mir_strcmp(psi->szName, cs.m_szName))
					cs.AssignStatus(psi->status, psi->lastStatus, cs.m_szMsg);
			}
		}
	}

	return 0;
}

static int CSStatusChangeEx(WPARAM wParam, LPARAM)
{
	// the status was changed by commonstatus (new)
	if (wParam != 0) {
		PROTOCOLSETTINGEX** protoSettings = *(PROTOCOLSETTINGEX***)wParam;
		if (protoSettings == nullptr)
			return -1;

		for (int i = 0; i < connectionSettings.getCount(); i++) {
			auto psi = protoSettings[i];
			if (psi->m_szName == nullptr)
				continue;

			for (int j = 0; j < connectionSettings.getCount(); j++) {
				TKSSettings &cs = connectionSettings[i];
				if (cs.m_szName == nullptr)
					continue;

				if (!mir_strcmp(psi->m_szName, cs.m_szName))
					cs.AssignStatus(psi->m_status, psi->m_lastStatus, psi->m_szMsg);
			}
		}
	}

	return 0;
}

static int StartTimerFunction(int timer, int timeout, BOOL restart)
{
	int res = 0;

	mir_cslock lck(GenTimerCS);
	log_debugA("StartTimer: %d, %d, %d", timer, timeout, restart);
	log_debugA("ack: %u, chk: %u, aft: %u, cnt: %u, con: %u", processAckTimerId, checkConnectionTimerId, afterCheckTimerId, checkContinTimerId, checkConnectingTimerId);
	if (timer & IDT_PROCESSACK) {
		res = (processAckTimerId == 0) ? 0 : 1;
		if (((processAckTimerId == 0) && (checkConnectionTimerId == 0)) || (restart)) {
			if (timeout != -1) {
				if (restart)
					KillTimer(nullptr, processAckTimerId);
				if (timeout == 0)
					processAckTimerId = SetTimer(nullptr, 0, ackDelay, CheckAckStatusTimer);
				else
					processAckTimerId = SetTimer(nullptr, 0, timeout, CheckAckStatusTimer);
			}
		}
	}

	if (timer & IDT_CHECKCONN) {
		res = (checkConnectionTimerId == 0 ? 0 : 1) || res;
		if ((checkConnectionTimerId == 0) || (restart)) {
			if (timeout != -1) {
				if (restart)
					KillTimer(nullptr, checkConnectionTimerId);
				if (timeout == 0)
					checkConnectionTimerId = SetTimer(nullptr, 0, initDelay, CheckConnectionTimer);
				else
					checkConnectionTimerId = SetTimer(nullptr, 0, timeout, CheckConnectionTimer);
			}
		}
	}

	if (timer & IDT_AFTERCHECK) {
		res = (afterCheckTimerId == 0 ? 0 : 1) || res;
		if ((afterCheckTimerId == 0) || (restart)) {
			if (timeout != -1) {
				if (restart)
					KillTimer(nullptr, afterCheckTimerId);
				if (timeout == 0)
					afterCheckTimerId = SetTimer(nullptr, 0, initDelay / 2, AfterCheckTimer);
				else
					afterCheckTimerId = SetTimer(nullptr, 0, timeout, AfterCheckTimer);
			}
		}
	}

	if (timer & IDT_CHECKCONTIN) {
		res = (checkContinTimerId == 0 ? 0 : 1) || res;
		if ((checkContinTimerId == 0) || (restart)) {
			if (timeout != -1) {
				if (restart)
					KillTimer(nullptr, checkContinTimerId);
				if (timeout == 0) {
					checkContinTimerId = SetTimer(nullptr, 0, 1000 * db_get_dw(0, KSMODULENAME, SETTING_CNTDELAY, CHECKCONTIN_DELAY), CheckContinueslyTimer);
				}
				else
					checkContinTimerId = SetTimer(nullptr, 0, timeout, CheckContinueslyTimer);
			}
		}
	}

	if (timer & IDT_CHECKCONNECTING) {
		res = (checkConnectingTimerId == 0 ? 0 : 1) || res;
		if ((checkConnectingTimerId == 0) || (restart)) {
			if (timeout != -1) {
				if (restart)
					KillTimer(nullptr, checkConnectingTimerId);
				if (timeout == 0) {
					timeout = initDelay / 2;
				}
				checkConnectingTimerId = SetTimer(nullptr, 0, timeout, CheckConnectingTimer);
			}
		}
	}

	log_debugA("ack: %u, chk: %u, aft: %u, cnt: %u, con: %u", processAckTimerId, checkConnectionTimerId, afterCheckTimerId, checkContinTimerId, checkConnectingTimerId);
	log_debugA("StartTimer done %d", res);
	return res;
}

static VOID CALLBACK StartTimerApcProc(ULONG_PTR param)
{
	struct TimerInfo *ti = (struct TimerInfo *)param;
	log_debugA("StartTimerApcProc %d %d %d", ti->timer, ti->timeout, ti->restart);
	ti->result = StartTimerFunction(ti->timer, ti->timeout, ti->restart);
	SetEvent(ti->hEvent);
}

static int StartTimer(int timer, int timeout, BOOL restart)
{
	if (GetCurrentThreadId() == mainThreadId)
		return StartTimerFunction(timer, timeout, restart);

	TimerInfo *ti = (TimerInfo*)calloc(1, sizeof(struct TimerInfo));
	ti->timer = timer;
	ti->timeout = timeout;
	ti->restart = restart;
	ti->hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	QueueUserAPC(StartTimerApcProc, hMainThread, (ULONG_PTR)ti);
	WaitForSingleObject(ti->hEvent, INFINITE);
	CloseHandle(ti->hEvent);
	int res = ti->result;
	free(ti);
	return res;
}

static int StopTimer(int timer)
{
	int res = 0;

	mir_cslock lck(GenTimerCS);
	log_debugA("StopTimer %d", timer);
	log_debugA("ack: %u, chk: %u, aft: %u, cnt: %u, con: %u", processAckTimerId, checkConnectionTimerId, afterCheckTimerId, checkContinTimerId, checkConnectingTimerId);

	if (timer & IDT_PROCESSACK) {
		if (processAckTimerId == 0)
			res = 0;
		else {
			KillTimer(nullptr, processAckTimerId);
			processAckTimerId = 0;
			res = 1;
		}
	}

	if (timer & IDT_CHECKCONN) {
		if (checkConnectionTimerId == 0)
			res = 0 || res;
		else {
			KillTimer(nullptr, checkConnectionTimerId);
			checkConnectionTimerId = 0;
			res = 1;
		}
	}

	if (timer & IDT_AFTERCHECK) {
		if (afterCheckTimerId == 0)
			res = 0 || res;
		else {
			KillTimer(nullptr, afterCheckTimerId);
			afterCheckTimerId = 0;
			res = 1;
		}
	}

	if (timer & IDT_CHECKCONTIN) {
		if (checkContinTimerId == 0)
			res = 0 || res;
		else {
			KillTimer(nullptr, checkContinTimerId);
			checkContinTimerId = 0;
			res = 1;
		}
	}

	if (timer & IDT_CHECKCONNECTING) {
		if (checkConnectingTimerId == 0)
			res = 0 || res;
		else {
			KillTimer(nullptr, checkConnectingTimerId);
			checkConnectingTimerId = 0;
			res = 1;
		}
	}

	log_debugA("ack: %u, chk: %u, aft: %u, cnt: %u, con: %u", processAckTimerId, checkConnectionTimerId, afterCheckTimerId, checkContinTimerId, checkConnectingTimerId);
	log_debugA("StopTimer done %d", res);
	return res;
}

static int ProcessProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_STATUS && ack->type != ACKTYPE_LOGIN)
		return 0;

	char dbSetting[128];
	mir_snprintf(dbSetting, "%s_enabled", ack->szModule);
	if (!db_get_b(0, KSMODULENAME, dbSetting, 1))
		return 0;

	if (ack->type == ACKTYPE_STATUS && ack->result == ACKRESULT_SUCCESS) {
		for (int i = 0; i < connectionSettings.getCount(); i++) {
			TKSSettings &cs = connectionSettings[i];
			if (!mir_strcmp(cs.m_szName, ack->szModule))
				cs.lastStatusAckTime = GetTickCount();
		}
		StartTimer(IDT_PROCESSACK, 0, FALSE);
		return 0;
	}

	if (ack->type == ACKTYPE_LOGIN) {
		if (ack->lParam == LOGINERR_OTHERLOCATION) {
			for (int i = 0; i < connectionSettings.getCount(); i++) {
				TKSSettings &cs = connectionSettings[i];
				if (!mir_strcmp(ack->szModule, cs.m_szName)) {
					cs.AssignStatus(ID_STATUS_OFFLINE);
					if (db_get_b(0, KSMODULENAME, SETTING_CNCOTHERLOC, 0)) {
						StopTimer(IDT_PROCESSACK);
						for (int j = 0; j < connectionSettings.getCount(); j++)
							connectionSettings[j].AssignStatus(ID_STATUS_OFFLINE);
					}

					NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_OTHERLOCATION, (LPARAM)cs.m_szName);
					ProcessPopup(KS_CONN_STATE_OTHERLOCATION, (LPARAM)ack->szModule);
				}
			}
		}
		else if (ack->result == ACKRESULT_FAILED) {
			// login failed
			NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_LOGINERROR, (LPARAM)ack->szModule);
			
			switch (db_get_b(0, KSMODULENAME, SETTING_LOGINERR, LOGINERR_NOTHING)) {
			case LOGINERR_CANCEL:
				log_infoA("KeepStatus: cancel on login error (%s)", ack->szModule);
				for (int i = 0; i < connectionSettings.getCount(); i++) {
					TKSSettings &cs = connectionSettings[i];
					if (!mir_strcmp(ack->szModule, cs.m_szName))
						cs.AssignStatus(ID_STATUS_OFFLINE);
				}
				ProcessPopup(KS_CONN_STATE_LOGINERROR, (LPARAM)ack->szModule);
				StopChecking();
				break;

			case LOGINERR_SETDELAY:
				{
					int newDelay = 1000 * db_get_dw(0, KSMODULENAME, SETTING_LOGINERR_DELAY, DEFAULT_MAXDELAY);
					log_infoA("KeepStatus: set delay to %d ms on login error (%s)", newDelay, ack->szModule);
					StartTimer(IDT_CHECKCONN, newDelay, TRUE);
				}
				ProcessPopup(KS_CONN_STATE_LOGINERROR, (LPARAM)ack->szModule);
				break;

			default:
			case LOGINERR_NOTHING:
				StartTimer(IDT_PROCESSACK, 0, FALSE);
				break;
			}
		}
	}

	return 0;
}

static VOID CALLBACK CheckConnectingTimer(HWND, UINT, UINT_PTR, DWORD)
{
	StopTimer(IDT_CHECKCONNECTING);

	for (int i = 0; i < connectionSettings.getCount(); i++) {
		TKSSettings &cs = connectionSettings[i];

		int curStatus = cs.GetStatus();
		if (IsStatusConnecting(curStatus)) { // connecting
			int maxConnectingTime = db_get_dw(0, KSMODULENAME, SETTING_MAXCONNECTINGTIME, 0);
			if (maxConnectingTime > 0) {
				if ((unsigned int)maxConnectingTime <= ((GetTickCount() - cs.lastStatusAckTime) / 1000)) {
					// set offline
					log_infoA("KeepStatus: %s is too long connecting; setting offline", cs.m_szName);
					CallProtoService(cs.m_szName, PS_SETSTATUS, (WPARAM)ID_STATUS_OFFLINE, 0);
				}
			}
		}
	}
}

static VOID CALLBACK CheckAckStatusTimer(HWND, UINT, UINT_PTR, DWORD)
{
	bool needChecking = false;

	StopTimer(IDT_PROCESSACK);
	for (int i = 0; i < connectionSettings.getCount(); i++) {
		TKSSettings &cs = connectionSettings[i];

		int curStatus = cs.GetStatus();
		int newStatus = CallProtoService(cs.m_szName, PS_GETSTATUS, 0, 0);
		// ok, np
		if (curStatus == ID_STATUS_CURRENT || curStatus == ID_STATUS_DISABLED || curStatus == newStatus || newStatus > MAX_STATUS)
			continue;

		if (IsStatusConnecting(newStatus)) { // connecting
			int maxConnectingTime = db_get_dw(0, KSMODULENAME, SETTING_MAXCONNECTINGTIME, 0);
			if (maxConnectingTime > 0)
				StartTimer(IDT_CHECKCONNECTING, (maxConnectingTime * 1000 - (GetTickCount() - cs.lastStatusAckTime)), FALSE);
		}
		// keepstatus' administration was wrong!
		else if (newStatus != ID_STATUS_OFFLINE)
			cs.AssignStatus(newStatus);

		// connection lost
		else if (newStatus == ID_STATUS_OFFLINE) {// start checking connection
			if (!StartTimer(IDT_CHECKCONN, -1, FALSE)) { /* check if not already checking */
				needChecking = true;
				log_infoA("KeepStatus: connection lost! (%s)", cs.m_szName);
				NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_LOST, (LPARAM)cs.m_szName);
				ProcessPopup(KS_CONN_STATE_LOST, (LPARAM)cs.m_szName);
			}
		}
	}

	if (needChecking)
		StartTimer(IDT_CHECKCONN, initDelay, FALSE);
}

static VOID CALLBACK CheckConnectionTimer(HWND, UINT, UINT_PTR, DWORD)
{
	log_debugA("CheckConnectionTimer");
	bool setStatus = false;

	for (int i = 0; i < connectionSettings.getCount() && !setStatus; i++) {
		TKSSettings &cs = connectionSettings[i];
		int realStatus = CallProtoService(cs.m_szName, PS_GETSTATUS, 0, 0);
		int shouldBeStatus = cs.GetStatus();
		if (shouldBeStatus == ID_STATUS_LAST)
			shouldBeStatus = cs.m_lastStatus;
		if (shouldBeStatus == ID_STATUS_DISABLED)
			continue;
		if ((shouldBeStatus != realStatus) && (realStatus == ID_STATUS_OFFLINE) || (realStatus < MIN_STATUS))
			setStatus = true;
	}

	// one of the status was wrong
	if (setStatus && (maxRetries == -1 || retryCount < maxRetries)) {
		if (increaseExponential)
			currentDelay = min(2 * currentDelay, maxDelay);

		if (((db_get_b(0, KSMODULENAME, SETTING_CHKINET, 0)) && (!InternetGetConnectedState(nullptr, 0))) || ((db_get_b(0, KSMODULENAME, SETTING_BYPING, FALSE)) && (!bLastPingResult))) {
			// no network
			NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_RETRYNOCONN, (LPARAM)retryCount + 1);
			ProcessPopup(KS_CONN_STATE_RETRYNOCONN, 0);
		}
		else {
			NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_RETRY, (LPARAM)retryCount + 1);
			SetCurrentStatus(); // set the status
		}
		retryCount += 1;
		StartTimer(IDT_AFTERCHECK, min(currentDelay, AFTERCHECK_DELAY) / 2, FALSE);
		StartTimer(IDT_CHECKCONN, currentDelay, TRUE); // restart this timer
	}
	else // all status set ok already, or stop checking
		StopChecking();

	log_debugA("CheckConnectionTimer done");
}

static int StopChecking()
{
	StopTimer(IDT_CHECKCONN | IDT_PROCESSACK | IDT_AFTERCHECK | IDT_CHECKCONNECTING);

	BOOL isOk = TRUE;
	for (int i = 0; i < connectionSettings.getCount() && isOk; i++) {
		TKSSettings &cs = connectionSettings[i];
		int curStatus = cs.GetStatus();
		int newStatus = CallProtoService(cs.m_szName, PS_GETSTATUS, 0, 0);
		if (newStatus != curStatus) {
			cs.AssignStatus(newStatus);
			isOk = FALSE;
		}
	}

	NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_STOPPEDCHECKING, (LPARAM)isOk);
	ProcessPopup(KS_CONN_STATE_STOPPEDCHECKING, (LPARAM)isOk);
	log_infoA("KeepStatus: stop checking (%s)", isOk ? "success" : "failure");
	retryCount = 0;
	currentDelay = initDelay;

	return 0;
}

static VOID CALLBACK AfterCheckTimer(HWND, UINT, UINT_PTR, DWORD)
{
	// after each connection check, this function is called to see if connection was recovered
	StopTimer(IDT_AFTERCHECK);

	bool setStatus = false;

	for (int i = 0; i < connectionSettings.getCount(); i++) {
		TKSSettings &cs = connectionSettings[i];
		int realStatus = CallProtoService(cs.m_szName, PS_GETSTATUS, 0, 0);
		int shouldBeStatus = cs.GetStatus();
		if (shouldBeStatus == ID_STATUS_LAST) // this should never happen
			shouldBeStatus = cs.m_lastStatus;
		if (shouldBeStatus == ID_STATUS_DISABLED) //  (on ignoring proto)
			continue;
		if ((shouldBeStatus != realStatus) && (realStatus == ID_STATUS_OFFLINE) || (realStatus < MIN_STATUS))
			setStatus = true;
	}

	if (!setStatus || retryCount == maxRetries)
		StopChecking();
}

static void CheckContinueslyFunction(void *)
{
	Thread_SetName("KeepStatus: CheckContinueslyFunction");

	static int pingFailures = 0;

	// one at the time is enough, do it the 'easy' way
	mir_cslock lck(CheckContinueslyCS);

	// do a ping, even if reconnecting
	bool doPing = false;
	for (int i = 0; i < connectionSettings.getCount(); i++) {
		TKSSettings &cs = connectionSettings[i];
		int shouldBeStatus = cs.GetStatus();
		if (shouldBeStatus == ID_STATUS_LAST)
			shouldBeStatus = cs.m_lastStatus;

		if (shouldBeStatus == ID_STATUS_DISABLED)
			continue;

		if (shouldBeStatus != ID_STATUS_OFFLINE) {
			log_debugA("CheckContinueslyFunction: %s should be %d", cs.m_szName, shouldBeStatus);
			doPing = true;
		}
	}

	if (!doPing) {
		log_debugA("CheckContinueslyFunction: All protocols should be offline, no need to check connection");
		return;
	}

	BOOL ping = db_get_b(0, KSMODULENAME, SETTING_BYPING, FALSE);
	if (ping) {
		DBVARIANT dbv;
		if (db_get(0, KSMODULENAME, SETTING_PINGHOST, &dbv))
			ping = FALSE;
		else {
			char *start, *end;
			char host[MAX_PATH];
			DWORD *addr;
			struct hostent *hostent;
			char reply[sizeof(ICMP_ECHO_REPLY) + 8];

			bLastPingResult = FALSE;
			HANDLE hICMPFile = (HANDLE)IcmpCreateFile();
			if (hICMPFile == INVALID_HANDLE_VALUE) {
				bLastPingResult = TRUE;
				log_infoA("KeepStatus: icmp.dll error (2)");
			}
			if (bLastPingResult == FALSE) {
				start = dbv.pszVal;
				while ((*start != '\0') && (!bLastPingResult)) {
					end = start;
					while ((*end != ' ') && (*end != '\0'))
						end++;
					memset(host, '\0', sizeof(host));
					strncpy(host, start, end - start);
					hostent = gethostbyname(host);
					if (hostent != nullptr) {
						addr = (DWORD *)(*hostent->h_addr_list);
						bLastPingResult = (IcmpSendEcho(hICMPFile, *addr, 0, 0, nullptr, reply, sizeof(ICMP_ECHO_REPLY) + 8, 5000) != 0);

						if (bLastPingResult)
							pingFailures = 0;
						else
							pingFailures++;

						log_debugA("CheckContinueslyFunction: pinging %s (result: %d/%d)", host, bLastPingResult, pingFailures);
					}
					else log_debugA("CheckContinueslyFunction: unable to resolve %s", host);

					start = end;
					while (*start == ' ')
						start++;
				}
			}
			IcmpCloseHandle(hICMPFile);
		}
		db_free(&dbv);
	}

	if (StartTimer(IDT_CHECKCONN, -1, FALSE)) // already connecting, leave
		return;

	if (((!ping) && (!InternetGetConnectedState(nullptr, 0))) || ((ping) && (!bLastPingResult) && (pingFailures >= db_get_w(0, KSMODULENAME, SETTING_PINGCOUNT, DEFAULT_PINGCOUNT)))) {
		pingFailures = 0;

		int count;
		PROTOACCOUNT** protos;
		Proto_EnumAccounts(&count, &protos);

		for (int i = 0; i < count; i++) {
			if (!IsSuitableProto(protos[i]))
				continue;

			if (IsStatusConnecting(CallProtoService(protos[i]->szModuleName, PS_GETSTATUS, 0, 0))) {
				log_debugA("CheckContinueslyFunction: %s is connecting", protos[i]->szModuleName);
				continue; // connecting, leave alone
			}
			if (IsProtocolEnabledService(0, (LPARAM)protos[i]->szModuleName)) {
				log_debugA("CheckContinueslyFunction: forcing %s offline", protos[i]->szModuleName);
				CallProtoService(protos[i]->szModuleName, PS_SETSTATUS, (WPARAM)ID_STATUS_OFFLINE, 0);
			}
		}
		if (StartTimer(IDT_CHECKCONN | IDT_PROCESSACK, -1, FALSE)) {// are our 'set offlines' noticed?
			log_debugA("CheckContinueslyFunction: currently checking");
			return;
		}
		log_infoA("KeepStatus: connection lost! (continuesly check)");
		NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_LOST, 0);
		ProcessPopup(KS_CONN_STATE_LOST, 0);
		maxRetries = db_get_b(0, KSMODULENAME, SETTING_MAXRETRIES, 0);
		if (maxRetries == 0)
			maxRetries = -1;
		StartTimer(IDT_CHECKCONN, initDelay, FALSE);
	}
}

static VOID CALLBACK CheckContinueslyTimer(HWND, UINT, UINT_PTR, DWORD)
{
	if (db_get_b(0, KSMODULENAME, SETTING_BYPING, FALSE))
		mir_forkthread(CheckContinueslyFunction, nullptr);
	else
		CheckContinueslyFunction(nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////
// popup

static wchar_t* GetHumanName(LPARAM lParam)
{
	PROTOACCOUNT *ProtoAccount = Proto_GetAccount((char*)lParam);
	return (ProtoAccount != nullptr) ? ProtoAccount->tszAccountName : TranslateT("Protocol");
}

static int ProcessPopup(int reason, LPARAM lParam)
{
	HICON hIcon = nullptr;
	wchar_t text[MAX_SECONDLINE];

	if (!db_get_b(0, KSMODULENAME, SETTING_SHOWCONNECTIONPOPUPS, FALSE) || !ServiceExists(MS_POPUP_ADDPOPUPT))
		return -1;

	switch (reason) {
	case KS_CONN_STATE_OTHERLOCATION: // lParam = 1 proto
		if (!db_get_b(0, KSMODULENAME, SETTING_PUOTHER, TRUE))
			return -1;

		hIcon = Skin_LoadProtoIcon((char*)lParam, SKINICON_STATUS_OFFLINE);
		mir_snwprintf(text, TranslateT("%s connected from another location"), GetHumanName(lParam));
		break;

	case KS_CONN_STATE_LOGINERROR:	// lParam = 1 proto
		if (!db_get_b(0, KSMODULENAME, SETTING_PUOTHER, TRUE))
			return -1;

		hIcon = Skin_LoadProtoIcon((char*)lParam, SKINICON_STATUS_OFFLINE);
		if (db_get_b(0, KSMODULENAME, SETTING_LOGINERR, LOGINERR_NOTHING) == LOGINERR_CANCEL)
			mir_snwprintf(text, TranslateT("%s login error, cancel reconnecting"), GetHumanName(lParam));
		else if (db_get_b(0, KSMODULENAME, SETTING_LOGINERR, LOGINERR_NOTHING) == LOGINERR_SETDELAY)
			mir_snwprintf(text, TranslateT("%s login error (next retry (%d) in %d s)"), GetHumanName(lParam), retryCount + 1, db_get_dw(0, KSMODULENAME, SETTING_LOGINERR_DELAY, DEFAULT_MAXDELAY));
		else
			return -1;
		break;

	case KS_CONN_STATE_LOST: // lParam = 1 proto, or nullptr
		if (!db_get_b(0, KSMODULENAME, SETTING_PUCONNLOST, TRUE))
			return -1;

		if (lParam) { // ”казатель на им€ модул€. 
			hIcon = Skin_LoadProtoIcon((char*)lParam, SKINICON_STATUS_OFFLINE);
			mir_snwprintf(text, TranslateT("%s status error (next retry (%d) in %d s)"), GetHumanName(lParam), retryCount + 1, currentDelay / 1000);
		}
		else mir_snwprintf(text, TranslateT("Status error (next retry (%d) in %d s)"), retryCount + 1, currentDelay / 1000);
		break;

	case KS_CONN_STATE_RETRY:  // lParam = PROTOCOLSETTINGEX**
		if (!db_get_b(0, KSMODULENAME, SETTING_PUCONNRETRY, TRUE))
			return -1;
		
		if (lParam) {
			PROTOCOLSETTINGEX **ps = (PROTOCOLSETTINGEX **)lParam;
			wchar_t protoInfoLine[512], protoInfo[MAX_SECONDLINE];
			memset(protoInfoLine, '\0', sizeof(protoInfoLine));
			memset(protoInfo, '\0', sizeof(protoInfo));
			mir_wstrcpy(protoInfo, L"\r\n");
			for (int i = 0; i < connectionSettings.getCount(); i++) {
				if (mir_wstrlen(ps[i]->m_tszAccName) > 0 && mir_strlen(ps[i]->m_szName) > 0) {
					if (db_get_b(0, KSMODULENAME, SETTING_PUSHOWEXTRA, TRUE)) {
						mir_snwprintf(protoInfoLine, TranslateT("%s\t(will be set to %s)\r\n"), ps[i]->m_tszAccName, pcli->pfnGetStatusModeDescription(ps[i]->m_status, 0));
						mir_wstrncat(protoInfo, protoInfoLine, _countof(protoInfo) - mir_wstrlen(protoInfo) - 1);
					}
				}
			}
			hIcon = Skin_LoadProtoIcon(ps[0]->m_szName, SKINICON_STATUS_OFFLINE);

			rtrimw(protoInfo);
			if (retryCount == (maxRetries - 1))
				mir_snwprintf(text, TranslateT("Resetting status... (last try (%d))%s"), retryCount + 1, protoInfo);
			else
				mir_snwprintf(text, TranslateT("Resetting status... (next retry (%d) in %d s)%s"), retryCount + 2, currentDelay / 1000, protoInfo);
		}
		break;

	case KS_CONN_STATE_RETRYNOCONN: // lParam = nullptr
		if (!db_get_b(0, KSMODULENAME, SETTING_PUOTHER, TRUE))
			return -1;

		if (retryCount == maxRetries - 1)
			mir_snwprintf(text, TranslateT("No internet connection seems available... (last try (%d))"), retryCount + 1);
		else
			mir_snwprintf(text, TranslateT("No internet connection seems available... (next retry (%d) in %d s)"), retryCount + 2, currentDelay / 1000);
		break;

	case KS_CONN_STATE_STOPPEDCHECKING: // lParam == BOOL succes
		if (!db_get_b(0, KSMODULENAME, SETTING_PURESULT, TRUE))
			return -1;

		if (lParam) {
			hIcon = Skin_LoadIcon(SKINICON_STATUS_ONLINE);
			mir_snwprintf(text, TranslateT("Status was set ok"));
		}
		else mir_snwprintf(text, TranslateT("Giving up"));
		break;
	}
	if (hIcon == nullptr)
		hIcon = Skin_LoadIcon(SKINICON_STATUS_OFFLINE);

	log_info(L"KeepStatus: %s", text);
	return ShowPopup(text, hIcon);
}

static INT_PTR ShowPopup(wchar_t *msg, HICON hIcon)
{
	POPUPDATAT ppd = {};
	ppd.lchIcon = hIcon;
	wcsncpy(ppd.lptzContactName, TranslateT("Keep status"), MAX_CONTACTNAME);
	wcsncpy(ppd.lptzText, msg, MAX_SECONDLINE);
	if (db_get_b(0, KSMODULENAME, SETTING_POPUP_USEWINCOLORS, 0)) {
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
	}
	else if (!db_get_b(0, KSMODULENAME, SETTING_POPUP_USEDEFCOLORS, 0)) {
		ppd.colorBack = db_get_dw(0, KSMODULENAME, SETTING_POPUP_BACKCOLOR, 0xAAAAAA);
		ppd.colorText = db_get_dw(0, KSMODULENAME, SETTING_POPUP_TEXTCOLOR, 0x0000CC);
	}
	ppd.PluginWindowProc = KSPopupDlgProc;

	switch (db_get_b(0, KSMODULENAME, SETTING_POPUP_DELAYTYPE, POPUP_DELAYFROMPU)) {
	case POPUP_DELAYCUSTOM:
		ppd.iSeconds = (int)db_get_dw(0, KSMODULENAME, SETTING_POPUP_TIMEOUT, 0);
		if (ppd.iSeconds == 0)
			ppd.iSeconds = currentDelay / 1000 - 1;
		break;

	case POPUP_DELAYPERMANENT:
		ppd.iSeconds = -1;
		break;

	case POPUP_DELAYFROMPU:
	default:
		ppd.iSeconds = 0;
		break;
	}
	return PUAddPopupT(&ppd);
}

LRESULT CALLBACK KSPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_CONTEXTMENU: // right
	case WM_COMMAND: // left
		switch (db_get_b(0, KSMODULENAME,
			(message == WM_COMMAND) ? SETTING_POPUP_LEFTCLICK : SETTING_POPUP_RIGHTCLICK,
			POPUP_ACT_CLOSEPOPUP)) {
		case POPUP_ACT_CANCEL:
			// cancel timer
			StopChecking();
			PUDeletePopup(hWnd);
			break;

		case POPUP_ACT_CLOSEPOPUP:
			// close the popup
			PUDeletePopup(hWnd);
			break;
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// services

INT_PTR StopReconnectingService(WPARAM, LPARAM)
{
	int ret = StartTimer(IDT_CHECKCONN | IDT_AFTERCHECK, -1, FALSE);
	StopChecking();
	return ret;
}

INT_PTR EnableProtocolService(WPARAM wParam, LPARAM lParam)
{
	char *szProto = (char *)lParam;
	if (szProto == nullptr)
		return -1;

	char dbSetting[128];
	mir_snprintf(dbSetting, "%s_enabled", szProto);
	if (!db_get_b(0, KSMODULENAME, dbSetting, 1)) // 'hard' disabled
		return -1;

	int ret = -2;
	for (int i = 0; i < connectionSettings.getCount(); i++) {
		TKSSettings &cs = connectionSettings[i];
		if (!mir_strcmp(szProto, cs.m_szName)) {
			if (wParam)
				cs.AssignStatus(CallProtoService(cs.m_szName, PS_GETSTATUS, 0, 0));
			else
				cs.AssignStatus(ID_STATUS_DISABLED);

			ret = 0;
			break;
		}
	}
	return ret;
}

INT_PTR IsProtocolEnabledService(WPARAM, LPARAM lParam)
{
	char *szProto = (char *)lParam;

	char dbSetting[128];
	mir_snprintf(dbSetting, "%s_enabled", szProto);
	if (!db_get_b(0, KSMODULENAME, dbSetting, 1))
		return FALSE;

	for (int i = 0; i < connectionSettings.getCount(); i++) {
		TKSSettings &cs = connectionSettings[i];
		if (!mir_strcmp(szProto, cs.m_szName))
			return cs.GetStatus() != ID_STATUS_DISABLED;
	}

	return FALSE;
}

INT_PTR AnnounceStatusChangeService(WPARAM, LPARAM lParam)
{
	PROTOCOLSETTINGEX *newSituation = (PROTOCOLSETTINGEX *)lParam;
	log_infoA("Another plugin announced a status change to %d for %s", newSituation->m_status, newSituation->m_szName == nullptr ? "all" : newSituation->m_szName);

	for (int i = 0; i < connectionSettings.getCount(); i++) {
		TKSSettings &cs = connectionSettings[i];
		if (!mir_strcmp(cs.m_szName, newSituation->m_szName))
			cs.AssignStatus(newSituation->m_status, newSituation->m_lastStatus, newSituation->m_szMsg);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// window for suspend

static DWORD CALLBACK MessageWndProc(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PROTOCOLSETTINGEX** ps = nullptr;

	switch (msg) {
	case WM_POWERBROADCAST:
		switch (wParam) {
		case PBT_APMSUSPEND:
			log_infoA("KeepStatus: suspend state detected: %08X %08X", wParam, lParam);
			if (ps == nullptr) {
				ps = GetCurrentProtoSettingsCopy();
				for (int i = 0; i < connectionSettings.getCount(); i++)
					EnableProtocolService(0, (LPARAM)ps[i]->m_szName);

				// set proto's offline, the clist will not try to reconnect in that case
				Clist_SetStatusMode(ID_STATUS_OFFLINE);
			}
			break;

			//case PBT_APMRESUMEAUTOMATIC: ?
		case PBT_APMRESUMESUSPEND:
		case PBT_APMRESUMECRITICAL:
			log_infoA("KeepStatus: resume from suspend state");
			if (ps != nullptr) {
				for (int i = 0; i < connectionSettings.getCount(); i++)
					connectionSettings[i].AssignStatus(ps[i]->m_status, ps[i]->m_lastStatus, ps[i]->m_szMsg);
				FreeProtoSettings(ps);
				ps = nullptr;
			}
			StartTimer(IDT_PROCESSACK, 0, FALSE);
			break;
		}
		break;

	case WM_DESTROY:
		if (ps != nullptr) {
			FreeProtoSettings(ps);
			ps = nullptr;
		}
		break;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Account control event

int OnKSAccChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;
	switch (wParam) {
	case PRAC_ADDED:
		connectionSettings.insert(new TKSSettings(pa));
		break;

	case PRAC_REMOVED:
		for (int i = 0; i < connectionSettings.getCount(); i++) {
			if (!mir_strcmp(connectionSettings[i].m_szName, pa->szModuleName)) {
				connectionSettings.remove(i);
				break;
			}
		}
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// init stuff

static int onShutdown(WPARAM, LPARAM)
{
	UnhookEvent(hStatusChangeHook);
	UnhookEvent(hProtoAckHook);
	UnhookEvent(hCSStatusChangeHook);
	UnhookEvent(hCSStatusChangeExHook);

	StopTimer(IDT_CHECKCONN | IDT_PROCESSACK | IDT_AFTERCHECK | IDT_CHECKCONTIN);
	if (IsWindow(hMessageWindow))
		DestroyWindow(hMessageWindow);

	connectionSettings.destroy();

	return 0;
}

int KSModuleLoaded(WPARAM, LPARAM)
{
	protoList = (OBJLIST<PROTOCOLSETTINGEX>*)&connectionSettings;

	hMessageWindow = nullptr;
	KSLoadOptions();

	HookEvent(ME_OPT_INITIALISE, KeepStatusOptionsInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, onShutdown);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnKSAccChanged);
	return 0;
}

void KeepStatusLoad()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, KSModuleLoaded);

	CreateHookableEvent(ME_KS_CONNECTIONEVENT);

	CreateServiceFunction(MS_KS_STOPRECONNECTING, StopReconnectingService);
	CreateServiceFunction(MS_KS_ENABLEPROTOCOL, EnableProtocolService);
	CreateServiceFunction(MS_KS_ISPROTOCOLENABLED, IsProtocolEnabledService);
	CreateServiceFunction(MS_KS_ANNOUNCESTATUSCHANGE, AnnounceStatusChangeService);

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, FALSE, 0);
	mainThreadId = GetCurrentThreadId();
}

void KeepStatusUnload()
{
	if (hMainThread)
		CloseHandle(hMainThread);

	DestroyHookableEvent(hConnectionEvent);
}
