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

#include "stdafx.h"

CFakePlugin KSPlugin(KSMODULENAME);

static HANDLE hConnectionEvent = nullptr;
static HANDLE hServices[4], hEvents[2];

static mir_cs GenTimerCS, GenStatusCS, CheckContinueslyCS;

static HANDLE hProtoAckHook = nullptr;
static HANDLE hStatusChangeHook = nullptr;
static HANDLE hCSStatusChangeHook = nullptr;
static HANDLE hCSStatusChangeExHook = nullptr;

static HWND hMessageWindow = nullptr;

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
LRESULT CALLBACK KSPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static uint32_t CALLBACK MessageWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// options.c
extern int KeepStatusOptionsInit(WPARAM wparam, LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////

void KSUnloadOptions()
{
	UnhookEvent(hProtoAckHook);
	UnhookEvent(hStatusChangeHook);
	UnhookEvent(hCSStatusChangeHook);
	UnhookEvent(hCSStatusChangeExHook);
	hProtoAckHook = hStatusChangeHook = hCSStatusChangeHook = hCSStatusChangeExHook = nullptr;

	if (IsWindow(hMessageWindow))
		DestroyWindow(hMessageWindow);

	if (StartTimer(IDT_CHECKCONTIN, -1, FALSE))
		WSACleanup();

	StopTimer(IDT_CHECKCONN | IDT_PROCESSACK | IDT_AFTERCHECK | IDT_CHECKCONTIN | IDT_CHECKCONNECTING);
}

int KSLoadOptions()
{
	KSUnloadOptions();

	if (KSPlugin.getByte(SETTING_CHECKCONNECTION, FALSE)) {
		if (KSPlugin.getByte(SETTING_CONTCHECK, FALSE)) {
			if (KSPlugin.getByte(SETTING_BYPING, FALSE)) {
				WSADATA wsaData;
				WSAStartup(MAKEWORD(2, 2), &wsaData);
			}
			StartTimer(IDT_CHECKCONTIN, 0, FALSE);
		}
		increaseExponential = KSPlugin.getByte(SETTING_INCREASEEXPONENTIAL, FALSE);
		currentDelay = initDelay = 1000 * KSPlugin.getDword(SETTING_INITDELAY, DEFAULT_INITDELAY);
		maxDelay = 1000 * KSPlugin.getDword(SETTING_MAXDELAY, DEFAULT_MAXDELAY);
		maxRetries = KSPlugin.getByte(SETTING_MAXRETRIES, 0);
		if (maxRetries == 0)
			maxRetries = -1;
		hProtoAckHook = HookEvent(ME_PROTO_ACK, ProcessProtoAck);
		hStatusChangeHook = HookEvent(ME_CLIST_STATUSMODECHANGE, StatusChange);
		if (ServiceExists(ME_CS_STATUSCHANGE))
			hCSStatusChangeHook = HookEvent(ME_CS_STATUSCHANGE, CSStatusChange);
		hCSStatusChangeExHook = HookEvent(ME_CS_STATUSCHANGEEX, CSStatusChangeEx);
		if (KSPlugin.getByte(SETTING_CHECKAPMRESUME, 0)) {
			if (!IsWindow(hMessageWindow)) {
				hMessageWindow = CreateWindowEx(0, L"STATIC", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
				SetWindowLongPtr(hMessageWindow, GWLP_WNDPROC, (LONG_PTR)MessageWndProc);
			}
		}
		retryCount = 0;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int SMProto::AssignStatus(int iStatus, int iLastStatus, wchar_t *pwszMsg)
{
	if (iStatus < MIN_STATUS || iStatus > MAX_STATUS)
		return -1;
	if (iStatus != ID_STATUS_OFFLINE && m_status == ID_STATUS_DISABLED)
		return -2;
	if (!KSPlugin.getByte(SETTING_NOLOCKED, 0) && Proto_GetAccount(m_szName)->IsLocked())
		return -3;

	mir_cslock lck(GenStatusCS);

	char dbSetting[128];
	mir_snprintf(dbSetting, "%s_enabled", m_szName);
	m_lastStatus = (iLastStatus == 0) ? m_status : iLastStatus;
	if (!KSPlugin.getByte(dbSetting, 1))
		m_status = ID_STATUS_DISABLED;
	else if (iStatus == ID_STATUS_LAST)
		m_status = m_lastStatus;
	else
		m_status = iStatus;

	log_info(0, "KeepStatus: assigning status %d (%d, %d) to %s", m_status, m_lastStatus, iLastStatus, m_szName);

	if (pwszMsg != nullptr && mir_wstrcmp(pwszMsg, m_szMsg)) {
		if (m_szMsg != nullptr)
			mir_free(m_szMsg);

		m_szMsg = mir_wstrdup(pwszMsg);
	}
	else if (pwszMsg != m_szMsg) {
		if (m_szMsg != nullptr)
			mir_free(m_szMsg);

		m_szMsg = nullptr;
	}
	return 0;
}

int SMProto::GetStatus() const
{
	switch (m_status) {
	case ID_STATUS_CURRENT:
		return Proto_GetStatus(m_szName);
	case ID_STATUS_LAST:
		return m_lastStatus;
	default:
		return m_status;
	}
}

static int SetCurrentStatus()
{
	TProtoSettings ps(protoList);
	for (auto &p : ps) {
		int realStatus = Proto_GetStatus(p->m_szName);
		int curStatus = p->GetStatus();
		if (curStatus == ID_STATUS_DISABLED)
			continue;
		if (curStatus == realStatus) {
			p->m_status = ID_STATUS_DISABLED;
			continue;
		}

		log_info(0, "KeepStatus: status for %s differs: stored = %d, real = %d", p->m_szName, curStatus, realStatus);

		// force offline before reconnecting?
		if (realStatus != ID_STATUS_OFFLINE && KSPlugin.getByte(SETTING_FIRSTOFFLINE, FALSE)) {
			log_info(0, "KeepStatus: Setting %s offline before making a new connection attempt", p->m_szName);
			CallProtoService(p->m_szName, PS_SETSTATUS, (WPARAM)ID_STATUS_OFFLINE, 0);
		}
	}
	ProcessPopup(KS_CONN_STATE_RETRY, (LPARAM)ps.getArray());
	return SetStatusEx(ps);
}

static int StatusChange(WPARAM wParam, LPARAM lParam)
{
	char *szProto = (char *)lParam;
	if (szProto == nullptr) { // global status change
		for (auto &it : protoList)
			it->AssignStatus(wParam, 0, it->m_szMsg);
	}
	else {
		for (auto &it : protoList)
			if (!mir_strcmp(it->m_szName, szProto))
				it->AssignStatus(wParam, 0, it->m_szMsg);
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

		for (int i = 0; i < protoList.getCount(); i++) {
			auto psi = protoSettings[i];
			if (psi->szName == nullptr)
				continue;

			for (auto &it : protoList) {
				if (it->m_szName == nullptr)
					continue;

				if (!mir_strcmp(psi->szName, it->m_szName))
					it->AssignStatus(psi->status, psi->lastStatus, it->m_szMsg);
			}
		}
	}

	return 0;
}

static int CSStatusChangeEx(WPARAM wParam, LPARAM pCount)
{
	// the status was changed by commonstatus (new)
	if (wParam != 0) {
		PROTOCOLSETTINGEX** protoSettings = *(PROTOCOLSETTINGEX***)wParam;
		if (protoSettings == nullptr)
			return -1;

		for (int i = 0; i < pCount; i++) {
			auto psi = protoSettings[i];
			if (psi->m_szName == nullptr)
				continue;

			for (auto &it : protoList) {
				if (it->m_szName == nullptr)
					continue;

				if (!mir_strcmp(psi->m_szName, it->m_szName))
					it->AssignStatus(psi->m_status, psi->m_lastStatus, psi->m_szMsg);
			}
		}
	}

	return 0;
}

struct TimerInfo
{
	int timer;
	int timeout;
	BOOL restart;
};

static INT_PTR CALLBACK StartTimerApcProc(void *param)
{
	TimerInfo *ti = (TimerInfo *)param;
	int res = 0;

	mir_cslock lck(GenTimerCS);
	log_debug(0, "StartTimer: %d, %d, %d", ti->timer, ti->timeout, ti->restart);
	log_debug(0, "ack: %u, chk: %u, aft: %u, cnt: %u, con: %u", processAckTimerId, checkConnectionTimerId, afterCheckTimerId, checkContinTimerId, checkConnectingTimerId);
	if (ti->timer & IDT_PROCESSACK) {
		res = (processAckTimerId == 0) ? 0 : 1;
		if (((processAckTimerId == 0) && (checkConnectionTimerId == 0)) || (ti->restart)) {
			if (ti->timeout != -1) {
				if (ti->restart)
					KillTimer(nullptr, processAckTimerId);
				if (ti->timeout == 0)
					ti->timeout = ackDelay;
				processAckTimerId = SetTimer(nullptr, 0, ti->timeout, CheckAckStatusTimer);
			}
		}
	}

	if (ti->timer & IDT_CHECKCONN) {
		res = (checkConnectionTimerId == 0 ? 0 : 1) || res;
		if ((checkConnectionTimerId == 0) || (ti->restart)) {
			if (ti->timeout != -1) {
				if (ti->restart)
					KillTimer(nullptr, checkConnectionTimerId);

				if (ti->timeout == 0)
					ti->timeout = initDelay;
				checkConnectionTimerId = SetTimer(nullptr, 0, ti->timeout, CheckConnectionTimer);
			}
		}
	}

	if (ti->timer & IDT_AFTERCHECK) {
		res = (afterCheckTimerId == 0 ? 0 : 1) || res;
		if ((afterCheckTimerId == 0) || (ti->restart)) {
			if (ti->timeout != -1) {
				if (ti->restart)
					KillTimer(nullptr, afterCheckTimerId);

				if (ti->timeout == 0)
					ti->timeout = initDelay / 2;
				afterCheckTimerId = SetTimer(nullptr, 0, ti->timeout, AfterCheckTimer);
			}
		}
	}

	if (ti->timer & IDT_CHECKCONTIN) {
		res = (checkContinTimerId == 0 ? 0 : 1) || res;
		if ((checkContinTimerId == 0) || (ti->restart)) {
			if (ti->timeout != -1) {
				if (ti->restart)
					KillTimer(nullptr, checkContinTimerId);

				if (ti->timeout == 0)
					ti->timeout = 1000 * KSPlugin.getDword(SETTING_CNTDELAY, CHECKCONTIN_DELAY);
				checkContinTimerId = SetTimer(nullptr, 0, ti->timeout, CheckContinueslyTimer);
			}
		}
	}

	if (ti->timer & IDT_CHECKCONNECTING) {
		res = (checkConnectingTimerId == 0 ? 0 : 1) || res;
		if ((checkConnectingTimerId == 0) || (ti->restart)) {
			if (ti->timeout != -1) {
				if (ti->restart)
					KillTimer(nullptr, checkConnectingTimerId);
				if (ti->timeout == 0)
					ti->timeout = initDelay / 2;
				checkConnectingTimerId = SetTimer(nullptr, 0, ti->timeout, CheckConnectingTimer);
			}
		}
	}

	log_debug(0, "ack: %u, chk: %u, aft: %u, cnt: %u, con: %u", processAckTimerId, checkConnectionTimerId, afterCheckTimerId, checkContinTimerId, checkConnectingTimerId);
	log_debug(0, "StartTimer done %d", res);
	return res;
}

static int StartTimer(int timer, int timeout, BOOL restart)
{
	TimerInfo ti = { timer, timeout, restart };
	return CallFunctionSync(&StartTimerApcProc, &ti);
}

static int StopTimer(int timer)
{
	int res = 0;

	mir_cslock lck(GenTimerCS);
	log_debug(0, "StopTimer %d", timer);
	log_debug(0, "ack: %u, chk: %u, aft: %u, cnt: %u, con: %u", processAckTimerId, checkConnectionTimerId, afterCheckTimerId, checkContinTimerId, checkConnectingTimerId);

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

	log_debug(0, "ack: %u, chk: %u, aft: %u, cnt: %u, con: %u", processAckTimerId, checkConnectionTimerId, afterCheckTimerId, checkContinTimerId, checkConnectingTimerId);
	log_debug(0, "StopTimer done %d", res);
	return res;
}

static int ProcessProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_STATUS && ack->type != ACKTYPE_LOGIN)
		return 0;

	char dbSetting[128];
	mir_snprintf(dbSetting, "%s_enabled", ack->szModule);
	if (!KSPlugin.getByte(dbSetting, 1))
		return 0;

	if (ack->type == ACKTYPE_STATUS && ack->result == ACKRESULT_SUCCESS) {
		for (auto &it : protoList)
			if (!mir_strcmp(it->m_szName, ack->szModule))
				it->lastStatusAckTime = GetTickCount();

		StartTimer(IDT_PROCESSACK, 0, FALSE);
		return 0;
	}

	if (ack->type == ACKTYPE_LOGIN) {
		if (ack->lParam == LOGINERR_OTHERLOCATION) {
			for (auto &it : protoList) {
				if (!mir_strcmp(ack->szModule, it->m_szName)) {
					it->AssignStatus(ID_STATUS_OFFLINE);
					if (KSPlugin.getByte(SETTING_CNCOTHERLOC, 0)) {
						StopTimer(IDT_PROCESSACK);
						for (auto &jt : protoList)
							jt->AssignStatus(ID_STATUS_OFFLINE);
					}

					NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_OTHERLOCATION, (LPARAM)it->m_szName);
					ProcessPopup(KS_CONN_STATE_OTHERLOCATION, (LPARAM)ack->szModule);
				}
			}
		}
		else if (ack->result == ACKRESULT_FAILED) {
			// login failed
			NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_LOGINERROR, (LPARAM)ack->szModule);
			
			switch (KSPlugin.getByte(SETTING_LOGINERR, LOGINERR_NOTHING)) {
			case LOGINERR_CANCEL:
				log_info(0, "KeepStatus: cancel on login error (%s)", ack->szModule);
				for (auto &it : protoList)
					if (!mir_strcmp(ack->szModule, it->m_szName))
						it->AssignStatus(ID_STATUS_OFFLINE);

				ProcessPopup(KS_CONN_STATE_LOGINERROR, (LPARAM)ack->szModule);
				StopChecking();
				break;

			case LOGINERR_SETDELAY:
				{
					int newDelay = 1000 * KSPlugin.getDword(SETTING_LOGINERR_DELAY, DEFAULT_MAXDELAY);
					log_info(0, "KeepStatus: set delay to %d ms on login error (%s)", newDelay, ack->szModule);
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

static void CALLBACK CheckConnectingTimer(HWND, UINT, UINT_PTR, DWORD)
{
	StopTimer(IDT_CHECKCONNECTING);

	for (auto &it : protoList) {
		int curStatus = it->GetStatus();
		if (IsStatusConnecting(curStatus)) { // connecting
			int maxConnectingTime = KSPlugin.getDword(SETTING_MAXCONNECTINGTIME, 0);
			if (maxConnectingTime > 0) {
				if ((unsigned int)maxConnectingTime <= ((GetTickCount() - it->lastStatusAckTime) / 1000)) {
					// set offline
					log_info(0, "KeepStatus: %s is too long connecting; setting offline", it->m_szName);
					CallProtoService(it->m_szName, PS_SETSTATUS, (WPARAM)ID_STATUS_OFFLINE, 0);
				}
			}
		}
	}
}

static void CALLBACK CheckAckStatusTimer(HWND, UINT, UINT_PTR, DWORD)
{
	bool needChecking = false;

	StopTimer(IDT_PROCESSACK);
	for (auto &it : protoList) {
		int curStatus = it->GetStatus();
		int newStatus = Proto_GetStatus(it->m_szName);
		// ok, np
		if (curStatus == ID_STATUS_CURRENT || curStatus == ID_STATUS_DISABLED || curStatus == newStatus || newStatus > MAX_STATUS)
			continue;

		if (IsStatusConnecting(newStatus)) { // connecting
			int maxConnectingTime = KSPlugin.getDword(SETTING_MAXCONNECTINGTIME, 0);
			if (maxConnectingTime > 0)
				StartTimer(IDT_CHECKCONNECTING, (maxConnectingTime * 1000 - (GetTickCount() - it->lastStatusAckTime)), FALSE);
		}
		// keepstatus' administration was wrong!
		else if (newStatus != ID_STATUS_OFFLINE)
			it->AssignStatus(newStatus);

		// connection lost
		else if (newStatus == ID_STATUS_OFFLINE) {// start checking connection
			if (!StartTimer(IDT_CHECKCONN, -1, FALSE)) { /* check if not already checking */
				needChecking = true;
				log_info(0, "KeepStatus: connection lost! (%s)", it->m_szName);
				NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_LOST, (LPARAM)it->m_szName);
				ProcessPopup(KS_CONN_STATE_LOST, (LPARAM)it->m_szName);
			}
		}
	}

	if (needChecking)
		StartTimer(IDT_CHECKCONN, initDelay, FALSE);
}

static void CALLBACK CheckConnectionTimer(HWND, UINT, UINT_PTR, DWORD)
{
	log_debug(0, "CheckConnectionTimer");
	bool setStatus = false;

	for (auto &it : protoList) {
		int realStatus = Proto_GetStatus(it->m_szName);
		int shouldBeStatus = it->GetStatus();
		if (shouldBeStatus == ID_STATUS_LAST)
			shouldBeStatus = it->m_lastStatus;
		if (shouldBeStatus == ID_STATUS_DISABLED)
			continue;
		if ((shouldBeStatus != realStatus) && (realStatus == ID_STATUS_OFFLINE) || (realStatus < MIN_STATUS)) {
			setStatus = true;
			break;
		}
	}

	// one of the status was wrong
	if (setStatus && (maxRetries == -1 || retryCount < maxRetries)) {
		if (increaseExponential)
			currentDelay = min(2 * currentDelay, maxDelay);

		if (((KSPlugin.getByte(SETTING_CHKINET, 0)) && (!InternetGetConnectedState(nullptr, 0))) || ((KSPlugin.getByte(SETTING_BYPING, FALSE)) && (!bLastPingResult))) {
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

	log_debug(0, "CheckConnectionTimer done");
}

static int StopChecking()
{
	StopTimer(IDT_CHECKCONN | IDT_PROCESSACK | IDT_AFTERCHECK | IDT_CHECKCONNECTING);

	BOOL isOk = TRUE;
	for (auto &it : protoList) {
		int curStatus = it->GetStatus();
		int newStatus = Proto_GetStatus(it->m_szName);
		if (newStatus != curStatus) {
			it->AssignStatus(newStatus);
			isOk = FALSE;
			break;
		}
	}

	NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_STOPPEDCHECKING, (LPARAM)isOk);
	ProcessPopup(KS_CONN_STATE_STOPPEDCHECKING, (LPARAM)isOk);
	log_info(0, "KeepStatus: stop checking (%s)", isOk ? "success" : "failure");
	retryCount = 0;
	currentDelay = initDelay;

	return 0;
}

static VOID CALLBACK AfterCheckTimer(HWND, UINT, UINT_PTR, DWORD)
{
	// after each connection check, this function is called to see if connection was recovered
	StopTimer(IDT_AFTERCHECK);

	bool setStatus = false;

	for (auto &it : protoList) {
		int realStatus = Proto_GetStatus(it->m_szName);
		int shouldBeStatus = it->GetStatus();
		if (shouldBeStatus == ID_STATUS_LAST) // this should never happen
			shouldBeStatus = it->m_lastStatus;
		if (shouldBeStatus == ID_STATUS_DISABLED) //  (on ignoring proto)
			continue;
		if ((shouldBeStatus != realStatus) && (realStatus == ID_STATUS_OFFLINE) || (realStatus < MIN_STATUS))
			setStatus = true;
	}

	if (!setStatus || retryCount == maxRetries)
		StopChecking();
}

static void CheckContinuouslyFunction(void *)
{
	Thread_SetName("KeepStatus: CheckContinuouslyFunction");

	static int pingFailures = 0;

	// one at the time is enough, do it the 'easy' way
	mir_cslock lck(CheckContinueslyCS);

	// do a ping, even if reconnecting
	bool doPing = false;
	for (auto &it : protoList) {
		int shouldBeStatus = it->GetStatus();
		if (shouldBeStatus == ID_STATUS_LAST)
			shouldBeStatus = it->m_lastStatus;

		if (shouldBeStatus == ID_STATUS_DISABLED)
			continue;

		if (shouldBeStatus != ID_STATUS_OFFLINE) {
			log_debug(0, "CheckContinuouslyFunction: %s should be %d", it->m_szName, shouldBeStatus);
			doPing = true;
		}
	}

	if (!doPing) {
		log_debug(0, "CheckContinuouslyFunction: All protocols should be offline, no need to check connection");
		return;
	}

	BOOL ping = KSPlugin.getByte(SETTING_BYPING, FALSE);
	if (ping) {
		DBVARIANT dbv;
		if (db_get_s(0, KSMODULENAME, SETTING_PINGHOST, &dbv))
			ping = FALSE;
		else {
			char *start, *end;
			char host[MAX_PATH];
			uint32_t *addr;
			struct hostent *hostent;
			char reply[sizeof(ICMP_ECHO_REPLY) + 8];

			bLastPingResult = FALSE;
			HANDLE hICMPFile = (HANDLE)IcmpCreateFile();
			if (hICMPFile == INVALID_HANDLE_VALUE) {
				bLastPingResult = TRUE;
				log_info(0, "KeepStatus: icmp.dll error (2)");
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
						addr = (uint32_t *)(*hostent->h_addr_list);
						bLastPingResult = (IcmpSendEcho(hICMPFile, *addr, nullptr, 0, nullptr, reply, sizeof(ICMP_ECHO_REPLY) + 8, 5000) != 0);

						if (bLastPingResult)
							pingFailures = 0;
						else
							pingFailures++;

						log_debug(0, "CheckContinuouslyFunction: pinging %s (result: %d/%d)", host, bLastPingResult, pingFailures);
					}
					else log_debug(0, "CheckContinuouslyFunction: unable to resolve %s", host);

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

		for (auto &pa : Accounts()) {
			if (!IsSuitableProto(pa))
				continue;

			if (IsStatusConnecting(pa->iRealStatus)) {
				log_debug(0, "CheckContinuouslyFunction: %s is connecting", pa->szModuleName);
				continue; // connecting, leave alone
			}
			if (IsProtocolEnabledService(0, (LPARAM)pa->szModuleName)) {
				log_debug(0, "CheckContinuouslyFunction: forcing %s offline", pa->szModuleName);
				CallProtoService(pa->szModuleName, PS_SETSTATUS, (WPARAM)ID_STATUS_OFFLINE, 0);
			}
		}

		if (StartTimer(IDT_CHECKCONN | IDT_PROCESSACK, -1, FALSE)) {// are our 'set offlines' noticed?
			log_debug(0, "CheckContinuouslyFunction: currently checking");
			return;
		}

		log_info(0, "KeepStatus: connection lost! (continuesly check)");
		NotifyEventHooks(hConnectionEvent, (WPARAM)KS_CONN_STATE_LOST, 0);
		ProcessPopup(KS_CONN_STATE_LOST, 0);
		maxRetries = KSPlugin.getByte(SETTING_MAXRETRIES, 0);
		if (maxRetries == 0)
			maxRetries = -1;
		StartTimer(IDT_CHECKCONN, initDelay, FALSE);
	}
}

static VOID CALLBACK CheckContinueslyTimer(HWND, UINT, UINT_PTR, DWORD)
{
	if (KSPlugin.getByte(SETTING_BYPING, FALSE))
		mir_forkthread(CheckContinuouslyFunction);
	else
		CheckContinuouslyFunction(nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////
// popup

static INT_PTR ShowPopup(const wchar_t *msg, HICON hIcon)
{
	POPUPDATAW ppd;
	ppd.lchIcon = hIcon;
	wcsncpy(ppd.lpwzContactName, TranslateT("Keep status"), MAX_CONTACTNAME);
	wcsncpy(ppd.lpwzText, msg, MAX_SECONDLINE);
	if (KSPlugin.getByte(SETTING_POPUP_USEWINCOLORS, 0)) {
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
	}
	else if (!KSPlugin.getByte(SETTING_POPUP_USEDEFCOLORS, 0)) {
		ppd.colorBack = KSPlugin.getDword(SETTING_POPUP_BACKCOLOR, 0xAAAAAA);
		ppd.colorText = KSPlugin.getDword(SETTING_POPUP_TEXTCOLOR, 0x0000CC);
	}
	ppd.PluginWindowProc = KSPopupDlgProc;

	switch (KSPlugin.getByte(SETTING_POPUP_DELAYTYPE, POPUP_DELAYFROMPU)) {
	case POPUP_DELAYCUSTOM:
		ppd.iSeconds = (int)KSPlugin.getDword(SETTING_POPUP_TIMEOUT, 0);
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
	return (INT_PTR)PUAddPopupW(&ppd);
}

static wchar_t* GetHumanName(LPARAM lParam)
{
	PROTOACCOUNT *ProtoAccount = Proto_GetAccount((char*)lParam);
	return (ProtoAccount != nullptr) ? ProtoAccount->tszAccountName : TranslateT("Protocol");
}

static int ProcessPopup(int reason, LPARAM lParam)
{
	if (!KSPlugin.getByte(SETTING_SHOWCONNECTIONPOPUPS, FALSE))
		return -1;

	HICON hIcon = nullptr;
	CMStringW wszText;

	switch (reason) {
	case KS_CONN_STATE_OTHERLOCATION: // lParam = 1 proto
		if (!KSPlugin.getByte(SETTING_PUOTHER, TRUE))
			return -1;

		hIcon = Skin_LoadProtoIcon((char*)lParam, SKINICON_STATUS_OFFLINE);
		wszText.Format(TranslateT("%s connected from another location"), GetHumanName(lParam));
		break;

	case KS_CONN_STATE_LOGINERROR:	// lParam = 1 proto
		if (!KSPlugin.getByte(SETTING_PUOTHER, TRUE))
			return -1;

		hIcon = Skin_LoadProtoIcon((char*)lParam, SKINICON_STATUS_OFFLINE);
		if (KSPlugin.getByte(SETTING_LOGINERR, LOGINERR_NOTHING) == LOGINERR_CANCEL)
			wszText.Format(TranslateT("%s login error, cancel reconnecting"), GetHumanName(lParam));
		else if (KSPlugin.getByte(SETTING_LOGINERR, LOGINERR_NOTHING) == LOGINERR_SETDELAY)
			wszText.Format(TranslateT("%s login error (next retry (%d) in %d s)"), GetHumanName(lParam), retryCount + 1, KSPlugin.getDword(SETTING_LOGINERR_DELAY, DEFAULT_MAXDELAY));
		else
			return -1;
		break;

	case KS_CONN_STATE_LOST: // lParam = 1 proto, or nullptr
		if (!KSPlugin.getByte(SETTING_PUCONNLOST, TRUE))
			return -1;

		if (lParam) { // указатель на имя модуля. 
			hIcon = Skin_LoadProtoIcon((char*)lParam, SKINICON_STATUS_OFFLINE);
			wszText.Format(TranslateT("%s status error (next retry (%d) in %d s)"), GetHumanName(lParam), retryCount + 1, currentDelay / 1000);
		}
		else wszText.Format(TranslateT("Status error (next retry (%d) in %d s)"), retryCount + 1, currentDelay / 1000);
		break;

	case KS_CONN_STATE_RETRY:  // lParam = PROTOCOLSETTINGEX**
		if (!KSPlugin.getByte(SETTING_PUCONNRETRY, TRUE))
			return -1;
		
		if (lParam) {
			PROTOCOLSETTINGEX **ps = (PROTOCOLSETTINGEX **)lParam;

			if (retryCount == maxRetries - 1)
				wszText.Format(TranslateT("Resetting status... (last try (%d))"), retryCount + 1);
			else
				wszText.Format(TranslateT("Resetting status... (next retry (%d) in %d s)"), retryCount + 2, currentDelay / 1000);
			wszText.Append(L"\r\n");
			for (int i = 0; i < protoList.getCount(); i++) {
				PROTOCOLSETTINGEX *p = ps[i];
				if (p->m_status == ID_STATUS_DISABLED)
					continue;
				
				if (mir_wstrlen(p->m_tszAccName) > 0)
					if (KSPlugin.getByte(SETTING_PUSHOWEXTRA, TRUE))
						wszText.AppendFormat(TranslateT("%s\t(will be set to %s)\r\n"), p->m_tszAccName, Clist_GetStatusModeDescription(p->m_status, 0));
			}

			hIcon = Skin_LoadProtoIcon(ps[0]->m_szName, SKINICON_STATUS_OFFLINE);
			wszText.TrimRight();
		}
		break;

	case KS_CONN_STATE_RETRYNOCONN: // lParam = nullptr
		if (!KSPlugin.getByte(SETTING_PUOTHER, TRUE))
			return -1;

		if (retryCount == maxRetries - 1)
			wszText.Format(TranslateT("No internet connection seems available... (last try (%d))"), retryCount + 1);
		else
			wszText.Format(TranslateT("No internet connection seems available... (next retry (%d) in %d s)"), retryCount + 2, currentDelay / 1000);
		break;

	case KS_CONN_STATE_STOPPEDCHECKING: // lParam == BOOL succes
		if (!KSPlugin.getByte(SETTING_PURESULT, TRUE))
			return -1;

		if (lParam) {
			hIcon = Skin_LoadIcon(SKINICON_STATUS_ONLINE);
			wszText.Format(TranslateT("Status was set ok"));
		}
		else wszText.Format(TranslateT("Giving up"));
		break;
	}
	if (hIcon == nullptr)
		hIcon = Skin_LoadIcon(SKINICON_STATUS_OFFLINE);

	Netlib_Logf(0, "KeepStatus: %s", wszText.c_str());
	return ShowPopup(wszText, hIcon);
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
	if (!KSPlugin.getByte(dbSetting, 1)) // 'hard' disabled
		return -1;

	int ret = -2;
	for (auto &it : protoList) {
		if (!mir_strcmp(szProto, it->m_szName)) {
			if (wParam)
				it->AssignStatus(Proto_GetStatus(it->m_szName));
			else
				it->AssignStatus(ID_STATUS_DISABLED);

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
	if (!KSPlugin.getByte(dbSetting, 1))
		return FALSE;

	for (auto &it : protoList)
		if (!mir_strcmp(szProto, it->m_szName))
			return it->GetStatus() != ID_STATUS_DISABLED;

	return FALSE;
}

INT_PTR AnnounceStatusChangeService(WPARAM, LPARAM lParam)
{
	PROTOCOLSETTINGEX *newSituation = (PROTOCOLSETTINGEX *)lParam;
	log_info(0, "Another plugin announced a status change to %d for %s", newSituation->m_status, newSituation->m_szName == nullptr ? "all" : newSituation->m_szName);

	for (auto &it : protoList)
		if (!mir_strcmp(it->m_szName, newSituation->m_szName))
			it->AssignStatus(newSituation->m_status, newSituation->m_lastStatus, newSituation->m_szMsg);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// window for suspend

static uint32_t CALLBACK MessageWndProc(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static TProtoSettings *ps = nullptr;

	switch (msg) {
	case WM_POWERBROADCAST:
		switch (wParam) {
		case PBT_APMSUSPEND:
			log_info(0, "KeepStatus: suspend state detected: %08X %08X", wParam, lParam);
			if (ps == nullptr) {
				ps = new TProtoSettings(protoList);
				for (auto &it : *ps)
					EnableProtocolService(0, (LPARAM)it->m_szName);

				// set proto's offline, the clist will not try to reconnect in that case
				Clist_SetStatusMode(ID_STATUS_OFFLINE);
			}
			break;

		case PBT_APMRESUMESUSPEND:
		case PBT_APMRESUMECRITICAL:
		// case PBT_APMRESUMEAUTOMATIC: ?
			log_info(0, "KeepStatus: resume from suspend state");
			if (ps != nullptr) {
				for (auto &it : *ps) {
					SMProto *p = protoList.find(it);
					if (p)
						p->AssignStatus(it->m_status, it->m_lastStatus, it->m_szMsg);
				}
				delete ps;
				ps = nullptr;
			}
			StartTimer(IDT_PROCESSACK, 0, FALSE);
			break;
		}
		break;

	case WM_DESTROY:
		if (ps != nullptr) {
			delete ps;
			ps = nullptr;
		}
		break;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// init stuff

static int onShutdown(WPARAM, LPARAM)
{
	KSUnloadOptions();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int KSModuleLoaded(WPARAM, LPARAM)
{
	hMessageWindow = nullptr;
	KSLoadOptions();

	hEvents[0] = HookEvent(ME_OPT_INITIALISE, KeepStatusOptionsInit);
	hEvents[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, onShutdown);
	return 0;
}

void KeepStatusLoad()
{
	if (g_plugin.bMirandaLoaded)
		KSModuleLoaded(0, 0);
	else
		HookEvent(ME_SYSTEM_MODULESLOADED, KSModuleLoaded);

	hConnectionEvent = CreateHookableEvent(ME_KS_CONNECTIONEVENT);

	hServices[0] = CreateServiceFunction(MS_KS_STOPRECONNECTING, StopReconnectingService);
	hServices[1] = CreateServiceFunction(MS_KS_ENABLEPROTOCOL, EnableProtocolService);
	hServices[2] = CreateServiceFunction(MS_KS_ISPROTOCOLENABLED, IsProtocolEnabledService);
	hServices[3] = CreateServiceFunction(MS_KS_ANNOUNCESTATUSCHANGE, AnnounceStatusChangeService);
}

void KeepStatusUnload()
{
	if (g_plugin.bMirandaLoaded)
		onShutdown(0, 0);

	KillModuleOptions(&KSPlugin);

	for (auto &it : hServices) {
		DestroyServiceFunction(it);
		it = nullptr;
	}

	for (auto &it : hEvents) {
		UnhookEvent(it);
		it = nullptr;
	}

	DestroyHookableEvent(hConnectionEvent); hConnectionEvent = nullptr;
}
