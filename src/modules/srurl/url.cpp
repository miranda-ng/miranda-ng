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
#include <m_url.h>
#include "url.h"

HANDLE hUrlWindowList = NULL;
static HANDLE hEventContactSettingChange = NULL;
static HANDLE hContactDeleted = NULL;
static HANDLE hSRUrlMenuItem = NULL;

INT_PTR CALLBACK DlgProcUrlSend(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcUrlRecv(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static INT_PTR ReadUrlCommand(WPARAM, LPARAM lParam)
{
	CreateDialogParam(hMirandaInst,MAKEINTRESOURCE(IDD_URLRECV),NULL,DlgProcUrlRecv,lParam);
	return 0;
}

static int UrlEventAdded(WPARAM wParam,LPARAM lParam)
{
	CLISTEVENT cle;
	DBEVENTINFO dbei;
	TCHAR szTooltip[256];

	ZeroMemory(&dbei,sizeof(dbei));
	dbei.cbSize=sizeof(dbei);
	dbei.cbBlob=0;
	CallService(MS_DB_EVENT_GET,lParam,(LPARAM)&dbei);
	if(dbei.flags&(DBEF_SENT|DBEF_READ) || dbei.eventType!=EVENTTYPE_URL) return 0;

	SkinPlaySound("RecvUrl");
	ZeroMemory(&cle,sizeof(cle));
	cle.cbSize=sizeof(cle);
	cle.flags = CLEF_TCHAR;
	cle.hContact=(HANDLE)wParam;
	cle.hDbEvent=(HANDLE)lParam;
	cle.hIcon = LoadSkinIcon( SKINICON_EVENT_URL );
	cle.pszService="SRUrl/ReadUrl";
	mir_sntprintf(szTooltip,SIZEOF(szTooltip),TranslateT("URL from %s"), cli.pfnGetContactDisplayName(( HANDLE )wParam, 0 ));
	cle.ptszTooltip=szTooltip;
	CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
	return 0;
}

static INT_PTR SendUrlCommand(WPARAM wParam, LPARAM)
{
	CreateDialogParam(hMirandaInst,MAKEINTRESOURCE(IDD_URLSEND),NULL,DlgProcUrlSend,wParam);
	return 0;
}

static void RestoreUnreadUrlAlerts(void)
{
	CLISTEVENT cle={0};
	DBEVENTINFO dbei={0};
	TCHAR toolTip[256];
	HANDLE hDbEvent,hContact;

	dbei.cbSize=sizeof(dbei);
	cle.cbSize=sizeof(cle);
	cle.hIcon = LoadSkinIcon( SKINICON_EVENT_URL );
	cle.pszService="SRUrl/ReadUrl";

	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hContact) {
		hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDFIRSTUNREAD,(WPARAM)hContact,0);
		while(hDbEvent) {
			dbei.cbBlob=0;
			CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei);
			if(!(dbei.flags&(DBEF_SENT|DBEF_READ)) && dbei.eventType==EVENTTYPE_URL) {
				cle.hContact=hContact;
				cle.hDbEvent=hDbEvent;
				cle.flags = CLEF_TCHAR;
				mir_sntprintf(toolTip,SIZEOF(toolTip),TranslateT("URL from %s"), cli.pfnGetContactDisplayName( hContact, 0 ));
				cle.ptszTooltip=toolTip;
				CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
			}
			hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0);
		}
		hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
	}
}

static int ContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	char *szProto;

	szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);
	if(lstrcmpA(cws->szModule,"CList") && (szProto==NULL || lstrcmpA(cws->szModule,szProto))) return 0;
	WindowList_Broadcast(hUrlWindowList,DM_UPDATETITLE,0,0);
	return 0;
}

static int SRUrlPreBuildMenu(WPARAM wParam, LPARAM)
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIF_HIDDEN;

	char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if ( szProto != NULL )
		if ( CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_URLSEND )
			mi.flags = CMIM_FLAGS;

	CallService( MS_CLIST_MODIFYMENUITEM, (WPARAM)hSRUrlMenuItem, (LPARAM)&mi );
	return 0;
}

static int SRUrlModulesLoaded(WPARAM, LPARAM)
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.position = -2000040000;
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.icolibItem = GetSkinIconHandle( SKINICON_EVENT_URL );
	mi.pszName = LPGEN("Web Page Address (&URL)");
	mi.pszService = MS_URL_SENDURL;
	hSRUrlMenuItem = (HANDLE)CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi );

	RestoreUnreadUrlAlerts();
	return 0;
}

static int SRUrlShutdown(WPARAM, LPARAM)
{
	if ( hEventContactSettingChange )
		UnhookEvent( hEventContactSettingChange );

	if ( hContactDeleted )
		UnhookEvent( hContactDeleted );

	if ( hUrlWindowList )
		WindowList_BroadcastAsync( hUrlWindowList, WM_CLOSE, 0, 0 );

	return 0;
}

int UrlContactDeleted(WPARAM wParam, LPARAM)
{
	HWND h = WindowList_Find(hUrlWindowList,(HANDLE)wParam);
	if ( h )
		SendMessage(h,WM_CLOSE,0,0);

	return 0;
}

int LoadSendRecvUrlModule(void)
{
	hUrlWindowList=(HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
	HookEvent(ME_SYSTEM_MODULESLOADED,SRUrlModulesLoaded);
	HookEvent(ME_DB_EVENT_ADDED,UrlEventAdded);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU,SRUrlPreBuildMenu);
	hEventContactSettingChange = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	hContactDeleted = HookEvent(ME_DB_CONTACT_DELETED, UrlContactDeleted);
	HookEvent(ME_SYSTEM_PRESHUTDOWN,SRUrlShutdown);
	CreateServiceFunction(MS_URL_SENDURL,SendUrlCommand);
	CreateServiceFunction("SRUrl/ReadUrl",ReadUrlCommand);
	SkinAddNewSoundEx("RecvUrl","URL","Incoming");
	return 0;
}
