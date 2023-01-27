/*

WhatsApp plugin for Miranda NG
Copyright © 2019-23 George Hazan

*/

#include "stdafx.h"

void WhatsAppProto::GC_RefreshMetadata()
{
	for (auto &it : m_arUsers) {
		if (it->bIsGroupChat) {
			GC_GetAllMetadata();
			break;
		}
	}
}

void WhatsAppProto::GC_GetAllMetadata()
{
	WANodeIq iq(IQ::GET, "w:g2", "@g.us");
	auto *pRoot = iq.addChild("participating");
	*pRoot << XCHILD("participants") << XCHILD("description");
	WSSendNode(iq, &WhatsAppProto::OnIqGcGetAllMetadata);
}

void WhatsAppProto::OnIqGcGetAllMetadata(const WANode &node)
{
	if (auto *pGroup = node.getChild("groups"))
		for (auto &it : pGroup->getChildren())
			GC_ParseMetadata(it);	
}

void WhatsAppProto::GC_ParseMetadata(const WANode *pGroup)
{
	auto *pszId = pGroup->getAttr("id");
	if (pszId == nullptr)
		return;

	auto *pChatUser = AddUser(CMStringA(pszId) + "@g.us", false);
	if (pChatUser == nullptr)
		return;

	CMStringW wszId(Utf2T(pChatUser->szId));

	auto *si = pChatUser->si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, getMStringW(pChatUser->hContact, "Nick"));

	Chat_AddGroup(si, TranslateT("Owner"));
	Chat_AddGroup(si, TranslateT("SuperAdmin"));
	Chat_AddGroup(si, TranslateT("Admin"));
	Chat_AddGroup(si, TranslateT("Participant"));

	CMStringA szOwner(pGroup->getAttr("creator")), szNick, szRole;

	for (auto &it : pGroup->getChildren()) {
		if (it->title == "description") {
			CMStringA szDescr = it->getBody();
			if (!szDescr.IsEmpty()) {
				GCEVENT gce = { si, GC_EVENT_INFORMATION };
				gce.dwFlags = GCEF_UTF8;
				gce.pszText.a = szDescr.c_str();
				Chat_Event(&gce);
			}
		}
		else if (it->title == "member_add_mode") {
			szRole = it->getBody();
		}
		else if (it->title == "participant") {
			auto *jid = it->getAttr("jid");

			// if role isn't specified, use the default one
			auto *role = it->getAttr("type");
			if (role == nullptr)
				role = szRole;

			GCEVENT gce = { si, GC_EVENT_JOIN };
			gce.dwFlags = GCEF_UTF8;
			gce.pszUID.a = jid;
			gce.bIsMe = (jid == m_szJid);

			if (jid == szOwner)
				gce.pszStatus.a = "Owner";
			else if (!mir_strcmp(role, "superadmin"))
				gce.pszStatus.a = "SuperAdmin";
			else if (!mir_strcmp(role, "adminadd"))
				gce.pszStatus.a = "Admin";
			else
				gce.pszStatus.a = "Participant";

			if (gce.bIsMe)
				szNick = ptrA(getUStringA(DBKEY_NICK));
			else if (auto *pUser = FindUser(jid))
				szNick = T2Utf(Clist_GetContactDisplayName(pUser->hContact)).get();
			else
				szNick = WAJid(jid).user;

			gce.pszNick.a = szNick;
			Chat_Event(&gce);
		}
	}

	if (auto *pszSubject = pGroup->getAttr("subject")) {
		time_t iSubjectTime = pGroup->getAttrInt("s_t");
		auto *pszUser = pGroup->getAttr("s_o");
		if (m_szJid == pszUser)
			szNick = ptrA(getUStringA(DBKEY_NICK));
		else if (auto *pUser = FindUser(pszUser))
			szNick = T2Utf(Clist_GetContactDisplayName(pUser->hContact)).get();
		else
			szNick = WAJid(pszUser).user;

		GCEVENT gce = { si, GC_EVENT_TOPIC };
		gce.dwFlags = GCEF_UTF8;
		gce.pszUID.a = pszUser;
		gce.pszText.a = pszSubject;
		gce.time = iSubjectTime;
		Chat_Event(&gce);

		setUString(pChatUser->hContact, "Nick", pszSubject);
	}

	pChatUser->bInited = true;
	Chat_Control(si, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(si, SESSION_ONLINE);
}

/////////////////////////////////////////////////////////////////////////////////////////

int WhatsAppProto::GcEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (gch == nullptr)
		return 0;

	if (mir_strcmpi(gch->si->pszModule, m_szModuleName))
		return 0;

	auto *pUser = FindUser(T2Utf(gch->si->ptszID));
	if (pUser == nullptr)
		return 0;

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		if (gch->ptszText && mir_wstrlen(gch->ptszText) > 0) {
			rtrimw(gch->ptszText);
			Chat_UnescapeTags(gch->ptszText);
			SendTextMessage(pUser->szId, T2Utf(gch->ptszText));
		}
		break;

	case GC_USER_PRIVMESS:
		break;

	case GC_USER_LOGMENU:
		break;

	case GC_USER_NICKLISTMENU:
		break;
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

int WhatsAppProto::GcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS* gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == nullptr)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	auto *pUser = FindUser(T2Utf(gcmi->pszID));
	if (pUser == nullptr)
		return 0;

	if (gcmi->Type == MENU_ON_LOG) {
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
	}
	return 0;
}
