/*
Copyright © 2016-17 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

enum {
	IDM_CANCEL,

	IDM_CHANGENICK, IDM_INVITE
};

static gc_item sttLogListItems[] =
{
	{ LPGENW("Change &nickname"), IDM_CHANGENICK, MENU_ITEM },
	{ LPGENW("&Invite a user"), IDM_INVITE, MENU_ITEM },
};

int CDiscordProto::GroupchatMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS* gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == NULL)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	CDiscordUser *pChat = FindUserByChannel(_wtoi64(gcmi->pszID));
	if (pChat == NULL)
		return 0;

	if (gcmi->Type == MENU_ON_LOG) {
		gcmi->nItems = _countof(sttLogListItems);
		gcmi->Item = sttLogListItems;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::Chat_SendPrivateMessage(GCHOOK *gch)
{
	SnowFlake userId = _wtoi64(gch->ptszUID);

	MCONTACT hContact;
	CDiscordUser *pUser = FindUser(userId);
	if (pUser == NULL) {
		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.id.w = (wchar_t*)gch->ptszUID;
		psr.nick.w = (wchar_t*)gch->ptszNick;
		if ((hContact = AddToList(PALF_TEMPORARY, &psr)) == 0)
			return;

		setId(hContact, DB_KEY_ID, userId);
		setId(hContact, DB_KEY_CHANNELID, _wtoi64(gch->pDest->ptszID));
		setWString(hContact, "Nick", gch->ptszNick);
		db_set_b(hContact, "CList", "Hidden", 1);
		db_set_dw(hContact, "Ignore", "Mask1", 0);
	}
	else hContact = pUser->hContact;

	CallService(MS_MSG_SENDMESSAGE, hContact, 0);
}

void CDiscordProto::Chat_ProcessLogMenu(GCHOOK *gch)
{
	CDiscordUser *pUser = FindUserByChannel(_wtoi64(gch->pDest->ptszID));
	if (pUser == NULL)
		return;

	switch (gch->dwData) {
	case IDM_INVITE:
		break;

	case IDM_CHANGENICK:
		ENTER_STRING es = { sizeof(es) };
		es.caption = TranslateT("Enter your new nick name:");
		es.type = ESF_COMBO;
		es.szModuleName = m_szModuleName;
		es.szDataPrefix = "chat_nick";
		es.recentCount = 5;
		if (EnterString(&es)) {
			JSONNode root; root << WCHAR_PARAM("nick", es.ptszResult);
			CMStringA szUrl(FORMAT, "/guilds/%lld/members/@me/nick", pUser->guildId);
			Push(new AsyncHttpRequest(this, REQUEST_PATCH, szUrl, NULL, &root));
			mir_free(es.ptszResult);
		}
		break;
	}
}

int CDiscordProto::GroupchatEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (gch == NULL)
		return 0;

	if (mir_strcmpi(gch->pDest->pszModule, m_szModuleName))
		return 0;

	switch (gch->pDest->iType) {
	case GC_USER_MESSAGE:
		if (mir_wstrlen(gch->ptszText) > 0) {
			rtrimw(gch->ptszText);

			if (m_bOnline) {
				wchar_t *wszText = NEWWSTR_ALLOCA(gch->ptszText);
				Chat_UnescapeTags(wszText);

				JSONNode body; body << WCHAR_PARAM("content", wszText);
				CMStringA szUrl(FORMAT, "/channels/%S/messages", gch->pDest->ptszID);
				Push(new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnReceiveMessage, &body));
			}
		}
		break;

	case GC_USER_PRIVMESS:
		Chat_SendPrivateMessage(gch);
		break;

	case GC_USER_LOGMENU:
		Chat_ProcessLogMenu(gch);
		break;

	case GC_USER_NICKLISTMENU:
		break;
	}

	return 0;
}
