/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "commonheaders.h"
#include <ctype.h>

#define DDEMESSAGETIMEOUT  1000
#define WNDCLASS_DDEMSGWINDOW  _T("MirandaDdeMsgWindow")

struct DdeMsgWindowData {
	int fAcked,fData;
	HWND hwndDde;
};

static LRESULT CALLBACK DdeMessageWindow(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	struct DdeMsgWindowData *dat;
	ATOM hSzItem;
	HGLOBAL hDdeData;

	dat=(struct DdeMsgWindowData*)GetWindowLongPtr(hwnd,0);
	switch(msg) {
		case WM_DDE_ACK:
			dat->fAcked=1;
			dat->hwndDde=(HWND)wParam;
			return 0;
		case WM_DDE_DATA:
			UnpackDDElParam(msg,lParam,(PUINT_PTR)&hDdeData,(PUINT_PTR)&hSzItem);
			dat->fData=1;
			if(hDdeData) {
				DDEDATA *data;
				int release;
				data=(DDEDATA*)GlobalLock(hDdeData);
				if(data->fAckReq) {
					DDEACK ack={0};
					PostMessage((HWND)wParam,WM_DDE_ACK,(WPARAM)hwnd,PackDDElParam(WM_DDE_ACK,*(PUINT)&ack,(UINT)hSzItem));
				}
				else GlobalDeleteAtom(hSzItem);
				release=data->fRelease;
				GlobalUnlock(hDdeData);
				if(release) GlobalFree(hDdeData);
			}
			else GlobalDeleteAtom(hSzItem);
			return 0;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static int DoDdeRequest(const char *szItemName,HWND hwndDdeMsg)
{
	ATOM hSzItemName;
	DWORD timeoutTick,thisTick;
	MSG msg;
	struct DdeMsgWindowData *dat=(struct DdeMsgWindowData*)GetWindowLongPtr(hwndDdeMsg,0);

	hSzItemName=GlobalAddAtomA(szItemName);
	if(!PostMessage(dat->hwndDde,WM_DDE_REQUEST,(WPARAM)hwndDdeMsg,MAKELPARAM(CF_TEXT,hSzItemName))) {
		GlobalDeleteAtom(hSzItemName);
		return 1;
	}
	timeoutTick=GetTickCount()+5000;
	dat->fData=0; dat->fAcked=0;
	do {
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if(dat->fData || dat->fAcked) break;
		thisTick=GetTickCount();
		if(thisTick>timeoutTick) break;
	} while(MsgWaitForMultipleObjects(0,NULL,FALSE,timeoutTick-thisTick,QS_ALLINPUT)==WAIT_OBJECT_0);

	if(!dat->fData) {
		GlobalDeleteAtom(hSzItemName);
		return 1;
	}
	return 0;
}

//see Q160957 and http://developer.netscape.com/docs/manuals/communicator/DDE/index.htm
static int DdeOpenUrl(const char *szBrowser,char *szUrl,int newWindow,HWND hwndDdeMsg)
{
	ATOM hSzBrowser,hSzTopic;
	DWORD_PTR dwResult;
	char *szItemName;
	struct DdeMsgWindowData *dat=(struct DdeMsgWindowData*)GetWindowLongPtr(hwndDdeMsg,0);

	hSzBrowser=GlobalAddAtomA(szBrowser);
	hSzTopic=GlobalAddAtomA("WWW_OpenURL");
	dat->fAcked=0;
	if(!SendMessageTimeout(HWND_BROADCAST,WM_DDE_INITIATE,(WPARAM)hwndDdeMsg,MAKELPARAM(hSzBrowser,hSzTopic),SMTO_ABORTIFHUNG|SMTO_NORMAL,DDEMESSAGETIMEOUT,&dwResult)
	   || !dat->fAcked) {
		GlobalDeleteAtom(hSzTopic);
		GlobalDeleteAtom(hSzBrowser);
		return 1;
	}
	szItemName=(char*)mir_alloc(lstrlenA(szUrl)+7);
	wsprintfA(szItemName,"\"%s\",,%d",szUrl,newWindow?0:-1);
	if(DoDdeRequest(szItemName,hwndDdeMsg)) {
		mir_free(szItemName);
		GlobalDeleteAtom(hSzTopic);
		GlobalDeleteAtom(hSzBrowser);
		return 1;
	}
	PostMessage(dat->hwndDde,WM_DDE_TERMINATE,(WPARAM)hwndDdeMsg,0);
	GlobalDeleteAtom(hSzTopic);
	GlobalDeleteAtom(hSzBrowser);
	mir_free(szItemName);
	return 0;
}

typedef struct {
    char *szUrl;
    int newWindow;
} TOpenUrlInfo;

static void OpenURLThread(void *arg)
{
   TOpenUrlInfo *hUrlInfo = (TOpenUrlInfo*)arg;
	char *szResult;
	HWND hwndDdeMsg;
	struct DdeMsgWindowData msgWndData={0};
	char *pszProtocol;
	HKEY hKey;
	char szSubkey[80];
	char szCommandName[MAX_PATH];
	DWORD dataLength;
	int success=0;
    
   if (!hUrlInfo->szUrl) return;
	hwndDdeMsg=CreateWindow(WNDCLASS_DDEMSGWINDOW,_T(""),0,0,0,0,0,NULL,NULL,hMirandaInst,NULL);
	SetWindowLongPtr(hwndDdeMsg,0,(LONG_PTR)&msgWndData);

	if(!_strnicmp(hUrlInfo->szUrl,"ftp:",4) || !_strnicmp(hUrlInfo->szUrl,"ftp.",4)) pszProtocol="ftp";
	if(!_strnicmp(hUrlInfo->szUrl,"mailto:",7)) pszProtocol="mailto";
	if(!_strnicmp(hUrlInfo->szUrl,"news:",5)) pszProtocol="news";
	else pszProtocol="http";
	wsprintfA(szSubkey,"%s\\shell\\open\\command",pszProtocol);
	if(RegOpenKeyExA(HKEY_CURRENT_USER,szSubkey,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS
	   || RegOpenKeyExA(HKEY_CLASSES_ROOT,szSubkey,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS) {
		dataLength=SIZEOF(szCommandName);
		if(RegQueryValueEx(hKey,NULL,NULL,NULL,(PBYTE)szCommandName,&dataLength)==ERROR_SUCCESS) {
			_strlwr(szCommandName);
			if(strstr(szCommandName,"mozilla") || strstr(szCommandName,"netscape"))
				success=(DdeOpenUrl("mozilla",hUrlInfo->szUrl,hUrlInfo->newWindow,hwndDdeMsg)==0 || DdeOpenUrl("netscape",hUrlInfo->szUrl,hUrlInfo->newWindow,hwndDdeMsg)==0);
			else if(strstr(szCommandName,"iexplore") || strstr(szCommandName,"msimn"))
				success=0==DdeOpenUrl("iexplore",hUrlInfo->szUrl,hUrlInfo->newWindow,hwndDdeMsg);
			else if(strstr(szCommandName,"opera"))
				success=0==DdeOpenUrl("opera",hUrlInfo->szUrl,hUrlInfo->newWindow,hwndDdeMsg);
			//opera's the default anyway
		}
		RegCloseKey(hKey);
	}

	DestroyWindow(hwndDdeMsg);
	if(success) return;

	//wack a protocol on it
	if((isalpha(hUrlInfo->szUrl[0]) && hUrlInfo->szUrl[1]==':') || hUrlInfo->szUrl[0]=='\\') {
		szResult=(char*)mir_alloc(lstrlenA(hUrlInfo->szUrl)+9);
		wsprintfA(szResult,"file:///%s",hUrlInfo->szUrl);
	}
	else {
		int i;
		for(i=0;isalpha(hUrlInfo->szUrl[i]);i++);
		if(hUrlInfo->szUrl[i]==':') szResult=mir_strdup(hUrlInfo->szUrl);
		else {
			if(!_strnicmp(hUrlInfo->szUrl,"ftp.",4)) {
				szResult=(char*)mir_alloc(lstrlenA(hUrlInfo->szUrl)+7);
				wsprintfA(szResult,"ftp://%s",hUrlInfo->szUrl);
			}
			else {
				szResult=(char*)mir_alloc(lstrlenA(hUrlInfo->szUrl)+8);
				wsprintfA(szResult,"http://%s",hUrlInfo->szUrl);
			}
		}
	}
	ShellExecuteA(NULL, "open", szResult, NULL, NULL, SW_SHOWDEFAULT);
	mir_free(szResult);
	mir_free(hUrlInfo->szUrl);
	mir_free(hUrlInfo);
	return;
}

static INT_PTR OpenURL(WPARAM wParam,LPARAM lParam) {
	TOpenUrlInfo *hUrlInfo = (TOpenUrlInfo*)mir_alloc(sizeof(TOpenUrlInfo));
	hUrlInfo->szUrl = (char*)lParam?mir_strdup((char*)lParam):NULL;
	hUrlInfo->newWindow = (int)wParam;
	forkthread(OpenURLThread, 0, (void*)hUrlInfo);
	return 0;
}

int InitOpenUrl(void)
{
	WNDCLASS wcl;
	wcl.lpfnWndProc=DdeMessageWindow;
	wcl.cbClsExtra=0;
	wcl.cbWndExtra=sizeof(void*);
	wcl.hInstance=hMirandaInst;
	wcl.hCursor=NULL;
	wcl.lpszClassName=WNDCLASS_DDEMSGWINDOW;
	wcl.hbrBackground=NULL;
	wcl.hIcon=NULL;
	wcl.lpszMenuName=NULL;
	wcl.style=0;
	RegisterClass(&wcl);
	CreateServiceFunction(MS_UTILS_OPENURL,OpenURL);
	return 0;
}
