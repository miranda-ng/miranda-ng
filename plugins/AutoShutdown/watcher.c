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
/* Overheat Shutdown */
static HANDLE hHookHddOverheat;
/* Services */
static HANDLE hServiceStartWatcher,hServiceStopWatcher,hServiceIsEnabled;
static HANDLE hEventWatcherChanged;
/* Misc */
static HANDLE hHookModulesLoaded;

/************************* Shared *************************************/

static WORD currentWatcherType;

static void __stdcall MainThreadMapping(HANDLE *phDoneEvent)
{
	ServiceShutdown(0,TRUE); /* ensure main thread (for cpu usage shutdown) */
	ServiceStopWatcher(0,0);
	if(*phDoneEvent!=NULL) SetEvent(*phDoneEvent);
}

static void __inline ShutdownAndStopWatcher(void)
{
	HANDLE hDoneEvent;
	hDoneEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	if(CallFunctionAsync(MainThreadMapping,&hDoneEvent))
		if(hDoneEvent!=NULL) WaitForSingleObject(hDoneEvent,INFINITE);
	if(hDoneEvent!=NULL) CloseHandle(hDoneEvent);
}

/************************* Msg Shutdown *******************************/

// ppBlob might get reallocated, must have been allocated using mir_alloc()
static TCHAR* GetMessageText(BYTE **ppBlob,DWORD *pcbBlob)
{
#if defined(_UNICODE)
	DWORD cb;
	(*ppBlob)[*pcbBlob]=0;
	cb=lstrlenA((char*)*ppBlob);
	/* use Unicode data if present */
	if(*pcbBlob>(cb+3)) {
		(*ppBlob)[*pcbBlob-1]=0;
		return (WCHAR*)&(*ppBlob)[cb];
	}
	/* no Unicode data present, convert from ANSI */
	{ 	int len;
		BYTE *buf;
		len=MultiByteToWideChar(CP_ACP,0,(char*)*ppBlob,-1,NULL,0);
		if(!len) return NULL;
		buf=(BYTE*)mir_realloc(*ppBlob,(*pcbBlob)+(len*sizeof(WCHAR)));
		if(buf==NULL) return NULL;
		*pcbBlob+=len*sizeof(WCHAR);
		*ppBlob=buf;
		buf=&(*ppBlob)[cb];
		MultiByteToWideChar(CP_ACP,0,(char*)*ppBlob,-1,(WCHAR*)buf,len);
		((WCHAR*)buf)[len-1]=0;
		return (WCHAR*)buf;
	}
#else
	(*ppBlob)[*pcbBlob]=0;
	return (char*)*ppBlob;
#endif
}

static int MsgEventAdded(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	if(currentWatcherType&SDWTF_MESSAGE) {
		DBEVENTINFO dbe;
		dbe.cbSize=sizeof(dbe);
		dbe.cbBlob=(DWORD)CallService(MS_DB_EVENT_GETBLOBSIZE,(WPARAM)lParam,0);
		dbe.pBlob=(BYTE*)mir_alloc(dbe.cbBlob+2); /* ensure term zero */
		if(dbe.pBlob==NULL) return 0;
		if(!CallService(MS_DB_EVENT_GET,(WPARAM)lParam,(LPARAM)&dbe))
			if(dbe.eventType==EVENTTYPE_MESSAGE && !(dbe.flags&DBEF_SENT)) {
				DBVARIANT dbv;
				TCHAR *pszMsg;
				if(!DBGetContactSettingTString(NULL,"AutoShutdown","Message",&dbv)) {
					TrimString(dbv.ptszVal);
					pszMsg=GetMessageText(&dbe.pBlob,&dbe.cbBlob);
					if(pszMsg!=NULL && _tcsstr(pszMsg,dbv.ptszVal)!=NULL)
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
	UNREFERENCED_PARAMETER(wParam);
	if(ack->type==ACKTYPE_FILE)
		switch(ack->result) {
			case ACKRESULT_DATA:
			{	int i;
				for(i=0;i<nTransfersCount;++i)
					if(transfers[i]==ack->hProcess)
						break; /* already in list */
				/* insert into list */
				{	HANDLE *buf=(HANDLE*)mir_realloc(transfers,(nTransfersCount+1)*sizeof(HANDLE));				if(buf!=NULL) {
						transfers=buf;
						transfers[nTransfersCount]=ack->hProcess;
						++nTransfersCount;
					}
				}
				break;
			}
			case ACKRESULT_SUCCESS:
			case ACKRESULT_FAILED:
			case ACKRESULT_DENIED:
			{	int i;
				for(i=0;i<nTransfersCount;++i)
					if(transfers[i]==ack->hProcess) {
						/* remove from list */
						HANDLE *buf;
						if(i<(nTransfersCount-1))
							MoveMemory(&transfers[i],&transfers[i+1],(nTransfersCount-i-1)*sizeof(HANDLE));
						--nTransfersCount;
						buf=(HANDLE*)mir_realloc(transfers,nTransfersCount*sizeof(HANDLE));
						if(buf!=NULL) transfers=buf;
						else if(!nTransfersCount) transfers=NULL;
						/* stop watcher */
						if(!nTransfersCount && (currentWatcherType&SDWTF_FILETRANSFER))
							ShutdownAndStopWatcher();
						break;
					}
				break;
			}
		}
	return 0;
}

/************************* Idle Shutdown ******************************/

static int IdleChanged(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	if(currentWatcherType&SDWTF_IDLE && lParam&IDF_ISIDLE)
		ShutdownAndStopWatcher();
	return 0;
}

/************************* Status Shutdown ****************************/

static BOOL CheckAllContactsOffline(void)
{
	BOOL fSmartCheck,fAllOffline=TRUE; /* tentatively */
	HANDLE hContact;
	char *pszProto;
	fSmartCheck=DBGetContactSettingByte(NULL,"AutoShutdown","SmartOfflineCheck",SETTING_SMARTOFFLINECHECK_DEFAULT);
	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hContact!=NULL) {
		pszProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
		if(pszProto!=NULL && CallProtoService(pszProto,PS_GETSTATUS,0,0)!=ID_STATUS_OFFLINE)
			if(DBGetContactSettingByte(hContact,pszProto,"ChatRoom",0)) continue;
			if(DBGetContactSettingWord(hContact,pszProto,"Status",0)!=ID_STATUS_OFFLINE) {
				if(fSmartCheck) {
					if(DBGetContactSettingByte(hContact,"CList","Hidden",0)) continue;
					if(DBGetContactSettingByte(hContact,"CList","NotOnList",0)) continue;
				}
				fAllOffline=FALSE;
				break;
			}
		hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
	}
	return fAllOffline;
}

static int StatusSettingChanged(WPARAM wParam,LPARAM lParam)
{
	if(currentWatcherType&SDWTF_STATUS) {
		DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING*)lParam;
 		if((HANDLE)wParam!=NULL && dbcws->value.wVal==ID_STATUS_OFFLINE && !lstrcmpA(dbcws->szSetting,"Status")) {
			char *pszProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);
			if(pszProto!=NULL && !lstrcmpA(dbcws->szModule,pszProto))
				if(CheckAllContactsOffline())
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
	if(idCpuUsageThread!=GetCurrentThreadId()) {
		nTimesBelow=0;
		return FALSE; /* stop poll thread */
	}
	/* ignore random peaks */
	if(nCpuUsage<(BYTE)lParam) ++nTimesBelow;
	else nTimesBelow=0;
	if(nTimesBelow==3) {
		nTimesBelow=0;
		ShutdownAndStopWatcher();
		return FALSE; /* stop poll thread */
	}
	return TRUE;
}

/************************* Weather Shutdown ***************************/

static int WeatherUpdated(WPARAM wParam,LPARAM lParam)
{
	char *pszProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);
	if((BOOL)lParam && pszProto!=NULL && CallProtoService(pszProto,PS_GETSTATUS,0,0)==THUNDER)
		if(DBGetContactSettingByte(NULL,"AutoShutdown","WeatherShutdown",SETTING_WEATHERSHUTDOWN_DEFAULT))
			ServiceShutdown(SDSDT_SHUTDOWN,TRUE);
	return 0;
}

/************************* Overheat Shutdown **************************/

static int HddOverheat(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if(DBGetContactSettingByte(NULL,"AutoShutdown","HddOverheatShutdown",SETTING_HDDOVERHEATSHUTDOWN_DEFAULT))
		ServiceShutdown(SDSDT_SHUTDOWN,TRUE);
	return 0;
}

/************************* Services ***********************************/

int ServiceStartWatcher(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	/* passing watcherType as lParam is only to be used internally, undocumented */
	if(lParam==0) lParam=(LPARAM)DBGetContactSettingWord(NULL,"AutoShutdown","WatcherFlags",0);

	if(!(lParam&SDWTF_MASK)) return 1; /* invalid flags or empty? */
	if(lParam&SDWTF_SPECIFICTIME && !(lParam&SDWTF_ST_MASK)) return 2; /* no specific time choice? */
	if(currentWatcherType==(WORD)lParam) return 3;

	if(currentWatcherType!=0) {
		/* Time Shutdown */
		CloseCountdownFrame(); /* fails if not opened */
		/* Cpu Shutdown */
		idCpuUsageThread=0;
	}
	SetShutdownMenuItem(TRUE);
	SetShutdownToolbarButton(TRUE);
	currentWatcherType=(WORD)lParam;
	NotifyEventHooks(hEventWatcherChanged,TRUE,0);

	/* Time Shutdown */
	if(currentWatcherType&SDWTF_SPECIFICTIME)
		ShowCountdownFrame(currentWatcherType); /* after modules loaded */
	/* Cpu Shutdown */
	if(currentWatcherType&SDWTF_CPUUSAGE)
		idCpuUsageThread=PollCpuUsage(CpuUsageWatcherProc,(LPARAM)DBGetContactSettingRangedByte(NULL,"AutoShutdown","CpuUsageThreshold",SETTING_CPUUSAGETHRESHOLD_DEFAULT,1,100),1500);
	/* Transfer Shutdown */
	if(currentWatcherType&SDWTF_FILETRANSFER && !nTransfersCount)
		ShutdownAndStopWatcher();
	/* Status Shutdown */
	if(currentWatcherType&SDWTF_STATUS && CheckAllContactsOffline())
		ShutdownAndStopWatcher();
	return 0;
}

int ServiceStopWatcher(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if(currentWatcherType==0) return 1;

	/* Time Shutdown */
	if(currentWatcherType&SDWTF_SPECIFICTIME)
		CloseCountdownFrame();
	/* Cpu Shutdown */
	idCpuUsageThread=0;

	currentWatcherType=0;
	SetShutdownMenuItem(FALSE);
	SetShutdownToolbarButton(FALSE);
	NotifyEventHooks(hEventWatcherChanged,FALSE,0);
	return 0;
}

int ServiceIsWatcherEnabled(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	return currentWatcherType!=0;
}

/************************* Misc ***********************************/

static int WatcherModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	/* Weather Shutdown */
	if(ServiceExists(MS_WEATHER_UPDATE))
		hHookWeatherUpdated=HookEvent(ME_WEATHER_UPDATED,WeatherUpdated);
	/* Overheat Shutdown */
	if(ServiceExists(MS_SYSINFO_HDDTEMP))
		hHookHddOverheat=HookEvent(ME_SYSINFO_HDDOVERHEAT,HddOverheat);

	/* restore watcher if it was running on last exit */
	if(DBGetContactSettingByte(NULL,"AutoShutdown","RememberOnRestart",0)==SDROR_RUNNING) {
		DBWriteContactSettingByte(NULL,"AutoShutdown","RememberOnRestart",1);
		ServiceStartWatcher(0,0); /* after modules loaded */
	}
	return 0;
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
	/* Overheat Shutdown */
	hHookHddOverheat=NULL;
	/* Services */
	hEventWatcherChanged=CreateHookableEvent(ME_AUTOSHUTDOWN_WATCHERCHANGED);
	hServiceStartWatcher=CreateServiceFunction(MS_AUTOSHUTDOWN_STARTWATCHER,ServiceStartWatcher);
 	hServiceStopWatcher=CreateServiceFunction(MS_AUTOSHUTDOWN_STOPWATCHER,ServiceStopWatcher);
	hServiceIsEnabled=CreateServiceFunction(MS_AUTOSHUTDOWN_ISWATCHERENABLED,ServiceIsWatcherEnabled);
	/* Misc */
	hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,WatcherModulesLoaded);
}

void UninitWatcher(void)
{
	/* remember watcher if running */
	if(!ServiceStopWatcher(0,0))
		if(DBGetContactSettingByte(NULL,"AutoShutdown","RememberOnRestart",SETTING_REMEMBERONRESTART_DEFAULT))
			DBWriteContactSettingByte(NULL,"AutoShutdown","RememberOnRestart",SDROR_RUNNING);

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
	/* Overheat Shutdown */
	UnhookEvent(hHookHddOverheat); /* does NULL check */
	/* Services */
	DestroyServiceFunction(hServiceStartWatcher);
	DestroyServiceFunction(hServiceStopWatcher);
	DestroyServiceFunction(hServiceIsEnabled);
	DestroyHookableEvent(hEventWatcherChanged);
	/* Misc */
	UnhookEvent(hHookModulesLoaded);
}
