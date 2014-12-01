/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

static int uniqueEventId = 0;

static bool Proto_IsAccountEnabled(PROTOACCOUNT *pa)
{
	return pa && ((pa->bIsEnabled && !pa->bDynDisabled) || pa->bOldProto);
}

static int UserOnlineSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact == NULL || strcmp(cws->szSetting, "Status"))
		return 0;

	int newStatus = cws->value.wVal;
	int oldStatus = db_get_w(hContact, "UserOnline", "OldStatus", ID_STATUS_OFFLINE);
	db_set_w(hContact, "UserOnline", "OldStatus", (WORD)newStatus);
	if (CallService(MS_IGNORE_ISIGNORED, hContact, IGNOREEVENT_USERONLINE)) return 0;
	if (db_get_b(hContact, "CList", "Hidden", 0)) return 0;
    if (newStatus == ID_STATUS_OFFLINE && oldStatus != ID_STATUS_OFFLINE) {
       // Remove the event from the queue if it exists since they are now offline
		 int lastEvent = (int)db_get_dw(hContact, "UserOnline", "LastEvent", 0);

       if (lastEvent) {
           CallService(MS_CLIST_REMOVEEVENT, hContact, (LPARAM)lastEvent);
			  db_set_dw(hContact, "UserOnline", "LastEvent", 0);
       }
    }
	if ((newStatus == ID_STATUS_ONLINE || newStatus == ID_STATUS_FREECHAT) &&
	   oldStatus != ID_STATUS_ONLINE && oldStatus != ID_STATUS_FREECHAT) {
		{
			DWORD ticked = db_get_dw(NULL, "UserOnline", cws->szModule, GetTickCount());
			// only play the sound (or show event) if this event happens at least 10 secs after the proto went from offline
			if (GetTickCount() - ticked > (1000*10)) {
				CLISTEVENT cle;
				TCHAR tooltip[256];

				memset(&cle, 0, sizeof(cle));
				cle.cbSize = sizeof(cle);
				cle.flags = CLEF_ONLYAFEW | CLEF_TCHAR;
				cle.hContact = hContact;
				cle.hDbEvent = (HANDLE)(uniqueEventId++);
				cle.hIcon = LoadSkinIcon(SKINICON_OTHER_USERONLINE, false);
				cle.pszService = "UserOnline/Description";
				mir_sntprintf(tooltip, SIZEOF(tooltip), TranslateT("%s is online"), pcli->pfnGetContactDisplayName(hContact, 0));
				cle.ptszTooltip = tooltip;
				CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
				IcoLib_ReleaseIcon(cle.hIcon, 0);
                db_set_dw(cle.hContact, "UserOnline", "LastEvent", (DWORD)cle.hDbEvent);
				SkinPlaySound("UserOnline");
			}
		}
	}
	return 0;
}

static int UserOnlineAck(WPARAM, LPARAM lParam)
{
	ACKDATA * ack = (ACKDATA*) lParam;
	if (ack != 0 && ack->szModule && ack->type == ACKTYPE_STATUS && ack->result == ACKRESULT_SUCCESS && ack->hProcess == (HANDLE)ID_STATUS_OFFLINE) {
		// if going from offline to any other mode, remember when it happened.
		db_set_dw(NULL, "UserOnline", ack->szModule, GetTickCount());
	}
	return 0;
}

static int UserOnlineModulesLoaded(WPARAM, LPARAM)
{
	int numAccounts;
	PROTOACCOUNT** accounts;
	ProtoEnumAccounts(&numAccounts, &accounts);

	// reset the counter
	for (int i = 0; i < numAccounts; i++)
		if (Proto_IsAccountEnabled(accounts[i]))
			db_set_dw(NULL, "UserOnline", accounts[i]->szModuleName, GetTickCount());

	return 0;
}

static int UserOnlineAccountsChanged(WPARAM eventCode, LPARAM lParam)
{
	PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;

	switch(eventCode) {
	case PRAC_ADDED:
	case PRAC_CHECKED:
		// reset the counter
		if (Proto_IsAccountEnabled(pa))
			db_set_dw(NULL, "UserOnline", pa->szModuleName, GetTickCount());
		break;
	}
	return 0;
}

int LoadUserOnlineModule(void)
{
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, UserOnlineSettingChanged);
	HookEvent(ME_PROTO_ACK, UserOnlineAck);
	HookEvent(ME_SYSTEM_MODULESLOADED, UserOnlineModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, UserOnlineAccountsChanged);
	SkinAddNewSoundEx("UserOnline", LPGEN("Alerts"), LPGEN("Online"));
	return 0;
}
