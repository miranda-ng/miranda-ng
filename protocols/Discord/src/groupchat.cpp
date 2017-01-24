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
				CMStringA szUrl(FORMAT, "/channels/%s/messages", gch->pDest->ptszID);
				Push(new AsyncHttpRequest(this, REQUEST_POST, szUrl, &CDiscordProto::OnReceiveMessage, &body));
			}
		}
		break;

	case GC_USER_PRIVMESS:
		MCONTACT hContact;
		{
			SnowFlake userId = _wtoi64(gch->ptszUID);

			CDiscordUser *pUser = FindUser(userId);
			if (pUser == NULL) {
				PROTOSEARCHRESULT psr = { sizeof(psr) };
				psr.id.w = (wchar_t*)gch->ptszUID;
				psr.nick.w = (wchar_t*)gch->ptszNick;
				if ((hContact = AddToList(PALF_TEMPORARY, &psr)) == 0)
					return 0;

				setId(hContact, DB_KEY_ID, userId);
				setId(hContact, DB_KEY_CHANNELID, _wtoi64(gch->pDest->ptszID));
				setWString(hContact, "Nick", gch->ptszNick);
				db_set_b(hContact, "CList", "Hidden", 1);
				db_set_dw(hContact, "Ignore", "Mask1", 0);
			}
			else hContact = pUser->hContact;
		}
		CallService(MS_MSG_SENDMESSAGE, hContact, 0);
		break;

	case GC_USER_LOGMENU:
	case GC_USER_NICKLISTMENU:
		break;
	}

	return 0;
}

int CDiscordProto::GroupchatMenuHook(WPARAM, LPARAM)
{
	return 0;
}
