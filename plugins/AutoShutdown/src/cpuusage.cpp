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

/************************* Stat Switch ********************************/

#define Li2Double(x) ((double)((x).HighPart)*4.294967296E9+(double)((x).LowPart)) 

static BOOL WinNT_PerfStatsSwitch(TCHAR *pszServiceName,BOOL fDisable)
{ 
	HKEY hKeyServices,hKeyService,hKeyPerf;
	DWORD dwData,dwDataSize;
	BOOL fSwitched=FALSE;
	/* Win2000+ */
	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,_T("System\\CurrentControlSet\\Services"),0,KEY_QUERY_VALUE|KEY_SET_VALUE,&hKeyServices)) {
		if (!RegOpenKeyEx(hKeyServices,pszServiceName,0,KEY_QUERY_VALUE|KEY_SET_VALUE,&hKeyService)) {
			if (!RegOpenKeyEx(hKeyService,_T("Performance"),0,KEY_QUERY_VALUE|KEY_SET_VALUE,&hKeyPerf)) {
				dwDataSize=sizeof(DWORD);
				if (!RegQueryValueEx(hKeyPerf,_T("Disable Performance Counters"),NULL,NULL,(BYTE*)&dwData,&dwDataSize))
					if ((dwData != 0) != fDisable)
						fSwitched=!RegSetValueEx(hKeyPerf,_T("Disable Performance Counters"),0,REG_DWORD,(BYTE*)&fDisable,dwDataSize);
				RegCloseKey(hKeyPerf);
			}
			RegCloseKey(hKeyService);
		}
		RegCloseKey(hKeyServices);
	}
	return fSwitched;
}

/************************* Poll Thread ********************************/
struct CpuUsageThreadParams {
	DWORD dwDelayMillis;
	CPUUSAGEAVAILPROC pfnDataAvailProc;
	LPARAM lParam;
	HANDLE hFirstEvent;
	DWORD *pidThread;
};

static BOOL CallBackAndWait(struct CpuUsageThreadParams *param,BYTE nCpuUsage)
{
	if (param->hFirstEvent != NULL) {
		/* return value for PollCpuUsage() */
		*param->pidThread=GetCurrentThreadId();
		SetEvent(param->hFirstEvent);
		param->hFirstEvent=NULL;
		/* lower priority after first call */
		SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_IDLE);
	}
	if (!param->pfnDataAvailProc(nCpuUsage,param->lParam)) return FALSE;
	SleepEx(param->dwDelayMillis,TRUE);
	return !Miranda_Terminated();
}

static void WinNT_PollThread(void *vparam)
{
	CpuUsageThreadParams *param = (CpuUsageThreadParams*)vparam;

	DWORD dwBufferSize=0,dwCount;
	BYTE *pBuffer=NULL;
	PERF_DATA_BLOCK *pPerfData=NULL; 
	LONG res,lCount;
	PERF_OBJECT_TYPE *pPerfObj;
	PERF_COUNTER_DEFINITION *pPerfCounter; 
	PERF_INSTANCE_DEFINITION *pPerfInstance; 
	PERF_COUNTER_BLOCK *pPerfCounterBlock;
	DWORD dwObjectId,dwCounterId; 
	WCHAR wszValueName[11],*pwszInstanceName;
	BYTE nCpuUsage;
	BOOL fSwitched,fFound,fIsFirst=FALSE;
	LARGE_INTEGER liPrevCounterValue={0},liCurrentCounterValue={0},liPrevPerfTime100nSec={0};

	/* init */
	dwObjectId=238;             /*'Processor' object */
	dwCounterId=6;              /* '% processor time' counter */
	pwszInstanceName=L"_Total"; /* '_Total' instance */
	_itot_s(dwObjectId, wszValueName, 10);
	fSwitched = WinNT_PerfStatsSwitch(_T("PerfOS"), FALSE);

	/* poll */
	for(;;) {
		/* retrieve data for given object */
		res=RegQueryValueExW(HKEY_PERFORMANCE_DATA,wszValueName,NULL,NULL,(BYTE*)pPerfData,&dwBufferSize);
		while(!pBuffer || res==ERROR_MORE_DATA) {
			pBuffer=(BYTE*)mir_realloc(pPerfData,dwBufferSize+=256);
			if (!pBuffer) break;
			pPerfData=(PERF_DATA_BLOCK*)pBuffer;
			res=RegQueryValueExW(HKEY_PERFORMANCE_DATA,wszValueName,NULL,NULL,pBuffer,&dwBufferSize);
		}
		if (res != ERROR_SUCCESS) break;

		/* find object in data */
		fFound=FALSE;
		/* first object */
		pPerfObj=(PERF_OBJECT_TYPE*)((BYTE*)pPerfData+pPerfData->HeaderLength);
		for(dwCount=0;dwCount<pPerfData->NumObjectTypes;++dwCount) {
			if (pPerfObj->ObjectNameTitleIndex==dwObjectId) {
				/* find counter in object data */
				/* first counter */
				pPerfCounter=(PERF_COUNTER_DEFINITION*)((BYTE*)pPerfObj+pPerfObj->HeaderLength);
				for(dwCount=0;dwCount<(pPerfObj->NumCounters);++dwCount) { 
					if (pPerfCounter->CounterNameTitleIndex==dwCounterId) {
						/* find instance in counter data */
						if (pPerfObj->NumInstances==PERF_NO_INSTANCES) { 
							pPerfCounterBlock=(PERF_COUNTER_BLOCK*)((BYTE*)pPerfObj+pPerfObj->DefinitionLength);
							liCurrentCounterValue=*(LARGE_INTEGER*)((BYTE*)pPerfCounterBlock+pPerfCounter->CounterOffset);
							fFound=TRUE;
						}
						else {
							/* first instance */
							pPerfInstance=(PERF_INSTANCE_DEFINITION*)((BYTE*)pPerfObj+pPerfObj->DefinitionLength);
							for(lCount=0;lCount<(pPerfObj->NumInstances);++lCount) {
								pPerfCounterBlock=(PERF_COUNTER_BLOCK*)((BYTE*)pPerfInstance+pPerfInstance->ByteLength);
								if (!mir_wstrcmpi(pwszInstanceName,(WCHAR*)((BYTE*)pPerfInstance+pPerfInstance->NameOffset))) {
									liCurrentCounterValue=*(LARGE_INTEGER*)((BYTE*)pPerfCounterBlock+pPerfCounter->CounterOffset);
									fFound=TRUE;
									break;
								}
								/* next instance */
								pPerfInstance=(PPERF_INSTANCE_DEFINITION)((BYTE*)pPerfCounterBlock+pPerfCounterBlock->ByteLength);
							} 
						} 
						break;
					}
					/* next counter */
					pPerfCounter=(PERF_COUNTER_DEFINITION*)((BYTE*)pPerfCounter+pPerfCounter->ByteLength);
				}
				break; 
			} 
			/* next object */
			pPerfObj=(PERF_OBJECT_TYPE*)((BYTE*)pPerfObj+pPerfObj->TotalByteLength);
		}
		if (!fFound) break;

		/* calc val from data, we need two samplings
		 * counter type: PERF_100NSEC_TIMER_INV
		 * calc: time base=100Ns, value=100*(1-(data_diff)/(100NsTime_diff)) */
		if (!fIsFirst) {
			nCpuUsage=(BYTE)((1.0-(Li2Double(liCurrentCounterValue)-Li2Double(liPrevCounterValue))/(Li2Double(pPerfData->PerfTime100nSec)-Li2Double(liPrevPerfTime100nSec)))*100.0+0.5);
			if (!CallBackAndWait(param,nCpuUsage)) break;
		}
		else fIsFirst=FALSE;
		/* store current sampling for next */
		memcpy(&liPrevCounterValue,&liCurrentCounterValue,sizeof(LARGE_INTEGER)); 
		memcpy(&liPrevPerfTime100nSec,&pPerfData->PerfTime100nSec,sizeof(LARGE_INTEGER)); 
	}

	/* uninit */
	if (pPerfData) mir_free(pPerfData);
	if (fSwitched) WinNT_PerfStatsSwitch(_T("PerfOS"),TRUE);

	/* return error for PollCpuUsage() if never succeeded */
	if (param->hFirstEvent != NULL) SetEvent(param->hFirstEvent);
	mir_free(param);
} 

/************************* Start Thread *******************************/

// returns poll thread id on success
DWORD PollCpuUsage(CPUUSAGEAVAILPROC pfnDataAvailProc,LPARAM lParam,DWORD dwDelayMillis)
{
	struct CpuUsageThreadParams *param;
	DWORD idThread=0;
	HANDLE hFirstEvent;

	/* init params */
	param=(struct CpuUsageThreadParams*)mir_alloc(sizeof(struct CpuUsageThreadParams));
	if (param==NULL) return FALSE;
	param->dwDelayMillis=dwDelayMillis;
	param->pfnDataAvailProc=pfnDataAvailProc;
	param->lParam=lParam;
	param->pidThread=&idThread;
	param->hFirstEvent=hFirstEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	if (hFirstEvent==NULL) {
		mir_free(param);
		return 0;
	}
	/* start thread */
	if ((int)mir_forkthread(WinNT_PollThread, param) != -1)
		WaitForSingleObject(hFirstEvent,INFINITE); /* wait for first success */
	else
		mir_free(param); /* thread not started */
	CloseHandle(hFirstEvent);
	return idThread;
}
