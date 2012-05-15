/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

extern int DefaultImageListColorDepth;

int InitCustomMenus(void);
void UninitCustomMenus(void);
int ContactSettingChanged(WPARAM wParam,LPARAM lParam);
int CListOptInit(WPARAM wParam,LPARAM lParam);
INT_PTR ContactChangeGroup(WPARAM wParam,LPARAM lParam);
INT_PTR HotkeysProcessMessage(WPARAM wParam,LPARAM lParam) { return pcli->pfnHotkeysProcessMessage(wParam, lParam); }
void InitTrayMenus(void);

HIMAGELIST hCListImages;

HANDLE hContactIconChangedEvent;
extern BYTE nameOrder[];

static HANDLE hSettingChanged, hProtoAckHook;

/////////// End by FYR ////////
int cli_IconFromStatusMode(const char *szProto,int nStatus, HANDLE hContact)
{
	int result=-1;
	if (hContact && szProto) {
		char * szActProto=(char*)szProto;
		char AdvancedService[255]={0};
		int  nActStatus=nStatus;
		HANDLE hActContact=hContact;
		if (!DBGetContactSettingByte(NULL,"CLC","Meta",0) && !strcmp(szActProto,"MetaContacts")) {
			// substitute params by mostonline contact datas
			HANDLE hMostOnlineContact=(HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT,(WPARAM)hActContact,0);
			if (hMostOnlineContact && hMostOnlineContact != (HANDLE)CALLSERVICE_NOTFOUND) {
				pdisplayNameCacheEntry cacheEntry;
				cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hMostOnlineContact);
				if (cacheEntry && cacheEntry->szProto) {
					szActProto=cacheEntry->szProto;
					nActStatus=cacheEntry->status;
					hActContact=hMostOnlineContact;
				}
			}
		}
		_snprintf(AdvancedService,sizeof(AdvancedService),"%s%s",szActProto,"/GetAdvancedStatusIcon");

		if (ServiceExists(AdvancedService))
			result = CallService(AdvancedService,(WPARAM)hActContact, (LPARAM)0);

		if (result==-1 || !(LOWORD(result))) 
			// result == -1 means no Advanced icon. LOWORD(result) == 0 happens when Advanced icon returned by ICQ (i.e. no transpot)
			result = saveIconFromStatusMode(szActProto,nActStatus,NULL);
	}
	else result=saveIconFromStatusMode(szProto,nStatus,NULL);
	return result;
}


////////// By FYR/////////////
int ExtIconFromStatusMode(HANDLE hContact, const char *szProto,int status)
{
/*	if ( DBGetContactSettingByte( NULL, "CLC", "Meta", 0 ) == 1 )
		return pcli->pfnIconFromStatusMode(szProto,status,hContact);

	if ( szProto != NULL ) {
		if (strcmp(szProto,"MetaContacts") == 0 ) {
			hContact=(HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT,(UINT)hContact,0);
			if ( hContact != 0 ) {
				szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(UINT)hContact,0);
				status=DBGetContactSettingWord(hContact,szProto,"Status",ID_STATUS_OFFLINE);
			}
		}
	}*/

	return pcli->pfnIconFromStatusMode(szProto,status,hContact);
}

/////////// End by FYR ////////

static int ProtocolAck(WPARAM wParam,LPARAM lParam)
{
	ACKDATA *ack=(ACKDATA*)lParam;
	if (ack->type==ACKTYPE_AWAYMSG && ack->lParam) {
		DBVARIANT dbv;
		if (!DBGetContactSettingString(ack->hContact, "CList", "StatusMsg", &dbv)) {
			if (!strcmp(dbv.pszVal, (char *)ack->lParam)) {
				DBFreeVariant(&dbv);
				return 0;
			}
			DBFreeVariant(&dbv);
		}
		if ( DBGetContactSettingByte(NULL,"CList","ShowStatusMsg",0) || DBGetContactSettingByte(ack->hContact,"CList","StatusMsgAuto",0))
         DBWriteContactSettingString(ack->hContact, "CList", "StatusMsg", (char *)ack->lParam);
	}

	return 0;
}

static INT_PTR GetStatusMode(WPARAM wParam, LPARAM lParam)
{
	return pcli->currentDesiredStatusMode;
}

static int ContactListShutdownProc(WPARAM wParam,LPARAM lParam)
{
	UnhookEvent(hProtoAckHook);
	UninitCustomMenus();
	return 0;
}

int LoadContactListModule(void)
{
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact!=NULL) {
		DBWriteContactSettingString(hContact, "CList", "StatusMsg", "");
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}

	hCListImages = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	DefaultImageListColorDepth=DBGetContactSettingDword(NULL,"CList","DefaultImageListColorDepth",ILC_COLOR32);

	hProtoAckHook = (HANDLE) HookEvent(ME_PROTO_ACK, ProtocolAck);
	HookEvent(ME_OPT_INITIALISE,CListOptInit);
	HookEvent(ME_SYSTEM_SHUTDOWN,ContactListShutdownProc);
	hSettingChanged=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,ContactSettingChanged);
	hContactIconChangedEvent=CreateHookableEvent(ME_CLIST_CONTACTICONCHANGED);
	CreateServiceFunction(MS_CLIST_CONTACTCHANGEGROUP,ContactChangeGroup);
	CreateServiceFunction(MS_CLIST_HOTKEYSPROCESSMESSAGE,HotkeysProcessMessage);
	CreateServiceFunction(MS_CLIST_GETSTATUSMODE, GetStatusMode);

	InitCustomMenus();
	InitTrayMenus();
	return 0;
}
