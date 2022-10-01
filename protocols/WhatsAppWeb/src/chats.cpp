/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

void WhatsAppProto::InitChat(WAUser *pUser)
{
	CMStringA jid = "";
	CMStringW wszId(Utf2T(jid.c_str())), wszNick(Utf2T(""));

	setWString(pUser->hContact, "Nick", wszNick);

	pUser->si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, wszNick);

	Chat_AddGroup(pUser->si, TranslateT("Owner"));
	Chat_AddGroup(pUser->si, TranslateT("SuperAdmin"));
	Chat_AddGroup(pUser->si, TranslateT("Admin"));
	Chat_AddGroup(pUser->si, TranslateT("Participant"));

	Chat_Control(m_szModuleName, wszId, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, wszId, SESSION_ONLINE);

	if (!pUser->bInited) {
		// CMStringA query(FORMAT, "[\"query\",\"GroupMetadata\",\"%s\"]", jid.c_str());
		// WSSend(query, &WhatsAppProto::OnGetChatInfo, pUser);
	}
}

void WhatsAppProto::OnGetChatInfo(const JSONNode &root, void *param)
{
	auto *pChatUser = (WAUser *)param;
	pChatUser->bInited = true;

	CMStringW wszOwner(root["owner"].as_mstring()), wszNick;

	for (auto &it : root["participants"]) {
		CMStringW jid(it["id"].as_mstring());
		CMStringA szJid(jid);

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_JOIN };
		gce.pszID.w = pChatUser->si->ptszID;
		gce.pszUID.w = jid;
		gce.bIsMe = (szJid == m_szJid);

		if (jid == wszOwner)
			gce.pszStatus.w = L"Owner";
		else if (it["isSuperAdmin"].as_bool())
			gce.pszStatus.w = L"SuperAdmin";
		else if (it["isAdmin"].as_bool())
			gce.pszStatus.w = L"Admin";
		else
			gce.pszStatus.w = L"Participant";

		if (gce.bIsMe)
			wszNick = getMStringW(DBKEY_NICK);
		else if (auto *pUser = FindUser(szJid))
			wszNick = Clist_GetContactDisplayName(pUser->hContact);
		else {
			int iPos = jid.Find('@');
			wszNick = (iPos != -1) ? jid.Left(iPos - 1) : jid;
		}

		gce.pszNick.w = wszNick;
		Chat_Event(&gce);
	}

	if (pChatUser->arHistory.getCount()) {
		for (auto &it : pChatUser->arHistory) {
			CMStringW jid(it->jid), text(Utf2T(it->text));

			GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
			gce.pszID.w = pChatUser->si->ptszID;
			gce.dwFlags = GCEF_ADDTOLOG;
			gce.pszUID.w = jid;
			gce.pszText.w = text;
			gce.time = it->timestamp;
			gce.bIsMe = (it->jid == m_szJid);
			Chat_Event(&gce);
		}
		pChatUser->arHistory.destroy();
	}

	CMStringW wszSubject(root["subject"].as_mstring());
	if (!wszSubject.IsEmpty()) {
		time_t iSubjectTime(root["subjectTime"].as_int());
		CMStringW wszSubjectSet(root["subjectOwner"].as_mstring());

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_TOPIC };
		gce.pszID.w	 = pChatUser->si->ptszID;
		gce.pszUID.w = wszSubjectSet;
		gce.pszText.w = wszSubject;
		gce.time = iSubjectTime;
		Chat_Event(&gce);
	}
}
