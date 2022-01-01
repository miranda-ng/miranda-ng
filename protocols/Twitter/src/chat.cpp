/*
Copyright © 2012-22 Miranda NG team
Copyright © 2009 Jim Porter

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

#include <set>
#include <ctime>

void CTwitterProto::UpdateChat(const twitter_user &update)
{
	CMStringA chatText = update.status.text.c_str();
	chatText.Replace("%", "%%");

	GCEVENT gce = { m_szModuleName, m_szChatId, GC_EVENT_MESSAGE };
	gce.dwFlags = GCEF_UTF8 + GCEF_ADDTOLOG;
	gce.bIsMe = (update.username.c_str() == m_szUserName);
	gce.pszUID.a = update.username.c_str();
	gce.pszText.a = chatText.c_str();
	gce.time = (uint32_t)update.status.time;

	MCONTACT hContact = UsernameToHContact(update.username.c_str());
	ptrA szNick(db_get_utfa(hContact, "CList", "MyHandle"));
	if (hContact && szNick)
		gce.pszNick.a = szNick;
	else
		gce.pszNick.a = update.username.c_str();

	Chat_Event(&gce);
}

int CTwitterProto::OnChatOutgoing(WPARAM, LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);
	if (mir_strcmp(hook->si->pszModule, m_szModuleName))
		return 0;

	switch (hook->iType) {
	case GC_USER_MESSAGE:
		debugLogW(L"**Chat - Outgoing message: %s", hook->ptszText);
		{
			CMStringA tweet(T2Utf(hook->ptszText).get());
			tweet.Replace("%%", "%"); // the chat plugin will turn "%" into "%%", so we have to change it back :/

			char *varTweet = mir_strdup(tweet.c_str());
			ForkThread(&CTwitterProto::SendTweetWorker, varTweet);
		}
		break;

	case GC_USER_PRIVMESS:
		{
			ptrA text(mir_u2a(hook->ptszUID));
			CallService(MS_MSG_SENDMESSAGE, UsernameToHContact(text), 0);
		}
		break;
	}

	return 1;
}

// TODO: remove nick?
void CTwitterProto::AddChatContact(const char *name, const char *nick)
{
	GCEVENT gce = { m_szModuleName, m_szChatId, GC_EVENT_JOIN };
	gce.dwFlags = GCEF_UTF8;
	gce.time = uint32_t(time(0));
	gce.pszNick.a = nick ? nick : name;
	gce.pszUID.a = name;
	gce.pszStatus.a = "Normal";
	Chat_Event(&gce);
}

void CTwitterProto::DeleteChatContact(const char *name)
{
	GCEVENT gce = { m_szModuleName, m_szChatId, GC_EVENT_PART };
	gce.dwFlags = GCEF_UTF8;
	gce.time = uint32_t(time(0));
	gce.pszUID.a = gce.pszNick.a = name;
	Chat_Event(&gce);
}

INT_PTR CTwitterProto::OnJoinChat(WPARAM, LPARAM suppress)
{
	// ***** Create the group chat session
	SESSION_INFO *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, m_tszUserName, m_tszUserName);
	if (!si || m_iStatus != ID_STATUS_ONLINE)
		return 0;

	// ***** Create a group
	Chat_AddGroup(si, TranslateT("Normal"));

	// ***** Hook events
	HookProtoEvent(ME_GC_EVENT, &CTwitterProto::OnChatOutgoing);

	// Note: Initialization will finish up in SetChatStatus, called separately
	if (!suppress)
		SetChatStatus(m_iStatus);

	in_chat_ = true;
	return 0;
}

INT_PTR CTwitterProto::OnLeaveChat(WPARAM, LPARAM)
{
	in_chat_ = false;

	Chat_Control(m_szModuleName, m_tszUserName, SESSION_OFFLINE);
	Chat_Terminate(m_szModuleName, m_tszUserName);
	return 0;
}

void CTwitterProto::SetChatStatus(int status)
{
	if (status == ID_STATUS_ONLINE) {
		// Add all friends to contact list
		for (auto &hContact : AccContacts()) {
			if (isChatRoom(hContact))
				continue;

			ptrA uid(getUStringA(hContact, TWITTER_KEY_UN)), nick(db_get_utfa(hContact, "CList", "MyHandle"));
			if (uid)
				AddChatContact(uid, nick);
		}

		// For some reason, I have to send an INITDONE message, even if I'm not actually
		// initializing the room...
		Chat_Control(m_szModuleName, m_tszUserName, SESSION_INITDONE);
		Chat_Control(m_szModuleName, m_tszUserName, SESSION_ONLINE);
	}
	else Chat_Control(m_szModuleName, m_tszUserName, SESSION_OFFLINE);
}
