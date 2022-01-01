/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015-17 Robert Pösel, 2017-22 Miranda NG team

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

void MinecraftDynmapProto::UpdateChat(const char *name, const char *message, const time_t timestamp, bool addtolog)
{
	// replace % to %% to not interfere with chat color codes
	CMStringA szMessage(message);
	szMessage.Replace("%", "%%");

	GCEVENT gce = { m_szModuleName, szRoomName, GC_EVENT_MESSAGE };
	gce.dwFlags = GCEF_UTF8;
	gce.time = timestamp;
	gce.pszText.a = szMessage.c_str();

	if (name == NULL) {
		gce.iType = GC_EVENT_INFORMATION;
		name = TranslateU("Server");
		gce.bIsMe = false;
	}
	else gce.bIsMe = (m_nick == name);

	if (addtolog)
		gce.dwFlags |= GCEF_ADDTOLOG;

	gce.pszUID.a = gce.pszNick.a = name;
	Chat_Event(&gce);
}

int MinecraftDynmapProto::OnChatEvent(WPARAM, LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);
	if(strcmp(hook->si->pszModule,m_szModuleName))
		return 0;

	switch(hook->iType) {
	case GC_USER_MESSAGE:
		{
			CMStringA szText(ptrA(mir_utf8encodeW(hook->ptszText)));
			szText.Replace("%%", "%");
			if (szText.IsEmpty())
				break;

			// Outgoing message
			debugLogA("**Chat - Outgoing message: %s", szText.c_str());
			ForkThread(&MinecraftDynmapProto::SendMsgWorker, new std::string(szText));
		}
		break;

	case GC_SESSION_TERMINATE:
		m_nick.clear();
		SetStatus(ID_STATUS_OFFLINE);
		break;
	}

	return 0;
}

void MinecraftDynmapProto::AddChatContact(const char *name)
{	
	GCEVENT gce = { m_szModuleName, szRoomName, GC_EVENT_JOIN };
	gce.time = uint32_t(time(0));
	gce.dwFlags = GCEF_UTF8 + GCEF_ADDTOLOG;
	gce.pszUID.a = gce.pszNick.a = name;
	gce.bIsMe = (m_nick == name);

	if (gce.bIsMe)
		gce.pszStatus.a = "Admin";
	else
		gce.pszStatus.a = "Normal";

	Chat_Event(&gce);
}

void MinecraftDynmapProto::DeleteChatContact(const char *name)
{
	GCEVENT gce = { m_szModuleName, szRoomName, GC_EVENT_PART };
	gce.dwFlags = GCEF_UTF8 + GCEF_ADDTOLOG;
	gce.pszUID.a = gce.pszNick.a = name;
	gce.time = uint32_t(time(0));
	gce.bIsMe = (m_nick == name);
	Chat_Event(&gce);
}

INT_PTR MinecraftDynmapProto::OnJoinChat(WPARAM,LPARAM suppress)
{	
	ptrW tszTitle(mir_a2u_cp(m_title.c_str(), CP_UTF8));

	// Create the group chat session
	SESSION_INFO *si = Chat_NewSession(GCW_PRIVMESS, m_szModuleName, m_tszUserName, tszTitle);
	if (!si || m_iStatus == ID_STATUS_OFFLINE)
		return 0;

	// Create a group
	Chat_AddGroup(si, TranslateT("Admin"));
	Chat_AddGroup(si, TranslateT("Normal"));
		
	// Note: Initialization will finish up in SetChatStatus, called separately
	if (!suppress)
		SetChatStatus(m_iStatus);

	return 0;
}

void MinecraftDynmapProto::SetTopic(const char *topic)
{		
	GCEVENT gce = { m_szModuleName, szRoomName, GC_EVENT_TOPIC };
	gce.dwFlags = GCEF_UTF8;
	gce.time = ::time(0);
	gce.pszText.a = topic;
	Chat_Event( &gce);
}

INT_PTR MinecraftDynmapProto::OnLeaveChat(WPARAM,LPARAM)
{
	Chat_Control(m_szModuleName, m_tszUserName, SESSION_OFFLINE);
	Chat_Terminate(m_szModuleName, m_tszUserName);
	return 0;
}

void MinecraftDynmapProto::SetChatStatus(int status)
{
	if (status == ID_STATUS_ONLINE)
	{		
		// Load actual name from database
		ptrA nick(db_get_sa(0, m_szModuleName, MINECRAFTDYNMAP_KEY_NAME, Translate("You")));
		m_nick = nick;

		// Add self contact
		AddChatContact(m_nick.c_str());

		Chat_Control(m_szModuleName, m_tszUserName, SESSION_INITDONE);
		Chat_Control(m_szModuleName, m_tszUserName, SESSION_ONLINE);
	}
	else Chat_Control(m_szModuleName, m_tszUserName, SESSION_OFFLINE);
}

void MinecraftDynmapProto::ClearChat()
{
	Chat_Control(m_szModuleName, m_tszUserName, WINDOW_CLEARLOG);
}

// TODO: Could this be done better?
MCONTACT MinecraftDynmapProto::GetChatHandle()
{
	/*if (chatHandle_ != NULL)
		return chatHandle_;

	for (auto &hContact : AccContacts()) {
		if (db_get_b(hContact, m_szModuleName, "ChatRoom", 0) > 0) {
			ptrA id = db_get_sa(hContact, m_szModuleName, "ChatRoomId");
			if (id != NULL && !strcmp(id, m_szModuleName))
				return hContact;
		}
	}

	return NULL;*/

	GC_INFO gci = {0};
	gci.Flags = GCF_HCONTACT;
	gci.pszModule = m_szModuleName;
	gci.pszID = m_tszUserName;
	Chat_GetInfo(&gci);

	return gci.hContact;
}