/*

'File Association Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"

/* Conversation */
extern HINSTANCE hInst;
static HWND hwndDdeMsg;
/* Misc */
static HANDLE hHookModulesLoaded,hHookPreShutdown;

/************************* Open Handler ***************************/

// pszFilePath needs to be allocated using mir_alloc()
static void __stdcall FileActionAsync(void *param)
{
	TCHAR *pszFilePath = (TCHAR*)param;
	/* invoke main handler */
	switch(InvokeFileHandler(pszFilePath)) { /* pszFilePath is always a long path name */
		case 0: /* success */ break;
		case CALLSERVICE_NOTFOUND:
			ShowInfoMessage(NIIF_ERROR,Translate("Miranda NG could not open file"),Translate("Miranda NG was not able to open \"%S\".\n\nThere is no registered handler for this file type."),pszFilePath);
			break;
		default:
			ShowInfoMessage(NIIF_ERROR,Translate("Miranda NG could not open file"),Translate("Miranda NG was not able to open \"%S\".\n\nThe file could not be processed."),pszFilePath);
	}
	mir_free(pszFilePath); /* async param */
}

// pszUrl needs to be allocated using mir_alloc()
static void __stdcall UrlActionAsync(void *param)
{
	TCHAR *pszUrl = (TCHAR*)param;
	/* invoke main handler */
	switch(InvokeUrlHandler(pszUrl)) {
		case 0: /* success */ break;
		case CALLSERVICE_NOTFOUND:
			ShowInfoMessage(NIIF_ERROR,Translate("Miranda NG could not open URL"),Translate("Miranda NG was not able to open \"%S\".\n\nThere is no registered handler for this URL type."),pszUrl);
			break;
		default:
			ShowInfoMessage(NIIF_ERROR,Translate("Miranda NG could not open URL"),Translate("Miranda NG was not able to open \"%S\".\n\nThe given URL is invalid and cannot be parsed."),pszUrl);
	}
	mir_free(pszUrl); /* async param */
}

/************************* Conversation ***************************/

#define DDEMESSAGETIMEOUT      30000
#define WNDCLASS_DDEMSGWINDOW  _T("MirandaDdeMsgWindow")

// returned pointer points into a substring of ppszString
// returns an empty string if the string does not have enough arguments
static TCHAR* GetExecuteParam(TCHAR **ppszString)
{
	TCHAR *pszParam,*p;
	BOOL fQuoted;
	
	fQuoted=(**ppszString==_T('"'));
	pszParam=*ppszString;
	if(fQuoted) pszParam++;
	p=_tcschr(pszParam,(TCHAR)(fQuoted?_T('"'):_T(',')));
	if(p!=NULL) {
		*(p++)=0;
		if(fQuoted && *p==_T(',')) p++;
	} else p=&pszParam[mir_tstrlen(pszParam)];
	*ppszString=p;
	return pszParam;
}

static LRESULT CALLBACK DdeMessageWindow(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
		case WM_DDE_INITIATE:
		{
			ATOM hSzApp=LOWORD(lParam); /* no UnpackDDElParam() here */
			ATOM hSzTopic=HIWORD(lParam);
			if ((hSzApp==GlobalFindAtom(DDEAPP) && hSzTopic==GlobalFindAtom(DDETOPIC)) || !hSzApp) {
				hSzApp=GlobalAddAtom(DDEAPP);
				hSzTopic=GlobalAddAtom(DDETOPIC);
				if(hSzApp && hSzTopic) 
					/* PackDDElParam() only for posted msgs */
					SendMessage((HWND)wParam,WM_DDE_ACK,(WPARAM)hwnd,MAKELPARAM(hSzApp,hSzTopic));
				if(hSzApp) GlobalDeleteAtom(hSzApp); 
				if(hSzTopic) GlobalDeleteAtom(hSzTopic);
			}
			return 0;
		}
		case WM_DDE_EXECUTE: /* posted message */
		{
			HGLOBAL hCommand;
			TCHAR *pszCommand;
			DDEACK ack;
			ZeroMemory(&ack,sizeof(ack));
			if(UnpackDDElParam(msg,lParam,NULL,(PUINT_PTR)&hCommand)) {
				/* ANSI execute command can't happen for shell */
				if(IsWindowUnicode((HWND)wParam)) {
					pszCommand = (TCHAR*)GlobalLock(hCommand);
					if(pszCommand!=NULL) {
						TCHAR *pszAction,*pszArg;
						pszAction=GetExecuteParam(&pszCommand);
						pszArg=mir_tstrdup(GetExecuteParam(&pszCommand));
						if(pszArg!=NULL) {
							/* we are inside miranda here, we make it async so the shell does
							 * not timeout regardless what the plugins try to do. */
							if (!mir_tstrcmpi(pszAction,_T("file")))
								CallFunctionAsync(FileActionAsync,pszArg);
							else if (!mir_tstrcmpi(pszAction,_T("url")))
								CallFunctionAsync(UrlActionAsync,pszArg);
						}
						GlobalUnlock(hCommand);
					}
				
				}
				
				lParam=ReuseDDElParam(lParam,msg,WM_DDE_ACK,*(PUINT)&ack,(UINT)hCommand);
				if (!PostMessage((HWND)wParam,WM_DDE_ACK,(WPARAM)hwnd,lParam)) {
					GlobalFree(hCommand);
					FreeDDElParam(WM_DDE_ACK,lParam);
				}
			}
			return 0;
		}
		case WM_DDE_TERMINATE:
			PostMessage((HWND)wParam,msg,(WPARAM)hwnd,0); /* ack reply */
			return 0;
		case WM_DDE_REQUEST:
		case WM_DDE_ADVISE:
		case WM_DDE_UNADVISE:
		case WM_DDE_POKE:
		/* fail safely for those to avoid memory leak in lParam */
		{
			ATOM hSzItem;
			DDEACK ack;
			ZeroMemory(&ack,sizeof(ack));
			if(UnpackDDElParam(msg,lParam,NULL,(PUINT_PTR)&hSzItem)) {
				lParam=ReuseDDElParam(lParam,msg,WM_DDE_ACK,*(PUINT)&ack,(UINT)hSzItem);
				if (!PostMessage((HWND)wParam,WM_DDE_ACK,(WPARAM)hwnd,lParam)) {
					if(hSzItem) GlobalDeleteAtom(hSzItem);
					FreeDDElParam(WM_DDE_ACK,lParam);
				}
			}
			return 0;
		}
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

// CloseHandle() the return value
static HANDLE StartupMainProcess(TCHAR *pszDatabasePath)
{
	TCHAR *p,szPath[MAX_PATH];

	/* we are inside RunDll32 here */
	if (!GetModuleFileName(hInst,szPath,SIZEOF(szPath))) return NULL;
	p=_tcsrchr(szPath,_T('\\'));
	if(p!=NULL) {	*p=0; p=_tcsrchr(szPath,_T('\\')); }
	if(p==NULL) return NULL;
	mir_tstrcpy(++p,_T("miranda32.exe"));

	/* inherit startup data from RunDll32 process */
	STARTUPINFO si;
	GetStartupInfo(&si);
	PROCESS_INFORMATION pi;
	if (!CreateProcess(szPath,pszDatabasePath,NULL,NULL,TRUE,GetPriorityClass(GetCurrentProcess()),NULL,NULL,&si,&pi))
		return NULL;
	CloseHandle(pi.hThread);
	return pi.hProcess;
}

#ifdef __cplusplus
extern "C" {
#endif 

// entry point for RunDll32, this is also WaitForDDEW
__declspec(dllexport) void CALLBACK WaitForDDE(HWND,HINSTANCE,TCHAR *pszCmdLine,int)
{
	HANDLE pHandles[2];
	DWORD dwTick;
	
	/* wait for dde window to be available (avoiding race condition) */
	pHandles[0]=CreateEvent(NULL,TRUE,FALSE,WNDCLASS_DDEMSGWINDOW);
	if(pHandles[0]!=NULL) {
		pHandles[1]=StartupMainProcess(pszCmdLine); /* obeys nCmdShow using GetStartupInfo() */
		if(pHandles[1]!=NULL) {
			dwTick=GetTickCount();
			/* either process terminated or dde window created */
			if(WaitForMultipleObjects(SIZEOF(pHandles),pHandles,FALSE,DDEMESSAGETIMEOUT)==WAIT_OBJECT_0) {
				dwTick=GetTickCount()-dwTick;
				if(dwTick<DDEMESSAGETIMEOUT)
					WaitForInputIdle(pHandles[1],DDEMESSAGETIMEOUT-dwTick);
			}
			CloseHandle(pHandles[1]);
		}
		CloseHandle(pHandles[0]);
	}
	/* shell called WaitForInputIdle() on us to detect when dde is ready,
	 * we are ready now: exit helper process */
}

#ifdef __cplusplus
}
#endif

/************************* Misc ***********************************/

static int DdePreShutdown(WPARAM,LPARAM)
{
	/* dde needs to be stopped before any plugins are unloaded */
	if(hwndDdeMsg!=NULL) DestroyWindow(hwndDdeMsg);
	UnregisterClass(WNDCLASS_DDEMSGWINDOW,hInst);
	return 0;
}

static int DdeModulesLoaded2(WPARAM,LPARAM)
{
	/* create message window */
	WNDCLASS wcl;
	wcl.lpfnWndProc=DdeMessageWindow;
	wcl.cbClsExtra=0;
	wcl.cbWndExtra=0;
	wcl.hInstance=hInst;
	wcl.hCursor=NULL;
	wcl.lpszClassName=WNDCLASS_DDEMSGWINDOW;
	wcl.hbrBackground=NULL;
	wcl.hIcon=NULL;
	wcl.lpszMenuName=NULL;
	wcl.style=0;
	RegisterClass(&wcl);
	/* Note: use of HWND_MESSAGE does not fit for DDE as the window must be a top-level one */
	hwndDdeMsg=CreateWindow(WNDCLASS_DDEMSGWINDOW,NULL,0,0,0,0,0,NULL,NULL,hInst,NULL);

	/* make known dde startup code is passed */
	HANDLE hEvent=OpenEvent(EVENT_MODIFY_STATE,FALSE,WNDCLASS_DDEMSGWINDOW);
	if(hEvent!=NULL) {
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}

	CleanupRegTreeBackupSettings();
	CleanupMimeTypeAddedSettings();
	CleanupAssocEnabledSettings();
	return 0;
}

static int DdeModulesLoaded(WPARAM,LPARAM)
{
	/* dde needs to be loaded after all the other plugins got loaded,
	 * hook again to get the latest position in chain */
	UnhookEvent(hHookModulesLoaded);
	hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,DdeModulesLoaded2);
	return 0;
}

void InitDde(void)
{
	hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,DdeModulesLoaded);
	hHookPreShutdown=HookEvent(ME_SYSTEM_PRESHUTDOWN,DdePreShutdown);
}

void UninitDde(void)
{
	UnhookEvent(hHookModulesLoaded);
	UnhookEvent(hHookPreShutdown);
}

