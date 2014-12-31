/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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
int ContactSettingChanged(WPARAM wParam, LPARAM lParam);
int CListOptInit(WPARAM wParam, LPARAM lParam);
INT_PTR ContactChangeGroup(WPARAM wParam, LPARAM lParam);
INT_PTR HotkeysProcessMessage(WPARAM wParam, LPARAM lParam) { return pcli->pfnHotkeysProcessMessage(wParam, lParam); }
void InitTrayMenus(void);

HIMAGELIST hCListImages;

HANDLE hContactIconChangedEvent;
extern BYTE nameOrder[];

static HANDLE hSettingChanged, hProtoAckHook;

int cli_IconFromStatusMode(const char *szProto,int nStatus, MCONTACT hContact)
{
	int result = -1;
	if (hContact && szProto) {
		char * szActProto = (char*)szProto;
		int  nActStatus = nStatus;
		MCONTACT hActContact = hContact;
		if (!db_get_b(NULL, "CLC", "Meta", 0) && !strcmp(szActProto, META_PROTO)) {
			// substitute params by mostonline contact datas
			MCONTACT hMostOnlineContact = db_mc_getMostOnline(hActContact);
			if (hMostOnlineContact && hMostOnlineContact != (MCONTACT)CALLSERVICE_NOTFOUND) {
				ClcCacheEntry *cacheEntry = (ClcCacheEntry *)pcli->pfnGetCacheEntry(hMostOnlineContact);
				if (cacheEntry && cacheEntry->szProto) {
					szActProto = cacheEntry->szProto;
					nActStatus = cacheEntry->status;
					hActContact = hMostOnlineContact;
				}
			}
		}

		if (ProtoServiceExists(szActProto, PS_GETADVANCEDSTATUSICON))
			result = ProtoCallService(szActProto, PS_GETADVANCEDSTATUSICON, (WPARAM)hActContact, 0);

		// result == -1 means no Advanced icon. LOWORD(result) == 0 happens when Advanced icon returned by ICQ (i.e. no transpot)
		if (result == -1 || !(LOWORD(result)))
			result = saveIconFromStatusMode(szActProto, nActStatus, NULL);
	}
	else result = saveIconFromStatusMode(szProto, nStatus, NULL);
	return result;
}

int ExtIconFromStatusMode(MCONTACT hContact, const char *szProto,int status)
{
	return pcli->pfnIconFromStatusMode(szProto,status,hContact);
}

/////////// End by FYR ////////

static int ProtocolAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type == ACKTYPE_AWAYMSG && ack->lParam) {
		DBVARIANT dbv;
		if ( !db_get_ts(ack->hContact, "CList", "StatusMsg", &dbv)) {
			if ( !_tcscmp(dbv.ptszVal, (TCHAR *)ack->lParam)) {
				db_free(&dbv);
				return 0;
			}
			db_free(&dbv);
		}
		if ( db_get_b(NULL,"CList","ShowStatusMsg",0) || db_get_b(ack->hContact,"CList","StatusMsgAuto",0))
         db_set_ts(ack->hContact, "CList", "StatusMsg", (TCHAR *)ack->lParam);
	}

	return 0;
}

static INT_PTR GetStatusMode(WPARAM wParam, LPARAM lParam)
{
	return pcli->currentDesiredStatusMode;
}

static int ContactListShutdownProc(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(hProtoAckHook);
	UninitCustomMenus();
	return 0;
}

int LoadContactListModule(void)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		db_set_s(hContact, "CList", "StatusMsg", "");

	hCListImages = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	DefaultImageListColorDepth = db_get_dw(NULL,"CList","DefaultImageListColorDepth",ILC_COLOR32);

	hProtoAckHook = (HANDLE) HookEvent(ME_PROTO_ACK, ProtocolAck);
	HookEvent(ME_OPT_INITIALISE,CListOptInit);
	HookEvent(ME_SYSTEM_SHUTDOWN,ContactListShutdownProc);
	hSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED,ContactSettingChanged);
	hContactIconChangedEvent = CreateHookableEvent(ME_CLIST_CONTACTICONCHANGED);
	CreateServiceFunction(MS_CLIST_CONTACTCHANGEGROUP,ContactChangeGroup);
	CreateServiceFunction(MS_CLIST_HOTKEYSPROCESSMESSAGE,HotkeysProcessMessage);
	CreateServiceFunction(MS_CLIST_GETSTATUSMODE, GetStatusMode);

	InitCustomMenus();
	InitTrayMenus();
	return 0;
}
