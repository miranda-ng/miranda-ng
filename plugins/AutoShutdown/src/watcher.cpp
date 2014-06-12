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

#include "common.h"

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
static HANDLE hServiceStartWatcher,hServiceStopWatcher,hServiceIsEnabled;
static HANDLE hEventWatcherChanged;
/* Misc */
static HANDLE hHookModulesLoaded;

/************************* Shared *************************************/

static WORD currentWatcherType;

static void __stdcall MainThreadMapping(void *param)
{
	HANDLE *phDoneEvent = (HANDLE*)param;
	ServiceShutdown(0,TRUE); /* ensure main thread (for cpu usage shutdown) */
	ServiceStopWatcher(0,0);
	if (*phDoneEvent != NULL)
		SetEvent(*phDoneEvent);
}

static void __inline ShutdownAndStopWatcher(void)
{
	HANDLE hDoneEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	CallFunctionAsync(MainThreadMapping, &hDoneEvent);
	if (hDoneEvent != NULL) {
		WaitForSingleObject(hDoneEvent,INFINITE);
		CloseHandle(hDoneEvent);
	}
}

/************************* Msg Shutdown *******************************/

// ppBlob might get reallocated, must have been allocated using mir_alloc()
static TCHAR* GetMessageText(BYTE **ppBlob,DWORD *pcbBlob)
{
	(*ppBlob)[*pcbBlob]=0;
	DWORD cb = lstrlenA((char*)*ppBlob);
	/* use Unicode data if present */
	if (*pcbBlob>(cb+3)) {
		(*ppBlob)[*pcbBlob-1]=0;
		return (WCHAR*)&(*ppBlob)[cb];
	}
	/* no Unicode data present, convert from ANSI */
	int len = MultiByteToWideChar(CP_ACP,0,(char*)*ppBlob,-1,NULL,0);
	if (!len)
		return NULL;

	BYTE *buf=(BYTE*)mir_realloc(*ppBlob,(*pcbBlob)+(len*sizeof(WCHAR)));
	if (buf == NULL)
		return NULL;

	*pcbBlob += len*sizeof(WCHAR);
	*ppBlob = buf;
	buf = &(*ppBlob)[cb];
	MultiByteToWideChar(CP_ACP,0,(char*)*ppBlob,-1,(WCHAR*)buf,len);
	((WCHAR*)buf)[len-1] = 0;
	return (WCHAR*)buf;
}

static int MsgEventAdded(WPARAM wParam,LPARAM lParam)
{
	HANDLE hDbEvent = (HANDLE)lParam;

	if (currentWatcherType & SDWTF_MESSAGE) {
		DBEVENTINFO dbe = { sizeof(dbe) };
		dbe.cbBlob = db_event_getBlobSize(hDbEvent);
		dbe.pBlob = (BYTE*)mir_alloc(dbe.cbBlob+2); /* ensure term zero */
		if (dbe.pBlob == NULL)
			return 0;
		if (!db_event_get(hDbEvent, &dbe))
			if (dbe.eventType == EVENTTYPE_MESSAGE && !(dbe.flags & DBEF_SENT)) {
				DBVARIANT dbv;
				if (!db_get_ts(NULL,"AutoShutdown","Message",&dbv)) {
					TrimString(dbv.ptszVal);
					TCHAR *pszMsg = GetMessageText(&dbe.pBlob,&dbe.cbBlob);
					if (pszMsg != NULL && _tcsstr(pszMsg,dbv.ptszVal) != NULL)
						ShutdownAndStopWatcher(); /* msg with specified text recvd */
					mir_free(dbv.ptszVal); /* does NULL check */
				}
			}
		mir_free(dbe.pBlob);
	}
	return 0;
}

/************************* Transfer Shutdown **************************/

static HANDLE *transfers;
static int nTransfersCount;

static int ProtoAck(WPARAM wParam,LPARAM lParam)
{
	ACKDATA *ack=(ACKDATA*)lParam;
	if (ack->type != ACKTYPE_FILE)
		return 0;

	switch(ack->result) {
	case ACKRESULT_DATA:
		{
			for(int i=0; i < nTransfersCount; ++i)
				if (transfers[i]==ack->hProcess)
					break; /* already in list */
			/* insert into list */
			HANDLE *buf = (HANDLE*)mir_realloc(transfers,(nTransfersCount+1)*sizeof(HANDLE));
			if (buf != NULL) {
				transfers = buf;
				transfers[nTransfersCount] = ack->hProcess;
				++nTransfersCount;
			}
			break;
		}
	case ACKRESULT_SUCCESS:
	case ACKRESULT_FAILED:
	case ACKRESULT_DENIED:
		for(int i=0;i<nTransfersCount;++i) {
			if (transfers[i]==ack->hProcess) {
				/* remove from list */
				if (i<(nTransfersCount-1))
					MoveMemory(&transfers[i],&transfers[i+1],(nTransfersCount-i-1)*sizeof(HANDLE));
				--nTransfersCount;
				HANDLE *buf = (HANDLE*)mir_realloc(transfers,nTransfersCount*sizeof(HANDLE));
				if (buf != NULL) transfers=buf;
				else if (!nTransfersCount) transfers=NULL;
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

static int IdleChanged(WPARAM,LPARAM lParam)
{
	if (currentWatcherType&SDWTF_IDLE && lParam&IDF_ISIDLE)
		ShutdownAndStopWatcher();
	return 0;
}

/************************* Status Shutdown ****************************/

static BOOL CheckAllContactsOffline(void)
{
	BOOL fSmartCheck,fAllOffline=TRUE; /* tentatively */
	fSmartCheck=db_get_b(NULL,"AutoShutdown","SmartOfflineCheck",SETTING_SMARTOFFLINECHECK_DEFAULT);
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *pszProto = GetContactProto(hContact);
		if (pszProto != NULL && CallProtoService(pszProto,PS_GETSTATUS,0,0) != ID_STATUS_OFFLINE) {
			if (db_get_b(hContact,pszProto,"ChatRoom",0)) continue;
			if (db_get_w(hContact,pszProto,"Status",0) != ID_STATUS_OFFLINE) {
				if (fSmartCheck) {
					if (db_get_b(hContact,"CList","Hidden",0)) continue;
					if (db_get_b(hContact,"CList","NotOnList",0)) continue;
				}
				fAllOffline=FALSE;
				break;
			}
		}
	}
	return fAllOffline;
}

static int StatusSettingChanged(WPARAM wParam,LPARAM lParam)
{
	if (currentWatcherType&SDWTF_STATUS) {
		DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING*)lParam;
 		if ((HANDLE)wParam != NULL && dbcws->value.wVal==ID_STATUS_OFFLINE && !lstrcmpA(dbcws->szSetting,"Status")) {
			char *pszProto = GetContactProto(wParam);
			if (pszProto != NULL && !lstrcmpA(dbcws->szModule,pszProto))
				if (CheckAllContactsOffline())
					ShutdownAndStopWatcher();
		}
	}
	return 0;
}

/************************* Cpu Shutdown *******************************/

static DWORD idCpuUsageThread;

static BOOL CALLBACK CpuUsageWatcherProc(BYTE nCpuUsage,LPARAM lParam)
{
	static BYTE nTimesBelow=0; /* only one watcher thread */
	/* terminated? */
	if (idCpuUsageThread != GetCurrentThreadId()) {
		nTimesBelow=0;
		return FALSE; /* stop poll thread */
	}
	/* ignore random peaks */
	if (nCpuUsage<(BYTE)lParam) ++nTimesBelow;
	else nTimesBelow=0;
	if (nTimesBelow==3) {
		nTimesBelow=0;
		ShutdownAndStopWatcher();
		return FALSE; /* stop poll thread */
	}
	return TRUE;
}

/************************* Weather Shutdown ***************************/

static int WeatherUpdated(WPARAM wParam,LPARAM lParam)
{
	char *pszProto = GetContactProto(wParam);
	if ((BOOL)lParam && pszProto != NULL && CallProtoService(pszProto,PS_GETSTATUS,0,0)==THUNDER)
		if (db_get_b(NULL,"AutoShutdown","WeatherShutdown",SETTING_WEATHERSHUTDOWN_DEFAULT))
			ServiceShutdown(SDSDT_SHUTDOWN,TRUE);
	return 0;
}

/************************* Services ***********************************/

INT_PTR ServiceStartWatcher(WPARAM wParam,LPARAM lParam)
{
	/* passing watcherType as lParam is only to be used internally, undocumented */
	if (lParam==0)
		lParam=(LPARAM)db_get_w(NULL,"AutoShutdown","WatcherFlags",0);

	/* invalid flags or empty? */
	if (!(lParam&SDWTF_MASK))
		return 1;

	/* no specific time choice? */
	if (lParam&SDWTF_SPECIFICTIME && !(lParam&SDWTF_ST_MASK))
		return 2;

	if (currentWatcherType==(WORD)lParam)
		return 3;

	if (currentWatcherType != 0) {
		/* Time Shutdown */
		CloseCountdownFrame(); /* fails if not opened */
		/* Cpu Shutdown */
		idCpuUsageThread=0;
	}
	SetShutdownMenuItem(true);
	SetShutdownToolbarButton(true);
	currentWatcherType=(WORD)lParam;
	NotifyEventHooks(hEventWatcherChanged,TRUE,0);

	/* Time Shutdown */
	if (currentWatcherType&SDWTF_SPECIFICTIME)
		ShowCountdownFrame(currentWatcherType); /* after modules loaded */
	/* Cpu Shutdown */
	if (currentWatcherType&SDWTF_CPUUSAGE)
		idCpuUsageThread=PollCpuUsage(CpuUsageWatcherProc,(LPARAM)DBGetContactSettingRangedByte(NULL,"AutoShutdown","CpuUsageThreshold",SETTING_CPUUSAGETHRESHOLD_DEFAULT,1,100),1500);
	/* Transfer Shutdown */
	if (currentWatcherType&SDWTF_FILETRANSFER && !nTransfersCount)
		ShutdownAndStopWatcher();
	/* Status Shutdown */
	if (currentWatcherType&SDWTF_STATUS && CheckAllContactsOffline())
		ShutdownAndStopWatcher();
	return 0;
}

INT_PTR ServiceStopWatcher(WPARAM,LPARAM)
{
	if (currentWatcherType==0) return 1;

	/* Time Shutdown */
	if (currentWatcherType&SDWTF_SPECIFICTIME)
		CloseCountdownFrame();
	/* Cpu Shutdown */
	idCpuUsageThread=0;

	currentWatcherType=0;
	SetShutdownMenuItem(FALSE);
	SetShutdownToolbarButton(FALSE);
	NotifyEventHooks(hEventWatcherChanged,FALSE,0);
	return 0;
}

INT_PTR ServiceIsWatcherEnabled(WPARAM,LPARAM)
{
	return currentWatcherType != 0;
}

/************************* Misc ***********************************/

void WatcherModulesLoaded(void)
{
	/* Weather Shutdown */
	if (ServiceExists(MS_WEATHER_UPDATE))
		hHookWeatherUpdated=HookEvent(ME_WEATHER_UPDATED,WeatherUpdated);

	/* restore watcher if it was running on last exit */
	if (db_get_b(NULL,"AutoShutdown","RememberOnRestart",0)==SDROR_RUNNING) {
		db_set_b(NULL,"AutoShutdown","RememberOnRestart",1);
		ServiceStartWatcher(0,0); /* after modules loaded */
	}
}

void InitWatcher(void)
{
	/* Shared */
	currentWatcherType=0;
	/* Message Shutdown */
	hHookEventAdded=HookEvent(ME_DB_EVENT_ADDED,MsgEventAdded);
	/* Status Shutdown*/
	hHookSettingChanged=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,StatusSettingChanged);
	/* Idle Shutdown */
	hHookIdleChanged=HookEvent(ME_IDLE_CHANGED,IdleChanged);
	/* Transfer Shutdown */
	transfers=NULL;
	nTransfersCount=0;
	hHookProtoAck=HookEvent(ME_PROTO_ACK,ProtoAck);
	/* Weather Shutdown */
	hHookWeatherUpdated=NULL;
	/* Services */
	hEventWatcherChanged=CreateHookableEvent(ME_AUTOSHUTDOWN_WATCHERCHANGED);
	hServiceStartWatcher = CreateServiceFunction(MS_AUTOSHUTDOWN_STARTWATCHER, ServiceStartWatcher);
 	hServiceStopWatcher = CreateServiceFunction(MS_AUTOSHUTDOWN_STOPWATCHER, ServiceStopWatcher);
	hServiceIsEnabled = CreateServiceFunction(MS_AUTOSHUTDOWN_ISWATCHERENABLED, ServiceIsWatcherEnabled);
}

void UninitWatcher(void)
{
	/* remember watcher if running */
	if (!ServiceStopWatcher(0,0))
		if (db_get_b(NULL,"AutoShutdown","RememberOnRestart",SETTING_REMEMBERONRESTART_DEFAULT))
			db_set_b(NULL,"AutoShutdown","RememberOnRestart",SDROR_RUNNING);

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
	DestroyServiceFunction(hServiceStartWatcher);
	DestroyServiceFunction(hServiceStopWatcher);
	DestroyServiceFunction(hServiceIsEnabled);
	DestroyHookableEvent(hEventWatcherChanged);
	/* Misc */
	UnhookEvent(hHookModulesLoaded);
}
