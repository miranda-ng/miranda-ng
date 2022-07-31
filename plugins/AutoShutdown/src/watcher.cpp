/*

'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

/* Msg Shutdown */
static HANDLE hHookEventAdded;
/* Transfer Shutdown */
static HANDLE hHookProtoAck;
/* Idle Shutdown */
static HANDLE hHookIdleChanged;
/* Status Shutdown */
static HANDLE hHookSettingChanged;
/* Weather Shutdown */
static HANDLE hHookWeatherUpdated;
/* Services */
static HANDLE hEventWatcherChanged;
/* Misc */
static HANDLE hHookModulesLoaded;

/************************* Shared *************************************/

static uint16_t currentWatcherType;

static void __stdcall MainThreadMapping(void *param)
{
	HANDLE *phDoneEvent = (HANDLE*)param;
	ServiceShutdown(0, TRUE); /* ensure main thread (for cpu usage shutdown) */
	ServiceStopWatcher(0, 0);
	if (*phDoneEvent != nullptr)
		SetEvent(*phDoneEvent);
}

static void __inline ShutdownAndStopWatcher(void)
{
	HANDLE hDoneEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	CallFunctionAsync(MainThreadMapping, &hDoneEvent);
	if (hDoneEvent != nullptr) {
		WaitForSingleObject(hDoneEvent, INFINITE);
		CloseHandle(hDoneEvent);
	}
}

/************************* Msg Shutdown *******************************/

static int MsgEventAdded(WPARAM, LPARAM hDbEvent)
{
	if (currentWatcherType & SDWTF_MESSAGE) {
		DB::EventInfo dbei;
		dbei.cbBlob = -1;
		if (db_event_get(hDbEvent, &dbei))
			return 0;

		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
			DBVARIANT dbv;
			if (!g_plugin.getWString("Message", &dbv)) {
				ltrimw(rtrimw(dbv.pwszVal));
				ptrW wszMsg(DbEvent_GetTextW(&dbei, CP_ACP));
				if (wszMsg != nullptr && wcsstr(wszMsg, dbv.pwszVal) != nullptr)
					ShutdownAndStopWatcher(); /* msg with specified text recvd */
				mir_free(dbv.pwszVal); /* does NULL check */
			}
		}
	}
	return 0;
}

/************************* Transfer Shutdown **************************/

static HANDLE *transfers;
static int nTransfersCount;

static int ProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_FILE)
		return 0;

	switch (ack->result) {
	case ACKRESULT_DATA:
		{
			for (int i = 0; i < nTransfersCount; ++i)
				if (transfers[i] == ack->hProcess)
					break; /* already in list */
			/* insert into list */
			HANDLE *buf = (HANDLE*)mir_realloc(transfers, (nTransfersCount + 1)*sizeof(HANDLE));
			if (buf != nullptr) {
				transfers = buf;
				transfers[nTransfersCount] = ack->hProcess;
				++nTransfersCount;
			}
			break;
		}
	case ACKRESULT_SUCCESS:
	case ACKRESULT_FAILED:
	case ACKRESULT_DENIED:
		for (int i = 0; i < nTransfersCount; ++i) {
			if (transfers[i] == ack->hProcess) {
				/* remove from list */
				if (i < (nTransfersCount - 1))
					memmove(&transfers[i], &transfers[i + 1], (nTransfersCount - i - 1)*sizeof(HANDLE));
				--nTransfersCount;
				HANDLE *buf = (HANDLE*)mir_realloc(transfers, nTransfersCount*sizeof(HANDLE));
				if (buf != nullptr) transfers = buf;
				else if (!nTransfersCount) transfers = nullptr;
				/* stop watcher */
				if (!nTransfersCount && (currentWatcherType&SDWTF_FILETRANSFER))
					ShutdownAndStopWatcher();
				break;
			}
		}
		break;
	}
	return 0;
}

/************************* Idle Shutdown ******************************/

static int IdleChanged(WPARAM, LPARAM lParam)
{
	if (currentWatcherType&SDWTF_IDLE && lParam&IDF_ISIDLE)
		ShutdownAndStopWatcher();
	return 0;
}

/************************* Status Shutdown ****************************/

static BOOL CheckAllContactsOffline(void)
{
	BOOL fSmartCheck, fAllOffline = TRUE; /* tentatively */
	fSmartCheck = g_plugin.getByte("SmartOfflineCheck", SETTING_SMARTOFFLINECHECK_DEFAULT);
	for (auto &hContact : Contacts()) {
		char *pszProto = Proto_GetBaseAccountName(hContact);
		if (pszProto != nullptr && Proto_GetStatus(pszProto) != ID_STATUS_OFFLINE) {
			if (Contact::IsGroupChat(hContact, pszProto)) continue;
			if (db_get_w(hContact, pszProto, "Status", 0) != ID_STATUS_OFFLINE) {
				if (fSmartCheck) {
					if (Contact::IsHidden(hContact)) continue;
					if (!Contact::OnList(hContact)) continue;
				}
				fAllOffline = FALSE;
				break;
			}
		}
	}
	return fAllOffline;
}

static int StatusSettingChanged(WPARAM wParam, LPARAM lParam)
{
	if (currentWatcherType&SDWTF_STATUS) {
		DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING*)lParam;
		if ((HANDLE)wParam != nullptr && dbcws->value.wVal == ID_STATUS_OFFLINE && !strcmp(dbcws->szSetting, "Status")) {
			char *pszProto = Proto_GetBaseAccountName(wParam);
			if (pszProto != nullptr && !strcmp(dbcws->szModule, pszProto))
				if (CheckAllContactsOffline())
					ShutdownAndStopWatcher();
		}
	}
	return 0;
}

/************************* Cpu Shutdown *******************************/

static uint32_t idCpuUsageThread;

static BOOL CALLBACK CpuUsageWatcherProc(uint8_t nCpuUsage, LPARAM lParam)
{
	static uint8_t nTimesBelow = 0; /* only one watcher thread */
	/* terminated? */
	if (idCpuUsageThread != GetCurrentThreadId()) {
		nTimesBelow = 0;
		return FALSE; /* stop poll thread */
	}
	/* ignore random peaks */
	if (nCpuUsage < (uint8_t)lParam) ++nTimesBelow;
	else nTimesBelow = 0;
	if (nTimesBelow == 3) {
		nTimesBelow = 0;
		ShutdownAndStopWatcher();
		return FALSE; /* stop poll thread */
	}
	return TRUE;
}

/************************* Weather Shutdown ***************************/

static int WeatherUpdated(WPARAM wParam, LPARAM lParam)
{
	char *pszProto = Proto_GetBaseAccountName(wParam);
	if ((BOOL)lParam && pszProto != nullptr && Proto_GetStatus(pszProto) == ID_STATUS_INVISIBLE)
		if (g_plugin.getByte("WeatherShutdown", SETTING_WEATHERSHUTDOWN_DEFAULT))
			ServiceShutdown(SDSDT_SHUTDOWN, TRUE);
	return 0;
}

/************************* Services ***********************************/

INT_PTR ServiceStartWatcher(WPARAM, LPARAM lParam)
{
	/* passing watcherType as lParam is only to be used internally, undocumented */
	if (lParam == 0)
		lParam = (LPARAM)g_plugin.getWord("WatcherFlags", 0);

	/* invalid flags or empty? */
	if (!(lParam&SDWTF_MASK))
		return 1;

	/* no specific time choice? */
	if (lParam&SDWTF_SPECIFICTIME && !(lParam&SDWTF_ST_MASK))
		return 2;

	if (currentWatcherType == (uint16_t)lParam)
		return 3;

	if (currentWatcherType != 0) {
		/* Time Shutdown */
		CloseCountdownFrame(); /* fails if not opened */
		/* Cpu Shutdown */
		idCpuUsageThread = 0;
	}
	SetShutdownMenuItem(true);
	SetShutdownToolbarButton(true);
	currentWatcherType = (uint16_t)lParam;
	NotifyEventHooks(hEventWatcherChanged, TRUE, 0);

	/* Time Shutdown */
	if (currentWatcherType&SDWTF_SPECIFICTIME)
		ShowCountdownFrame(currentWatcherType); /* after modules loaded */
	/* Cpu Shutdown */
	if (currentWatcherType&SDWTF_CPUUSAGE)
		idCpuUsageThread = PollCpuUsage(CpuUsageWatcherProc, (LPARAM)DBGetContactSettingRangedByte(0, MODULENAME, "CpuUsageThreshold", SETTING_CPUUSAGETHRESHOLD_DEFAULT, 1, 100), 1500);
	/* Transfer Shutdown */
	if (currentWatcherType&SDWTF_FILETRANSFER && !nTransfersCount)
		ShutdownAndStopWatcher();
	/* Status Shutdown */
	if (currentWatcherType&SDWTF_STATUS && CheckAllContactsOffline())
		ShutdownAndStopWatcher();
	return 0;
}

INT_PTR ServiceStopWatcher(WPARAM, LPARAM)
{
	if (currentWatcherType == 0) return 1;

	/* Time Shutdown */
	if (currentWatcherType&SDWTF_SPECIFICTIME)
		CloseCountdownFrame();
	/* Cpu Shutdown */
	idCpuUsageThread = 0;

	currentWatcherType = 0;
	SetShutdownMenuItem(FALSE);
	SetShutdownToolbarButton(FALSE);
	NotifyEventHooks(hEventWatcherChanged, FALSE, 0);
	return 0;
}

INT_PTR ServiceIsWatcherEnabled(WPARAM, LPARAM)
{
	return currentWatcherType != 0;
}

/************************* Misc ***********************************/

void WatcherModulesLoaded(void)
{
	/* Weather Shutdown */
	if (ServiceExists(MS_WEATHER_UPDATE))
		hHookWeatherUpdated = HookEvent(ME_WEATHER_UPDATED, WeatherUpdated);

	/* restore watcher if it was running on last exit */
	if (g_plugin.getByte("RememberOnRestart", 0) == SDROR_RUNNING) {
		g_plugin.setByte("RememberOnRestart", 1);
		ServiceStartWatcher(0, 0); /* after modules loaded */
	}
}

void InitWatcher(void)
{
	/* Shared */
	currentWatcherType = 0;
	/* Message Shutdown */
	hHookEventAdded = HookEvent(ME_DB_EVENT_ADDED, MsgEventAdded);
	/* Status Shutdown*/
	hHookSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, StatusSettingChanged);
	/* Idle Shutdown */
	hHookIdleChanged = HookEvent(ME_IDLE_CHANGED, IdleChanged);
	/* Transfer Shutdown */
	transfers = nullptr;
	nTransfersCount = 0;
	hHookProtoAck = HookEvent(ME_PROTO_ACK, ProtoAck);
	/* Weather Shutdown */
	hHookWeatherUpdated = nullptr;
	/* Services */
	hEventWatcherChanged = CreateHookableEvent(ME_AUTOSHUTDOWN_WATCHERCHANGED);
	CreateServiceFunction(MS_AUTOSHUTDOWN_STARTWATCHER, ServiceStartWatcher);
	CreateServiceFunction(MS_AUTOSHUTDOWN_STOPWATCHER, ServiceStopWatcher);
	CreateServiceFunction(MS_AUTOSHUTDOWN_ISWATCHERENABLED, ServiceIsWatcherEnabled);
}

void UninitWatcher(void)
{
	/* remember watcher if running */
	if (!ServiceStopWatcher(0, 0))
		if (g_plugin.getByte("RememberOnRestart", SETTING_REMEMBERONRESTART_DEFAULT))
			g_plugin.setByte("RememberOnRestart", SDROR_RUNNING);

	/* Message Shutdown */
	UnhookEvent(hHookEventAdded);
	/* Status Shutdown*/
	UnhookEvent(hHookSettingChanged);
	/* Idle Shutdown */
	UnhookEvent(hHookIdleChanged);
	/* Transfer Shutdown */
	UnhookEvent(hHookProtoAck);
	mir_free(transfers); /* does NULL check */
	/* Weather Shutdown */
	UnhookEvent(hHookWeatherUpdated); /* does NULL check */
	/* Services */
	DestroyHookableEvent(hEventWatcherChanged);
	/* Misc */
	UnhookEvent(hHookModulesLoaded);
}
