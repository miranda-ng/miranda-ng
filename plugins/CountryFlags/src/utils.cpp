/*
Miranda IM Country Flags Plugin
Copyright (C) 2006-1007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Flags-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "flags.h"

/************************* Buffered Functions *********************/

struct BufferedCallData {
	DWORD startTick;
	UINT uElapse;
	BUFFEREDPROC pfnBuffProc;
	LPARAM lParam;
	#ifdef _DEBUG
	const char *pszProcName;
	#endif
};

static UINT idBufferedTimer;
static struct BufferedCallData *callList;
static int nCallListCount;

// always gets called in main message loop
static void CALLBACK BufferedProcTimer(HWND hwnd,UINT msg,UINT_PTR idTimer,DWORD currentTick)
{
	struct BufferedCallData *buf;
	UINT uElapsed,uElapseNext=USER_TIMER_MAXIMUM;
	BUFFEREDPROC pfnBuffProc;
	LPARAM lParam;
	#ifdef _DEBUG
	char szDbgLine[256];
	const char *pszProcName;
	#endif

	for(int i=0; i < nCallListCount; ++i) {
		/* find elapsed procs */
		uElapsed=currentTick-callList[i].startTick; /* wraparound works */
		if ((uElapsed+USER_TIMER_MINIMUM)>=callList[i].uElapse) { 
			/* call elapsed proc */
			pfnBuffProc=callList[i].pfnBuffProc;
			lParam=callList[i].lParam;
			#ifdef _DEBUG
				pszProcName=callList[i].pszProcName;
			#endif
			/* resize storage array */
			if ((i+1)<nCallListCount)
				MoveMemory(&callList[i],&callList[i+1],((nCallListCount-i-1)*sizeof(struct BufferedCallData)));
			--nCallListCount;
			--i; /* reiterate current */
			if (nCallListCount) {
				buf=(struct BufferedCallData*)mir_realloc(callList,nCallListCount*sizeof(struct BufferedCallData));
				if (buf != NULL) callList=buf;
			} else {
				mir_free(callList);
				callList=NULL;
			}
			#ifdef _DEBUG
				mir_snprintf(szDbgLine,SIZEOF(szDbgLine),"buffered call: %s(0x%X)\n",pszProcName,lParam); /* all ascii */
				OutputDebugStringA(szDbgLine);
			#endif
			CallFunctionAsync((void (CALLBACK *)(void*))pfnBuffProc,(void*)lParam); /* compatible */
		}
		/* find next timer delay */
		else if ((callList[i].uElapse-uElapsed)<uElapseNext)
			uElapseNext=callList[i].uElapse-uElapsed;
	}

	/* set next timer */
	if (nCallListCount) {
		#ifdef _DEBUG
			mir_snprintf(szDbgLine,SIZEOF(szDbgLine),"next buffered timeout: %ums\n",uElapseNext); /* all ascii */
			OutputDebugStringA(szDbgLine);
		#endif
		idBufferedTimer=SetTimer(hwnd,idBufferedTimer,uElapseNext,BufferedProcTimer); /* will be reset */
	} else {
		KillTimer(hwnd,idTimer);
		idBufferedTimer=0;
		#ifdef _DEBUG
			OutputDebugStringA("empty buffered queue\n");
		#endif
	}
}

// assumes to be called in context of main thread
#ifdef _DEBUG
void _CallFunctionBuffered(BUFFEREDPROC pfnBuffProc,const char *pszProcName,LPARAM lParam,BOOL fAccumulateSameParam,UINT uElapse)
#else
void _CallFunctionBuffered(BUFFEREDPROC pfnBuffProc,LPARAM lParam,BOOL fAccumulateSameParam,UINT uElapse)
#endif
{
	struct BufferedCallData *data=NULL;
	int i;

	/* find existing */
	for(i=0;i<nCallListCount;++i)
		if (callList[i].pfnBuffProc == pfnBuffProc)
			if (!fAccumulateSameParam || callList[i].lParam == lParam) {
				data=&callList[i];
				break;
			}
	/* append new */
	if (data == NULL) {
		/* resize storage array */
		data=(struct BufferedCallData*)mir_realloc(callList,(nCallListCount+1)*sizeof(struct BufferedCallData));
		if (data == NULL) return;
		callList=data;
		data=&callList[nCallListCount];
		++nCallListCount;
	}
	/* set delay */
	data->startTick=GetTickCount();
	data->uElapse=uElapse;
	data->lParam=lParam;
	data->pfnBuffProc=pfnBuffProc;
	#ifdef _DEBUG
		{	char szDbgLine[256];
			data->pszProcName=pszProcName;
			mir_snprintf(szDbgLine,SIZEOF(szDbgLine),"buffered queue: %s(0x%X)\n",pszProcName,lParam); /* all ascii */
			OutputDebugStringA(szDbgLine);
			if (!idBufferedTimer) {
				mir_snprintf(szDbgLine,SIZEOF(szDbgLine),"next buffered timeout: %ums\n",uElapse); /* all ascii */
				OutputDebugStringA(szDbgLine);
			}
		}
	#endif
	/* set next timer */
	if (idBufferedTimer) uElapse=USER_TIMER_MINIMUM; /* will get recalculated */
	idBufferedTimer=SetTimer(NULL,idBufferedTimer,uElapse,BufferedProcTimer);
}

// assumes to be called in context of main thread
void PrepareBufferedFunctions(void)
{
	idBufferedTimer=0;
	nCallListCount=0;
	callList=NULL;
}

// assumes to be called in context of main thread
void KillBufferedFunctions(void)
{
	if (idBufferedTimer) KillTimer(NULL,idBufferedTimer);
	nCallListCount=0;
	mir_free(callList); /* does NULL check */
}
