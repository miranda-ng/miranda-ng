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

void CSkypeProto::InitGroupChatModule()
{
	GCREGISTER gcr = { sizeof(gcr) };
	gcr.iMaxText = 0;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

	HookProtoEvent(ME_GC_EVENT, &CSkypeProto::OnGroupChatEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CSkypeProto::OnGroupChatMenuHook);

	CreateProtoService(PS_JOINCHAT, &CSkypeProto::OnJoinChatRoom);
	CreateProtoService(PS_LEAVECHAT, &CSkypeProto::OnLeaveChatRoom);
}

void CSkypeProto::CloseAllChatChatSessions()
{
	GC_INFO gci = { 0 };
	gci.Flags = GCF_BYINDEX | GCF_ID | GCF_DATA;
	gci.pszModule = m_szModuleName;

	int count = CallServiceSync(MS_GC_GETSESSIONCOUNT, 0, (LPARAM)m_szModuleName);
	for (int i = 0; i < count; i++)
	{
		gci.iItem = i;
		if (!CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci))
		{
			GCDEST gcd = { m_szModuleName, gci.pszID, GC_EVENT_CONTROL };
			GCEVENT gce = { sizeof(gce), &gcd };
			CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
			CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
		}
	}
}

MCONTACT CSkypeProto::FindChatRoom(const char *chatname)
{
	MCONTACT hContact = NULL;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		if (!isChatRoom(hContact))
			continue;

		ptrA cChatname(getStringA(hContact, "ChatID"));
		if (mir_strcmpi(chatname, cChatname) == 0)
			break;
	}
	return hContact;
}

MCONTACT CSkypeProto::AddChatRoom(const char *chatname)
{
	MCONTACT hContact = FindChatRoom(chatname);
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

int CSkypeProto::OnGroupChatEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (!gch)
	{
		return 1;
	}
	else if (strcmp(gch->pDest->pszModule, m_szModuleName) != 0)
	{
		return 0;
	}
	return 0;
}

void CSkypeProto::StartChatRoom(MCONTACT hChatRoom, bool showWindow)
{
	ptrT tszChatID(getTStringA(hChatRoom, "ChatID"));
	ptrT tszNick(getTStringA(hChatRoom, "Nick"));
	if (tszChatID == NULL)
		return;

	// start chat session
	GCSESSION gcw = { 0 };
	gcw.cbSize = sizeof(gcw);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = tszNick;
	gcw.ptszID = tszChatID;
	gcw.dwItemData = (DWORD)tszChatID;
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	GCDEST gcd = { m_szModuleName, tszChatID, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	CallServiceSync(MS_GC_EVENT, showWindow ? SESSION_INITDONE : WINDOW_HIDDEN, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);

}

int CSkypeProto::OnGroupChatMenuHook(MCONTACT, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;
	if (stricmp(gcmi->pszModule, m_szModuleName) != 0)
	{
		return 0;
	}
	return 0;
}

INT_PTR CSkypeProto::OnJoinChatRoom(MCONTACT hContact, LPARAM)
{
	if (hContact)
	{
	}
	return 0;
}

INT_PTR CSkypeProto::OnLeaveChatRoom(MCONTACT hContact, LPARAM)
{
	if (hContact)
	{
	}
	return 0;
}

/* CHAT EVENT */

void CSkypeProto::OnChatEvent(JSONNODE *node)
{
	ptrA clientMsgId(mir_t2a(ptrT(json_as_string(json_get(node, "clientmessageid")))));
	ptrA skypeEditedId(mir_t2a(ptrT(json_as_string(json_get(node, "skypeeditedid")))));
	
	ptrA from(mir_t2a(ptrT(json_as_string(json_get(node, "from")))));

	ptrT composeTime(json_as_string(json_get(node, "composetime")));
	time_t timestamp = IsoToUnixTime(composeTime);

	ptrA content(mir_t2a(ptrT(json_as_string(json_get(node, "content")))));
	//int emoteOffset = json_as_int(json_get(node, "skypeemoteoffset"));

	ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(node, "conversationLink")))));
	ptrA chatname(ChatUrlToName(conversationLink));

	ptrA topic(mir_t2a(ptrT(json_as_string(json_get(node, "threadtopic")))));
	
	MCONTACT hChatRoom = AddChatRoom(chatname);
	StartChatRoom(hChatRoom);
	
	ptrA messageType(mir_t2a(ptrT(json_as_string(json_get(node, "messagetype")))));
	if (!mir_strcmpi(messageType, "Text") || !mir_strcmpi(messageType, "RichText"))
	{

	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/AddMember"))
	{

	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/DeleteMember"))
	{

	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/TopicUpdate"))
	{

	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/RoleUpdate"))
	{

	}
	return; //chats not supported
}