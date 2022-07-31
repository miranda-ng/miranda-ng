/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"

static int uniqueEventId = 0;

static int UserOnlineSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact == NULL || strcmp(cws->szSetting, "Status"))
		return 0;

	int newStatus = cws->value.wVal;
	int oldStatus = g_plugin.getWord(hContact, "OldStatus", ID_STATUS_OFFLINE);
	g_plugin.setWord(hContact, "OldStatus", (uint16_t)newStatus);
	if (Ignore_IsIgnored(hContact, IGNOREEVENT_USERONLINE)) return 0;
	if (Contact::IsHidden(hContact)) return 0;
	if (newStatus == ID_STATUS_OFFLINE && oldStatus != ID_STATUS_OFFLINE) {
		// Remove the event from the queue if it exists since they are now offline
		MEVENT lastEvent = g_plugin.getDword(hContact, "LastEvent");
		if (lastEvent) {
			g_clistApi.pfnRemoveEvent(hContact, lastEvent);
			g_plugin.setDword(hContact, "LastEvent", 0);
		}
	}

	if ((newStatus == ID_STATUS_ONLINE || newStatus == ID_STATUS_FREECHAT) && oldStatus != ID_STATUS_ONLINE && oldStatus != ID_STATUS_FREECHAT) {
		uint32_t ticked = g_plugin.getDword(cws->szModule, GetTickCount());
		// only play the sound (or show event) if this event happens at least 10 secs after the proto went from offline
		if (GetTickCount() - ticked > (1000 * 10)) {
			wchar_t tooltip[256];
			mir_snwprintf(tooltip, TranslateT("%s is online"), Clist_GetContactDisplayName(hContact));

			CLISTEVENT cle = {};
			cle.flags = CLEF_ONLYAFEW | CLEF_UNICODE;
			cle.hContact = hContact;
			cle.hDbEvent = uniqueEventId++;
			cle.hIcon = Skin_LoadIcon(SKINICON_OTHER_USERONLINE, false);
			cle.pszService = "UserOnline/Description";
			cle.szTooltip.w = tooltip;
			g_clistApi.pfnAddEvent(&cle);
			IcoLib_ReleaseIcon(cle.hIcon, 0);
			g_plugin.setDword(cle.hContact, "LastEvent", cle.hDbEvent);
			Skin_PlaySound(MODULENAME);
		}
	}
	return 0;
}

static int UserOnlineAck(WPARAM, LPARAM lParam)
{
	ACKDATA * ack = (ACKDATA*)lParam;
	if (ack != nullptr && ack->szModule && ack->type == ACKTYPE_STATUS && ack->result == ACKRESULT_SUCCESS && ack->hProcess == (HANDLE)ID_STATUS_OFFLINE) {
		// if going from offline to any other mode, remember when it happened.
		g_plugin.setDword(ack->szModule, GetTickCount());
	}
	return 0;
}

static int UserOnlineModulesLoaded(WPARAM, LPARAM)
{
	for (auto &pa : Accounts())
		if (pa->IsEnabled())
			g_plugin.setDword(pa->szModuleName, GetTickCount());

	return 0;
}

static int UserOnlineAccountsChanged(WPARAM eventCode, LPARAM lParam)
{
	PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;

	switch (eventCode) {
	case PRAC_ADDED:
	case PRAC_CHECKED:
		// reset the counter
		if (pa->IsEnabled())
			g_plugin.setDword(pa->szModuleName, GetTickCount());
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

	g_plugin.addSound(MODULENAME, LPGENW("Alerts"), LPGENW("Online"));
	return 0;
}
