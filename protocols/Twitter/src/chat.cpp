/*
Copyright © 2012-20 Miranda NG team
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
#include "proto.h"

#include <set>
#include <ctime>

void TwitterProto::UpdateChat(const twitter_user &update)
{
	GCEVENT gce = { m_szModuleName, m_szChatId, GC_EVENT_MESSAGE };
	gce.dwFlags = GCEF_UTF8 + GCEF_ADDTOLOG;
	gce.bIsMe = (update.username == twit_.get_username());
	gce.pszUID.a = update.username.c_str();
	//TODO: write code here to replace % with %% in update.status.text (which is a std::string)

	std::string chatText = update.status.text;

	replaceAll(chatText, "%", "%%");

	gce.pszText.a = chatText.c_str();
	gce.time = static_cast<DWORD>(update.status.time);

	MCONTACT hContact = UsernameToHContact(update.username.c_str());
	CMStringA szNick = db_get_sm(hContact, "CList", "MyHandle");
	if (hContact && !szNick.IsEmpty())
		gce.pszNick.a = szNick;
	else
		gce.pszNick.a = update.username.c_str();

	Chat_Event(&gce);

	mir_free(const_cast<wchar_t*>(gce.pszNick.w));
	mir_free(const_cast<wchar_t*>(gce.pszUID.w));
	mir_free(const_cast<wchar_t*>(gce.pszText.w));
}

int TwitterProto::OnChatOutgoing(WPARAM, LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);
	if (mir_strcmp(hook->si->pszModule, m_szModuleName))
		return 0;

	switch (hook->iType) {
	case GC_USER_MESSAGE:
		debugLogW(L"**Chat - Outgoing message: %s", hook->ptszText);
		{
			T2Utf text(hook->ptszText);

			std::string tweet(text);
			replaceAll(tweet, "%%", "%"); // the chat plugin will turn "%" into "%%", so we have to change it back :/

			char *varTweet = mir_strdup(tweet.c_str());
			ForkThread(&TwitterProto::SendTweetWorker, varTweet);
		}
		break;

	case GC_USER_PRIVMESS:
	{
		ptrA text(mir_u2a(hook->ptszUID));
		CallService(MS_MSG_SENDMESSAGE, WPARAM(UsernameToHContact(text)), 0);
	}
	break;
	}

	return 0;
}

// TODO: remove nick?
void TwitterProto::AddChatContact(const char *name, const char *nick)
{
	GCEVENT gce = { m_szModuleName, m_szChatId, GC_EVENT_JOIN };
	gce.dwFlags = GCEF_UTF8;
	gce.time = DWORD(time(0));
	gce.pszNick.a = nick ? nick : name;
	gce.pszUID.a = name;
	gce.pszStatus.a = "Normal";
	Chat_Event(&gce);
}

void TwitterProto::DeleteChatContact(const char *name)
{
	GCEVENT gce = { m_szModuleName, m_szChatId, GC_EVENT_PART };
	gce.dwFlags = GCEF_UTF8;
	gce.time = DWORD(time(0));
	gce.pszUID.a = gce.pszNick.a = name;
	Chat_Event(&gce);
}

INT_PTR TwitterProto::OnJoinChat(WPARAM, LPARAM suppress)
{
	// ***** Create the group chat session
	SESSION_INFO *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, m_tszUserName, m_tszUserName);
	if (!si || m_iStatus != ID_STATUS_ONLINE)
		return 0;

	// ***** Create a group
	Chat_AddGroup(si, TranslateT("Normal"));

	// ***** Hook events
	HookProtoEvent(ME_GC_EVENT, &TwitterProto::OnChatOutgoing);

	// Note: Initialization will finish up in SetChatStatus, called separately
	if (!suppress)
		SetChatStatus(m_iStatus);

	in_chat_ = true;
	return 0;
}

INT_PTR TwitterProto::OnLeaveChat(WPARAM, LPARAM)
{
	in_chat_ = false;

	Chat_Control(m_szModuleName, m_tszUserName, SESSION_OFFLINE);
	Chat_Terminate(m_szModuleName, m_tszUserName);
	return 0;
}

void TwitterProto::SetChatStatus(int status)
{
	if (status == ID_STATUS_ONLINE) {
		// Add all friends to contact list
		for (auto &hContact : AccContacts()) {
			if (isChatRoom(hContact))
				continue;

			DBVARIANT uid, nick;
			if (getString(hContact, TWITTER_KEY_UN, &uid))
				continue;

			if (!db_get_s(hContact, "CList", "MyHandle", &nick)) {
				AddChatContact(uid.pszVal, nick.pszVal);
				db_free(&nick);
			}
			else
				AddChatContact(uid.pszVal);

			db_free(&uid);
		}

		// For some reason, I have to send an INITDONE message, even if I'm not actually
		// initializing the room...
		Chat_Control(m_szModuleName, m_tszUserName, SESSION_INITDONE);
		Chat_Control(m_szModuleName, m_tszUserName, SESSION_ONLINE);
	}
	else Chat_Control(m_szModuleName, m_tszUserName, SESSION_OFFLINE);
}
