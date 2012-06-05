/*
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation,  either version 2 of the License,  or
(at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not,  see <http://www.gnu.org/licenses/>.
*/

#include "common.h"
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
	gce.ptszUID  = mir_a2t( update.username.c_str());
	gce.bIsMe    = (update.username == twit_.get_username());
	gce.ptszText = update.status.text.c_str();
	gce.time     = static_cast<DWORD>(update.status.time);

	DBVARIANT nick;
	HANDLE hContact = UsernameToHContact( _A2T(update.username.c_str()));
	if (hContact && !DBGetContactSettingTString(hContact, "CList", "MyHandle", &nick)) {
		gce.ptszNick = mir_tstrdup( nick.ptszVal );
		DBFreeVariant(&nick);
	}
	else gce.ptszNick = mir_a2t( update.username.c_str());

	CallServiceSync(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));

	mir_free(const_cast<TCHAR*>(gce.ptszNick));
	mir_free(const_cast<TCHAR*>(gce.ptszUID));
	mir_free(const_cast<TCHAR*>(gce.ptszText));
}

int TwitterProto::OnChatOutgoing(WPARAM wParam, LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);
	if (strcmp(hook->pDest->pszModule, m_szModuleName))
		return 0;

	TCHAR *text;
	switch(hook->pDest->iType) {
	case GC_USER_MESSAGE:
		text = mir_tstrdup(hook->ptszText);
		ForkThread(&TwitterProto::SendTweetWorker, this, text);
		break;
	
	case GC_USER_PRIVMESS:
		text = mir_tstrdup(hook->ptszUID);
		CallService(MS_MSG_SENDMESSAGE, reinterpret_cast<WPARAM>( UsernameToHContact(text)), 0);
		mir_free(text);
		break;
	}

	return 0;
}

// TODO: remove nick?
void TwitterProto::AddChatContact(const TCHAR *name, const TCHAR *nick)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);
	gcd.iType  = GC_EVENT_JOIN;

	GCEVENT gce    = {sizeof(gce)};
	gce.pDest      = &gcd;
	gce.dwFlags    = GC_TCHAR;
	gce.ptszNick   = nick ? nick:name;
	gce.ptszUID    = name;
	gce.bIsMe      = false;
	gce.ptszStatus = _T("Normal");
	gce.time       = static_cast<DWORD>(time(0));
	CallServiceSync(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));
}

void TwitterProto::DeleteChatContact(const TCHAR *name)
{
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);
	gcd.iType  = GC_EVENT_PART;

	GCEVENT gce    = {sizeof(gce)};
	gce.pDest      = &gcd;
	gce.dwFlags    = GC_TCHAR;
	gce.ptszNick   = name;
	gce.ptszUID    = gce.ptszNick;
	gce.time       = static_cast<DWORD>(time(0));
	CallServiceSync(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));

	mir_free(const_cast<TCHAR*>(gce.ptszNick));
}

int TwitterProto::OnJoinChat(WPARAM, LPARAM suppress)
{
	GCSESSION gcw = {sizeof(gcw)};

	// ***** Create the group chat session
	gcw.dwFlags   = GC_TCHAR;
	gcw.iType     = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName  = m_tszUserName;
	gcw.ptszID    = m_tszUserName;
	CallServiceSync(MS_GC_NEWSESSION,  0,  (LPARAM)&gcw);

	if (m_iStatus != ID_STATUS_ONLINE)
		return 0;

	// ***** Create a group
	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);

	GCEVENT gce = {sizeof(gce)};
	gce.pDest = &gcd;
	gce.dwFlags = GC_TCHAR;

	gcd.iType = GC_EVENT_ADDGROUP;
	gce.ptszStatus = _T("Normal");
	CallServiceSync(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));

	// ***** Hook events
	HookProtoEvent(ME_GC_EVENT, &TwitterProto::OnChatOutgoing, this);

	// Note: Initialization will finish up in SetChatStatus,  called separately
	if (!suppress)
		SetChatStatus(m_iStatus);

	in_chat_ = true;
	return 0;
}

int TwitterProto::OnLeaveChat(WPARAM, LPARAM)
{
	in_chat_ = false;

	GCDEST gcd = { m_szModuleName };
	gcd.ptszID = const_cast<TCHAR*>(m_tszUserName);
	gcd.iType = GC_EVENT_CONTROL;

	GCEVENT gce = {sizeof(gce)};
	gce.dwFlags = GC_TCHAR;
	gce.pDest = &gcd;

	CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE,   reinterpret_cast<LPARAM>(&gce));
	CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, reinterpret_cast<LPARAM>(&gce));

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

	if (status == ID_STATUS_ONLINE) {
		// Add all friends to contact list
		for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
			hContact;
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
		{
			if (!IsMyContact(hContact))
				continue;

			DBVARIANT uid, nick;
			if ( DBGetContactSettingTString(hContact, m_szModuleName, TWITTER_KEY_UN, &uid))
				continue;

			if ( !DBGetContactSettingTString(hContact, "CList", "MyHandle", &nick))
				AddChatContact(uid.ptszVal, nick.ptszVal);
			else
				AddChatContact(uid.ptszVal);

			DBFreeVariant(&nick);
			DBFreeVariant(&uid);
		}

		// For some reason,  I have to send an INITDONE message,  even if I'm not actually
		// initializing the room...
		CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, reinterpret_cast<LPARAM>(&gce));
		CallServiceSync(MS_GC_EVENT, SESSION_ONLINE,   reinterpret_cast<LPARAM>(&gce));
	}
	else CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, reinterpret_cast<LPARAM>(&gce));
}