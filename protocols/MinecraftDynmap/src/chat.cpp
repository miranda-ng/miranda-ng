/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015 Robert Pösel

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

void MinecraftDynmapProto::UpdateChat(const TCHAR *name, const TCHAR *message, const time_t timestamp, bool addtolog)
{
	// replace % to %% to not interfere with chat color codes
	std::tstring smessage = message;
	utils::text::treplace_all(&smessage, _T("%"), _T("%%"));

	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_MESSAGE };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.time = timestamp;
	gce.ptszText = smessage.c_str();

	if (name == NULL) {
		gcd.iType = GC_EVENT_INFORMATION;
		name = TranslateT("Server");
		gce.bIsMe = false;
	}
	else gce.bIsMe = !mir_tstrcmp(name, this->nick_);

	if (addtolog)
		gce.dwFlags  |= GCEF_ADDTOLOG;

	gce.ptszNick = name;
	gce.ptszUID  = gce.ptszNick;
	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));
}

int MinecraftDynmapProto::OnChatEvent(WPARAM, LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);

	if(strcmp(hook->pDest->pszModule,m_szModuleName))
		return 0;

	switch(hook->pDest->iType)
	{
	case GC_USER_MESSAGE:
	{		
		std::string text = mir_t2a_cp(hook->ptszText,CP_UTF8);

		// replace %% back to %, because chat automatically does this to sent messages
		utils::text::replace_all(&text, "%%", "%");

		if (text.empty())
			break;

		// Outgoing message
		debugLogA("**Chat - Outgoing message: %s", text.c_str());
		ForkThread(&MinecraftDynmapProto::SendMsgWorker, new std::string(text));

		break;
	}

	case GC_USER_LEAVE:
	case GC_SESSION_TERMINATE:
		nick_ = NULL;
		SetStatus(ID_STATUS_OFFLINE);
		break;
	}

	return 0;
}

void MinecraftDynmapProto::AddChatContact(const TCHAR *name)
{	
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_JOIN };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.time = DWORD(time(0));
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = name;
	gce.ptszUID = gce.ptszNick;

	if (name == NULL)
		gce.bIsMe = false;
	else 
		gce.bIsMe = mir_tstrcmp(name, this->nick_);

	if (gce.bIsMe)
		gce.ptszStatus = _T("Admin");
	else
		gce.ptszStatus = _T("Normal");

	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));
}

void MinecraftDynmapProto::DeleteChatContact(const TCHAR *name)
{
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_PART };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = name;
	gce.ptszUID = gce.ptszNick;
	gce.time = DWORD(time(0));
	if (name == NULL)
		gce.bIsMe = false;
	else 
		gce.bIsMe = mir_tstrcmp(name, this->nick_);

	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));
}

INT_PTR MinecraftDynmapProto::OnJoinChat(WPARAM,LPARAM suppress)
{	
	ptrT tszTitle(mir_a2t(m_title.c_str()));

	// Create the group chat session
	GCSESSION gcw = {sizeof(gcw)};
	gcw.iType = GCW_PRIVMESS;
	gcw.ptszID = m_tszUserName;
	gcw.ptszName = tszTitle;
	gcw.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	if (m_iStatus == ID_STATUS_OFFLINE)
		return 0;

	// Create a group
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };
	
	gce.ptszStatus = _T("Admin");
	CallServiceSync(MS_GC_EVENT, NULL, reinterpret_cast<LPARAM>(&gce));
	
	gce.ptszStatus = _T("Normal");
	CallServiceSync(MS_GC_EVENT, NULL, reinterpret_cast<LPARAM>(&gce));
		
	// Note: Initialization will finish up in SetChatStatus, called separately
	if (!suppress)
		SetChatStatus(m_iStatus);

	return 0;
}

void MinecraftDynmapProto::SetTopic(const TCHAR *topic)
{		
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_TOPIC };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.time = ::time(NULL);
	gce.ptszText = topic;

	CallServiceSync(MS_GC_EVENT,0,  reinterpret_cast<LPARAM>(&gce));
}

INT_PTR MinecraftDynmapProto::OnLeaveChat(WPARAM,LPARAM)
{
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.time = ::time(NULL);

	CallServiceSync(MS_GC_EVENT,SESSION_OFFLINE,  reinterpret_cast<LPARAM>(&gce));
	CallServiceSync(MS_GC_EVENT,SESSION_TERMINATE,reinterpret_cast<LPARAM>(&gce));

	return 0;
}

void MinecraftDynmapProto::SetChatStatus(int status)
{
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.time = ::time(NULL);
	
	if (status == ID_STATUS_ONLINE)
	{		
		// Load actual name from database
		nick_ = db_get_tsa(NULL, m_szModuleName, MINECRAFTDYNMAP_KEY_NAME);
		if (nick_ == NULL) {
			nick_ = mir_tstrdup(TranslateT("You"));
			db_set_ts(NULL, m_szModuleName, MINECRAFTDYNMAP_KEY_NAME, nick_);
		}

		// Add self contact
		AddChatContact(nick_);

		CallServiceSync(MS_GC_EVENT,SESSION_INITDONE,reinterpret_cast<LPARAM>(&gce));
		CallServiceSync(MS_GC_EVENT,SESSION_ONLINE,  reinterpret_cast<LPARAM>(&gce));
	}
	else
	{
		CallServiceSync(MS_GC_EVENT,SESSION_OFFLINE,reinterpret_cast<LPARAM>(&gce));
	}
}

void MinecraftDynmapProto::ClearChat()
{
	GCDEST gcd = { m_szModuleName, m_tszUserName, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	CallServiceSync(MS_GC_EVENT, WINDOW_CLEARLOG, reinterpret_cast<LPARAM>(&gce));
}

// TODO: Could this be done better?
MCONTACT MinecraftDynmapProto::GetChatHandle()
{
	/*if (chatHandle_ != NULL)
		return chatHandle_;

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
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
	CallService(MS_GC_GETINFO, 0, (LPARAM)&gci);

	return gci.hContact;
}