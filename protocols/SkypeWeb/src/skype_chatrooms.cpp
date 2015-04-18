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

static LPCTSTR sttStatuses[] = { LPGENT("User"), LPGENT("Admin") };

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
		mir_sntprintf(title, SIZEOF(title), _T("%s #%s"), TranslateT("Groupchat"), ptrT(mir_a2t(chatname)));
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

	GCDEST gcdg = { m_szModuleName, tszChatID, GC_EVENT_ADDGROUP };
	GCEVENT gceg = { sizeof(gce), &gcdg };
	for (int i = SIZEOF(sttStatuses)-1; i >= 0; i--) {
		gceg.ptszStatus = TranslateTS(sttStatuses[i]);
		CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gceg);
	}

	SendRequest(new GetChatInfoRequest(RegToken, ptrA(mir_t2a(tszChatID)), Server), &CSkypeProto::OnGetChatInfo); 

}

int CSkypeProto::OnGroupChatMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;
	if (stricmp(gcmi->pszModule, m_szModuleName) != 0)
	{
		return 0;
	}
	return 0;
}

INT_PTR CSkypeProto::OnJoinChatRoom(WPARAM hContact, LPARAM)
{
	if (hContact)
	{
	}
	return 0;
}

INT_PTR CSkypeProto::OnLeaveChatRoom(WPARAM hContact, LPARAM)
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

	time_t timestamp = IsoToUnixTime(ptrT(json_as_string(json_get(node, "composetime"))));

	ptrA content(mir_t2a(ptrT(json_as_string(json_get(node, "content")))));
	ptrT tcontent(json_as_string(json_get(node, "content")));
	//int emoteOffset = json_as_int(json_get(node, "skypeemoteoffset"));

	ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(node, "conversationLink")))));
	ptrA chatname(ChatUrlToName(conversationLink));

	ptrA topic(mir_t2a(ptrT(json_as_string(json_get(node, "threadtopic")))));
	
	MCONTACT hChatRoom = AddChatRoom(chatname);
	StartChatRoom(hChatRoom);
	
	ptrA messageType(mir_t2a(ptrT(json_as_string(json_get(node, "messagetype")))));
	if (!mir_strcmpi(messageType, "Text") || !mir_strcmpi(messageType, "RichText"))
	{
		GCDEST gcd = { m_szModuleName, ptrT(mir_a2t(chatname)), GC_EVENT_MESSAGE };
		GCEVENT gce = { sizeof(GCEVENT), &gcd };
		gce.bIsMe = IsMe(ContactUrlToName(from));
		gce.ptszUID = ptrT(mir_a2t(ContactUrlToName(from)));
		gce.time = timestamp;
		gce.ptszNick = ptrT(mir_a2t(ContactUrlToName(from)));
		gce.ptszText = tcontent;
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/AddMember"))
	{
		ptrA xinitiator, xtarget, initiator, target;
		//content = <addmember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></addmember>

		HXML xml = xi.parseString(ptrT(mir_a2t(content)), 0, _T("addmember"));
		if (xml != NULL) {

			HXML xmlNode = xi.getChildByPath(xml, _T("target"), 0);
			xtarget = node != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

			xi.destroyNode(xml);
		}

		target = ParseUrl(xtarget, "8:");

		GCDEST gcd = { m_szModuleName, ptrT(mir_a2t(chatname)), GC_EVENT_JOIN };
		GCEVENT gce = { sizeof(GCEVENT), &gcd };
		gce.bIsMe = IsMe(target);
		gce.ptszUID = ptrT(mir_a2t(target));
		gce.ptszNick = ptrT(mir_a2t(target));
		gce.ptszStatus = TranslateT("User");
		gce.time = timestamp;
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/DeleteMember"))
	{
		ptrA xinitiator, xtarget, initiator, target;
		//content = <addmember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></addmember>

		HXML xml = xi.parseString(ptrT(mir_a2t(content)), 0, _T("deletemember"));
		if (xml != NULL) {
			HXML xmlNode = xi.getChildByPath(xml, _T("initiator"), 0);
			xinitiator = node != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

			xmlNode = xi.getChildByPath(xml, _T("target"), 0);
			xtarget = node != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

			xi.destroyNode(xml);
		}
		if(xtarget == NULL)
			return;

		target = ParseUrl(xtarget, "8:");

		bool isKick = false;
		initiator = ParseUrl(xinitiator, "8:");
		isKick = true;

		if (isKick)
		{
			GCDEST gcd = { m_szModuleName, ptrT(mir_a2t(chatname)), GC_EVENT_KICK };
			GCEVENT gce = { sizeof(GCEVENT), &gcd };
			gce.ptszUID = ptrT(mir_a2t(target));
			gce.ptszNick = ptrT(mir_a2t(target));
			gce.ptszStatus = ptrT(mir_a2t(initiator));
			gce.time = timestamp;
			CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		}
		else
		{
			GCDEST gcd = { m_szModuleName, ptrT(mir_a2t(chatname)), GC_EVENT_PART };
			GCEVENT gce = { sizeof(GCEVENT), &gcd };
			gce.ptszUID = ptrT(mir_a2t(target));
			gce.ptszNick = ptrT(mir_a2t(target));
			gce.time = timestamp;
			CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		}
	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/TopicUpdate"))
	{

	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/RoleUpdate"))
	{

	}
}

void CSkypeProto::OnGetChatInfo(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
		return;

	JSONROOT root(response->pData);
	JSONNODE *members = json_get(root, "members");
	ptrA chatId(ChatUrlToName(ptrA(mir_t2a(ptrT(json_as_string(json_get(root, "messages")))))));

	for (size_t i = 0; i < json_size(members); i++)
	{
		JSONNODE *member = json_at(members, i);

		ptrA username(ContactUrlToName(ptrA(mir_t2a(ptrT(json_as_string(json_get(member, "userLink")))))));
		ptrT role(json_as_string(json_get(member, "role")));

		GCDEST gcd = { m_szModuleName, ptrT(mir_a2t(chatId)), GC_EVENT_JOIN };
		GCEVENT gce = { sizeof(GCEVENT), &gcd };
		gce.bIsMe = false;
		gce.ptszUID = ptrT(mir_a2t(username));
		gce.ptszNick = ptrT(mir_a2t(username));
		gce.ptszStatus = TranslateTS(role);
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	}
}