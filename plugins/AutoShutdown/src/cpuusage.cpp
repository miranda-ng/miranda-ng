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

/************************* Stat Switch ********************************/

#define Li2Double(x) ((double)((x).HighPart)*4.294967296E9+(double)((x).LowPart)) 

static BOOL WinNT_PerfStatsSwitch(wchar_t *pszServiceName, BOOL fDisable)
{
	HKEY hKeyServices, hKeyService, hKeyPerf;
	DWORD dwData, dwDataSize;
	BOOL fSwitched = FALSE;
	/* Win2000+ */
	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hKeyServices)) {
		if (!RegOpenKeyEx(hKeyServices, pszServiceName, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hKeyService)) {
			if (!RegOpenKeyEx(hKeyService, L"Performance", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hKeyPerf)) {
				dwDataSize = sizeof(uint32_t);
				if (!RegQueryValueEx(hKeyPerf, L"Disable Performance Counters", nullptr, nullptr, (uint8_t*)&dwData, &dwDataSize))
					if ((dwData != 0) != fDisable)
						fSwitched = !RegSetValueEx(hKeyPerf, L"Disable Performance Counters", 0, REG_DWORD, (uint8_t*)&fDisable, dwDataSize);
				RegCloseKey(hKeyPerf);
			}
			RegCloseKey(hKeyService);
		}
		RegCloseKey(hKeyServices);
	}
	return fSwitched;
}

/************************* Poll Thread ********************************/
struct CpuUsageThreadParams
{
	uint32_t dwDelayMillis;
	CPUUSAGEAVAILPROC pfnDataAvailProc;
	LPARAM lParam;
	HANDLE hFirstEvent;
	uint32_t *pidThread;
};

static BOOL CallBackAndWait(struct CpuUsageThreadParams *param, uint8_t nCpuUsage)
{
	if (param->hFirstEvent != nullptr) {
		/* return value for PollCpuUsage() */
		*param->pidThread = GetCurrentThreadId();
		SetEvent(param->hFirstEvent);
		param->hFirstEvent = nullptr;
		/* lower priority after first call */
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	}
	if (!param->pfnDataAvailProc(nCpuUsage, param->lParam)) return FALSE;
	SleepEx(param->dwDelayMillis, TRUE);
	return !Miranda_IsTerminated();
}

static void WinNT_PollThread(CpuUsageThreadParams *param)
{
	DWORD dwBufferSize = 0, dwCount;
	uint8_t *pBuffer = nullptr;
	PERF_DATA_BLOCK *pPerfData = nullptr;
	LONG res, lCount;
	PERF_OBJECT_TYPE *pPerfObj;
	PERF_COUNTER_DEFINITION *pPerfCounter;
	PERF_INSTANCE_DEFINITION *pPerfInstance;
	PERF_COUNTER_BLOCK *pPerfCounterBlock;
	uint32_t dwObjectId, dwCounterId;
	wchar_t wszValueName[11], *pwszInstanceName;
	uint8_t nCpuUsage;
	BOOL fSwitched, fFound, fIsFirst = FALSE;
	LARGE_INTEGER liPrevCounterValue = { 0 }, liCurrentCounterValue = { 0 }, liPrevPerfTime100nSec = { 0 };

	/* init */
	dwObjectId = 238;             /*'Processor' object */
	dwCounterId = 6;              /* '% processor time' counter */
	pwszInstanceName = L"_Total"; /* '_Total' instance */
	_itow_s(dwObjectId, wszValueName, 10);
	fSwitched = WinNT_PerfStatsSwitch(L"PerfOS", FALSE);

	/* poll */
	for (;;) {
		/* retrieve data for given object */
		res = RegQueryValueExW(HKEY_PERFORMANCE_DATA, wszValueName, nullptr, nullptr, (uint8_t*)pPerfData, &dwBufferSize);
		while (!pBuffer || res == ERROR_MORE_DATA) {
			pBuffer = (uint8_t*)mir_realloc(pPerfData, dwBufferSize += 256);
			if (!pBuffer) break;
			pPerfData = (PERF_DATA_BLOCK*)pBuffer;
			res = RegQueryValueExW(HKEY_PERFORMANCE_DATA, wszValueName, nullptr, nullptr, pBuffer, &dwBufferSize);
		}
		if (res != ERROR_SUCCESS) break;

		/* find object in data */
		fFound = FALSE;
		/* first object */
		pPerfObj = (PERF_OBJECT_TYPE*)((uint8_t*)pPerfData + pPerfData->HeaderLength);
		for (dwCount = 0; dwCount < pPerfData->NumObjectTypes; ++dwCount) {
			if (pPerfObj->ObjectNameTitleIndex == dwObjectId) {
				/* find counter in object data */
				/* first counter */
				pPerfCounter = (PERF_COUNTER_DEFINITION*)((uint8_t*)pPerfObj + pPerfObj->HeaderLength);
				for (dwCount = 0; dwCount < (pPerfObj->NumCounters); ++dwCount) {
					if (pPerfCounter->CounterNameTitleIndex == dwCounterId) {
						/* find instance in counter data */
						if (pPerfObj->NumInstances == PERF_NO_INSTANCES) {
							pPerfCounterBlock = (PERF_COUNTER_BLOCK*)((uint8_t*)pPerfObj + pPerfObj->DefinitionLength);
							liCurrentCounterValue = *(LARGE_INTEGER*)((uint8_t*)pPerfCounterBlock + pPerfCounter->CounterOffset);
							fFound = TRUE;
						}
						else {
							/* first instance */
							pPerfInstance = (PERF_INSTANCE_DEFINITION*)((uint8_t*)pPerfObj + pPerfObj->DefinitionLength);
							for (lCount = 0; lCount < (pPerfObj->NumInstances); ++lCount) {
								pPerfCounterBlock = (PERF_COUNTER_BLOCK*)((uint8_t*)pPerfInstance + pPerfInstance->ByteLength);
								if (!mir_wstrcmpi(pwszInstanceName, (wchar_t*)((uint8_t*)pPerfInstance + pPerfInstance->NameOffset))) {
									liCurrentCounterValue = *(LARGE_INTEGER*)((uint8_t*)pPerfCounterBlock + pPerfCounter->CounterOffset);
									fFound = TRUE;
									break;
								}
								/* next instance */
								pPerfInstance = (PPERF_INSTANCE_DEFINITION)((uint8_t*)pPerfCounterBlock + pPerfCounterBlock->ByteLength);
							}
						}
						break;
					}
					/* next counter */
					pPerfCounter = (PERF_COUNTER_DEFINITION*)((uint8_t*)pPerfCounter + pPerfCounter->ByteLength);
				}
				break;
			}
			/* next object */
			pPerfObj = (PERF_OBJECT_TYPE*)((uint8_t*)pPerfObj + pPerfObj->TotalByteLength);
		}
		if (!fFound)
			break;

		/* calc val from data, we need two samplings
		 * counter type: PERF_100NSEC_TIMER_INV
		 * calc: time base=100Ns, value=100*(1-(data_diff)/(100NsTime_diff)) */
		if (!fIsFirst) {
			nCpuUsage = (uint8_t)((1.0 - (Li2Double(liCurrentCounterValue) - Li2Double(liPrevCounterValue)) / (Li2Double(pPerfData->PerfTime100nSec) - Li2Double(liPrevPerfTime100nSec)))*100.0 + 0.5);
			if (!CallBackAndWait(param, nCpuUsage))
				break;
		}
		else fIsFirst = FALSE;
		/* store current sampling for next */
		memcpy(&liPrevCounterValue, &liCurrentCounterValue, sizeof(LARGE_INTEGER));
		memcpy(&liPrevPerfTime100nSec, &pPerfData->PerfTime100nSec, sizeof(LARGE_INTEGER));
	}

	/* uninit */
	if (pPerfData)
		mir_free(pPerfData);
	if (fSwitched)
		WinNT_PerfStatsSwitch(L"PerfOS", TRUE);

	/* return error for PollCpuUsage() if never succeeded */
	if (param->hFirstEvent != nullptr)
		SetEvent(param->hFirstEvent);
	mir_free(param);
}

/************************* Start Thread *******************************/

// returns poll thread id on success
uint32_t PollCpuUsage(CPUUSAGEAVAILPROC pfnDataAvailProc, LPARAM lParam, uint32_t dwDelayMillis)
{
	uint32_t idThread = 0;

	/* init params */
	CpuUsageThreadParams *param = (struct CpuUsageThreadParams*)mir_alloc(sizeof(struct CpuUsageThreadParams));
	if (param == nullptr)
		return FALSE;

	HANDLE hFirstEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (hFirstEvent == nullptr) {
		mir_free(param);
		return FALSE;
	}

	param->dwDelayMillis = dwDelayMillis;
	param->pfnDataAvailProc = pfnDataAvailProc;
	param->lParam = lParam;
	param->pidThread = &idThread;
	param->hFirstEvent = hFirstEvent;

	/* start thread */
	if (mir_forkThread<CpuUsageThreadParams>(WinNT_PollThread, param) != INVALID_HANDLE_VALUE)
		WaitForSingleObject(hFirstEvent, INFINITE); /* wait for first success */
	else
		mir_free(param); /* thread not started */
	CloseHandle(hFirstEvent);
	return idThread;
}
