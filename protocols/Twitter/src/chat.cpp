/*
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

#include "proto.h"

#include <set>
#include <ctime>

void TwitterProto::UpdateChat(const twitter_user &update)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);
	gcd.iType  = GC_EVENT_MESSAGE;

	GCEVENT gce  = {sizeof(gce)};
	gce.pDest    = &gcd;
	gce.dwFlags  = GC_TCHAR|GCEF_ADDTOLOG;
	gce.pDest    = &gcd;
	gce.ptszUID  = mir_a2t(update.username.c_str());
	gce.bIsMe    = (update.username == twit_.get_username());
	//TODO: write code here to replace % with %% in update.status.text (which is a std::string)

	std::string chatText = update.status.text;

	replaceAll(chatText, "%", "%%");

	gce.ptszText = mir_a2t_cp(chatText.c_str(),CP_UTF8);
	//gce.ptszText = mir_a2t_cp(update.status.text.c_str(),CP_UTF8);
	gce.time     = static_cast<DWORD>(update.status.time);

	DBVARIANT nick;
	HANDLE hContact = UsernameToHContact(update.username.c_str());
	if(hContact && !db_get_s(hContact,"CList","MyHandle",&nick))
	{
		gce.ptszNick = mir_a2t(nick.pszVal);
		db_free(&nick);
	}
	else
		gce.ptszNick = mir_a2t(update.username.c_str());

	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));

	mir_free(const_cast<TCHAR*>(gce.ptszNick));
	mir_free(const_cast<TCHAR*>(gce.ptszUID));
	mir_free(const_cast<TCHAR*>(gce.ptszText));
}

int TwitterProto::OnChatOutgoing(WPARAM wParam,LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);
	if ( strcmp(hook->pDest->pszModule, m_szModuleName))
		return 0;

	switch(hook->pDest->iType) {
	case GC_USER_MESSAGE:
		debugLog( _T("**Chat - Outgoing message: %s"), hook->ptszText);
		{
			ptrA text( mir_utf8encodeT(hook->ptszText));

			std::string tweet(text);
			replaceAll(tweet, "%%", "%"); // the chat plugin will turn "%" into "%%", so we have to change it back :/

			char *varTweet = mir_strdup( tweet.c_str());
			ForkThread(&TwitterProto::SendTweetWorker, varTweet);
		}
		break;

	case GC_USER_PRIVMESS:
		{
			ptrA text( mir_t2a(hook->ptszUID));
			CallService(MS_MSG_SENDMESSAGE, WPARAM(UsernameToHContact(text)), 0);
		}
		break;
	}

	return 0;
}

// TODO: remove nick?
void TwitterProto::AddChatContact(const char *name,const char *nick)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);
	gcd.iType  = GC_EVENT_JOIN;

	GCEVENT gce    = {sizeof(gce)};
	gce.pDest      = &gcd;
	gce.dwFlags    = GC_TCHAR;
	gce.ptszNick   = mir_a2t(nick ? nick:name);
	gce.ptszUID    = mir_a2t(name);
	gce.bIsMe      = false;
	gce.ptszStatus = _T("Normal");
	gce.time       = static_cast<DWORD>(time(0));
	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));

	mir_free(const_cast<TCHAR*>(gce.ptszNick));
	mir_free(const_cast<TCHAR*>(gce.ptszUID));
}

void TwitterProto::DeleteChatContact(const char *name)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);
	gcd.iType  = GC_EVENT_PART;

	GCEVENT gce    = {sizeof(gce)};
	gce.pDest      = &gcd;
	gce.dwFlags    = GC_TCHAR;
	gce.ptszNick   = mir_a2t(name);
	gce.ptszUID    = gce.ptszNick;
	gce.time       = static_cast<DWORD>(time(0));
	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));

	mir_free(const_cast<TCHAR*>(gce.ptszNick));
}

INT_PTR TwitterProto::OnJoinChat(WPARAM,LPARAM suppress)
{
	GCSESSION gcw = {sizeof(gcw)};

	// ***** Create the group chat session
	gcw.dwFlags   = GC_TCHAR;
	gcw.iType     = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName  = m_tszUserName;
	gcw.ptszID    = m_tszUserName;
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	if(m_iStatus != ID_STATUS_ONLINE)
		return 0;

	// ***** Create a group
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);

	GCEVENT gce = {sizeof(gce)};
	gce.pDest = &gcd;
	gce.dwFlags = GC_TCHAR;

	gcd.iType = GC_EVENT_ADDGROUP;
	gce.ptszStatus = _T("Normal");
	CallServiceSync(MS_GC_EVENT,0,reinterpret_cast<LPARAM>(&gce));

	// ***** Hook events
	HookProtoEvent(ME_GC_EVENT, &TwitterProto::OnChatOutgoing);

	// Note: Initialization will finish up in SetChatStatus, called separately
	if(!suppress)
		SetChatStatus(m_iStatus);

	in_chat_ = true;
	return 0;
}

INT_PTR TwitterProto::OnLeaveChat(WPARAM,LPARAM)
{
	in_chat_ = false;

	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);
	gcd.iType = GC_EVENT_CONTROL;

	GCEVENT gce = {sizeof(gce)};
	gce.dwFlags = GC_TCHAR;
	gce.pDest = &gcd;

	CallServiceSync(MS_GC_EVENT,SESSION_OFFLINE,  reinterpret_cast<LPARAM>(&gce));
	CallServiceSync(MS_GC_EVENT,SESSION_TERMINATE,reinterpret_cast<LPARAM>(&gce));

	return 0;
}

void TwitterProto::SetChatStatus(int status)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);
	gcd.iType = GC_EVENT_CONTROL;

	GCEVENT gce = {sizeof(gce)};
	gce.dwFlags = GC_TCHAR;
	gce.pDest = &gcd;

	if(status == ID_STATUS_ONLINE)
	{
		// Add all friends to contact list
		for(HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			if( isChatRoom(hContact))
				continue;

			DBVARIANT uid,nick;
			if(db_get_s(hContact,m_szModuleName,TWITTER_KEY_UN,&uid))
				continue;

			if(!db_get_s(hContact,"CList","MyHandle",&nick))
			{
				AddChatContact(uid.pszVal,nick.pszVal);
				db_free(&nick);
			}
			else
				AddChatContact(uid.pszVal);

			db_free(&uid);
		}

		// For some reason, I have to send an INITDONE message, even if I'm not actually
		// initializing the room...
		CallServiceSync(MS_GC_EVENT,SESSION_INITDONE,reinterpret_cast<LPARAM>(&gce));
		CallServiceSync(MS_GC_EVENT,SESSION_ONLINE,  reinterpret_cast<LPARAM>(&gce));
	}
	else
		CallServiceSync(MS_GC_EVENT,SESSION_OFFLINE,reinterpret_cast<LPARAM>(&gce));
}