/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-12 Robert Pösel

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

#include "common.h"

void FacebookProto::UpdateChat(const char *chat_id, const char *id, const char *name, const char *message)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = mir_a2t(chat_id);

	GCEVENT gce  = {sizeof(gce)};
	gce.pDest    = &gcd;
	gce.ptszText = mir_a2t_cp(message,CP_UTF8);
	gce.time     = ::time(NULL);
	gce.dwFlags  = GC_TCHAR;
	gcd.iType  = GC_EVENT_MESSAGE;
	gce.bIsMe = !strcmp(id,facy.self_.user_id.c_str());
	gce.dwFlags  |= GCEF_ADDTOLOG;

	gce.ptszNick = mir_a2t_cp(name,CP_UTF8);
	gce.ptszUID  = mir_a2t(id);

	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));

	mir_free(const_cast<TCHAR*>(gce.ptszUID));
	mir_free(const_cast<TCHAR*>(gce.ptszNick));
	mir_free(const_cast<TCHAR*>(gce.ptszText));
	mir_free(const_cast<TCHAR*>(gcd.ptszID));

	
	// Close chat window, if set
	ForkThread( &FacebookProto::MessagingWorker, this, new send_messaging(chat_id, FACEBOOK_SEND_MESSAGE ) );
}

int FacebookProto::OnChatOutgoing(WPARAM wParam,LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);
	char *text;
	char *id;

	if (strcmp(hook->pDest->pszModule,m_szModuleName))
		return 0;

	switch(hook->pDest->iType)
	{
	case GC_USER_MESSAGE:
	{
		text = mir_t2a_cp(hook->ptszText,CP_UTF8);
		std::string msg = text;

		id = mir_t2a_cp(hook->pDest->ptszID,CP_UTF8);
		std::string chat_id = id;

		mir_free(text);
		mir_free(id);
	
		if (isOnline()) {
			LOG("**Chat - Outgoing message: %s", text);
			ForkThread(&FacebookProto::SendChatMsgWorker, this, new send_chat(chat_id, msg) );
		}
	
		break;
	}

	case GC_USER_LEAVE:
	case GC_SESSION_TERMINATE:
	{
		break;
	}
	}

	return 0;
}

void FacebookProto::AddChatContact(const char *chat_id, const char *id, const char *name)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = mir_a2t(chat_id);
	gcd.iType  = GC_EVENT_JOIN;

	GCEVENT gce    = {sizeof(gce)};
	gce.pDest      = &gcd;
	gce.dwFlags    = GC_TCHAR | GCEF_ADDTOLOG;
	gce.ptszNick   = mir_a2t_cp(name, CP_UTF8);
	gce.ptszUID    = mir_a2t(id);
	gce.time       = ::time(NULL);
	gce.bIsMe      = !strcmp(id, facy.self_.user_id.c_str());

	if (gce.bIsMe)
		gce.ptszStatus = _T("Admin");
	else
		gce.ptszStatus = _T("Normal");

	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));

	mir_free(const_cast<TCHAR*>(gce.ptszNick));
	mir_free(const_cast<TCHAR*>(gce.ptszUID));
	mir_free(const_cast<TCHAR*>(gcd.ptszID));
}


void FacebookProto::RemoveChatContact(const char *chat_id, const char *id)
{
	// We dont want to remove our self-contact from chat. Ever.
	if (!strcmp(id, facy.self_.user_id.c_str()))
		return;
	
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = mir_a2t(chat_id);
	gcd.iType  = GC_EVENT_PART;

	GCEVENT gce    = {sizeof(gce)};
	gce.pDest      = &gcd;
	gce.dwFlags    = GC_TCHAR | GCEF_ADDTOLOG;
	//gce.ptszNick   = mir_a2t_cp(name, CP_UTF8);
	gce.ptszUID    = mir_a2t(id);
	gce.ptszNick   = gce.ptszUID;
	gce.time       = ::time(NULL);
	gce.bIsMe      = false;//!strcmp(id, facy.self_.user_id.c_str());

	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));

	mir_free(const_cast<TCHAR*>(gcd.ptszID));
	mir_free(const_cast<TCHAR*>(gce.ptszNick));
	mir_free(const_cast<TCHAR*>(gce.ptszUID));	
}

char *FacebookProto::GetChatUsers(const char *chat_id)
{
	GC_INFO gci = {0};
	gci.Flags = USERS;
	gci.pszModule = m_szModuleName;
	gci.pszID = mir_a2t(chat_id);
	CallService(MS_GC_GETINFO, 0, (LPARAM)(GC_INFO *) &gci);

	LOG("**Chat - Users in chat %s: %s", chat_id, gci.pszUsers);

	mir_free(gci.pszID);

	// mir_free(gci.pszUsers);
	return gci.pszUsers;
}

bool FacebookProto::IsChatContact(const char *chat_id, const char *id)
{
	char *users = GetChatUsers(chat_id);
	bool found = false;

	if (users != NULL && strstr(users, id) != NULL)
		found = true;

	mir_free(users);
	return found;
}

void FacebookProto::AddChat(const char *id, const char *name)
{
	GCSESSION gcw = {sizeof(gcw)};

	// Create the group chat session
	gcw.dwFlags   = GC_TCHAR;
	gcw.iType     = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName  = mir_a2t_cp(name, CP_UTF8);
	gcw.ptszID    = mir_a2t(id);
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	mir_free(const_cast<TCHAR*>(gcw.ptszName));
	mir_free(const_cast<TCHAR*>(gcw.ptszID));

	// Send setting events
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = mir_a2t(id);

	GCEVENT gce = {sizeof(gce)};
	gce.pDest = &gcd;
	gce.dwFlags = GC_TCHAR;

	// Create a user statuses
	gcd.iType = GC_EVENT_ADDGROUP;
	gce.ptszStatus = _T("Admin");
	CallServiceSync( MS_GC_EVENT, NULL, reinterpret_cast<LPARAM>(&gce) );
	gce.ptszStatus = _T("Normal");
	CallServiceSync( MS_GC_EVENT, NULL, reinterpret_cast<LPARAM>(&gce) );
	
	// Finish initialization
	gcd.iType = GC_EVENT_CONTROL;
	gce.time = ::time(NULL);
	gce.pDest = &gcd;

	// Add self contact
	AddChatContact(id, facy.self_.user_id.c_str(), facy.self_.real_name.c_str());
	CallServiceSync(MS_GC_EVENT,SESSION_INITDONE,reinterpret_cast<LPARAM>(&gce));
	CallServiceSync(MS_GC_EVENT,SESSION_ONLINE,  reinterpret_cast<LPARAM>(&gce));

	mir_free(const_cast<TCHAR*>(gcd.ptszID));
}

/*void FacebookProto::SetTopic(const char *topic)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);
	gcd.iType = GC_EVENT_TOPIC;

	GCEVENT gce = {sizeof(gce)};
	gce.pDest = &gcd;
	gce.dwFlags = GC_TCHAR;
	gce.time = ::time(NULL);
	
	std::string top = Translate(topic);
	gce.ptszText = mir_a2t(top.c_str());
	CallServiceSync(MS_GC_EVENT,0,  reinterpret_cast<LPARAM>(&gce));
}
*/

int FacebookProto::OnJoinChat(WPARAM,LPARAM suppress)
{	
/*	GCSESSION gcw = {sizeof(gcw)};

	// Create the group chat session
	gcw.dwFlags   = GC_TCHAR;
	gcw.iType     = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName  = m_tszUserName;
	gcw.ptszID    = m_tszUserName;
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	if(m_iStatus != ID_STATUS_ONLINE)
		return 0;

	// Create a group
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);

	GCEVENT gce = {sizeof(gce)};
	gce.pDest = &gcd;
	gce.dwFlags = GC_TCHAR;

	gcd.iType = GC_EVENT_ADDGROUP;

	gce.ptszStatus = _T("Admin");
	CallServiceSync( MS_GC_EVENT, NULL, reinterpret_cast<LPARAM>(&gce) );
	
	gce.ptszStatus = _T("Normal");
	CallServiceSync( MS_GC_EVENT, NULL, reinterpret_cast<LPARAM>(&gce) );

	SetTopic("Omegle is a great way of meeting new friends!");

	// Note: Initialization will finish up in SetChatStatus, called separately
	if(!suppress)
		SetChatStatus(m_iStatus);
*/
	return 0;
}

int FacebookProto::OnLeaveChat(WPARAM,LPARAM)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = NULL;
	gcd.iType = GC_EVENT_CONTROL;

	GCEVENT gce = {sizeof(gce)};
	gce.dwFlags = GC_TCHAR;
	gce.time = ::time(NULL);
	gce.pDest = &gcd;

	CallServiceSync(MS_GC_EVENT,SESSION_OFFLINE,  reinterpret_cast<LPARAM>(&gce));
	CallServiceSync(MS_GC_EVENT,SESSION_TERMINATE,reinterpret_cast<LPARAM>(&gce));

	return 0;
}

/*
void FacebookProto::SetChatStatus(int status)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);
	gcd.iType = GC_EVENT_CONTROL;

	GCEVENT gce = {sizeof(gce)};
	gce.dwFlags = GC_TCHAR;
	gce.time = ::time(NULL);
	gce.pDest = &gcd;

	if(status == ID_STATUS_ONLINE)
	{
		// Add self contact
		AddChatContact(facy.nick_.c_str());

		CallServiceSync(MS_GC_EVENT,SESSION_INITDONE,reinterpret_cast<LPARAM>(&gce));
		CallServiceSync(MS_GC_EVENT,SESSION_ONLINE,  reinterpret_cast<LPARAM>(&gce));
	}
	else
	{
		CallServiceSync(MS_GC_EVENT,SESSION_OFFLINE,reinterpret_cast<LPARAM>(&gce));
	}
}
*/