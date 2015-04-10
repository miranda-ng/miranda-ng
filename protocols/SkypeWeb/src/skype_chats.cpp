/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

MCONTACT CSkypeProto::GetChat(const char *skypename)
{
	MCONTACT hContact = NULL;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		ptrA cSkypename(getStringA(hContact, "ChatID"));
		if (mir_strcmpi(skypename, cSkypename) == 0)
			break;
	}
	return hContact;
}

MCONTACT CSkypeProto::AddChatRoom(const char *chatname)
{
	MCONTACT hContact = GetChat(chatname);
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		setString(hContact, "ChatID", chatname);

		TCHAR title[MAX_PATH];
		mir_sntprintf(title, SIZEOF(title), _T("%s #%d"), TranslateT("Groupchat"), chatname);
		setTString(hContact, "Nick", title);

		DBVARIANT dbv;
		if (!db_get_s(NULL, "Chat", "AddToGroup", &dbv, DBVT_TCHAR))
		{
			db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
			db_free(&dbv);
		}

		setByte(hContact, "ChatRoom", 1);
	}
	return hContact;
}

void CSkypeProto::SetChatStatus(MCONTACT hContact, int iStatus)
{
	ptrT tszChatID(getTStringA(hContact, "ChatID"));
	ptrT tszNick(getTStringA(hContact, "Nick"));
	if (tszChatID == NULL)
		return;

	// start chat session
	GCSESSION gcw = {0};
	gcw.cbSize = sizeof(gcw);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = tszNick;
	gcw.ptszID = tszChatID; 
	gcw.dwItemData = (DWORD)tszChatID;
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	GCDEST gcd = { m_szModuleName, tszChatID, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	CallServiceSync(MS_GC_EVENT, true ? SESSION_INITDONE : WINDOW_HIDDEN, (LPARAM)&gce); 
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
}