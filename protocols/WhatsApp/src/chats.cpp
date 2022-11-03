/*

WhatsApp plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

void WhatsAppProto::GC_Init(WAUser *pUser)
{
	CMStringW wszId(Utf2T(pUser->szId));

	pUser->si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, getMStringW(pUser->hContact, "Nick"));

	Chat_AddGroup(pUser->si, TranslateT("Owner"));
	Chat_AddGroup(pUser->si, TranslateT("SuperAdmin"));
	Chat_AddGroup(pUser->si, TranslateT("Admin"));
	Chat_AddGroup(pUser->si, TranslateT("Participant"));

	if (pUser->bInited) {
		Chat_Control(m_szModuleName, wszId, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
		Chat_Control(m_szModuleName, wszId, SESSION_ONLINE);
	}
	else GC_GetMetadata(pUser->szId);
}

void WhatsAppProto::GC_GetMetadata(const char *szId)
{
	WANodeIq iq(IQ::GET, "w:g2", szId);
	iq.addChild("query")->addAttr("request", "interactive");
	WSSendNode(iq, &WhatsAppProto::OnIqGcMetadata);
}

void WhatsAppProto::OnIqGcMetadata(const WANode &node)
{
	auto *pGroup = node.getChild("group");
	auto *pChatUser = FindUser(node.getAttr("from"));
	if (pChatUser == nullptr || pGroup == nullptr)
		return;

	CMStringA szOwner(pGroup->getAttr("creator")), szNick, szRole;

	for (auto &it : pGroup->getChildren()) {
		if (it->title == "description") {
			CMStringA szDescr = it->getBody();
			if (!szDescr.IsEmpty()) {
				GCEVENT gce = {m_szModuleName, 0, GC_EVENT_INFORMATION};
				gce.dwFlags = GCEF_UTF8;
				gce.pszID.a = pChatUser->szId;
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

			GCEVENT gce = {m_szModuleName, 0, GC_EVENT_JOIN};
			gce.dwFlags = GCEF_UTF8;
			gce.pszID.a = pChatUser->szId;
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

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_TOPIC };
		gce.dwFlags = GCEF_UTF8;
		gce.pszID.a	 = pChatUser->szId;
		gce.pszUID.a = pszUser;
		gce.pszText.a = pszSubject;
		gce.time = iSubjectTime;
		Chat_Event(&gce);

		setUString(pChatUser->hContact, "Nick", pszSubject);
	}

	pChatUser->bInited = true;
	CMStringW wszId(Utf2T(pChatUser->szId));
	Chat_Control(m_szModuleName, wszId, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, wszId, SESSION_ONLINE);
}
