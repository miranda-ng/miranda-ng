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

#include "common.h"
#include "proto.h"


void TwitterProto::AddToListWorker(void *p)
{
	// TODO: what happens if there is an error?
	if (p == 0)
		return;
	TCHAR *name = static_cast<TCHAR*>(p);

	try
	{
		ScopedLock s(twitter_lock_);
		twitter_user user = twit_.add_friend(name);
		s.Unlock();

		HANDLE hContact = UsernameToHContact(name);
		UpdateAvatar(hContact, user.profile_image_url);
	}
	catch(const std::exception &e)
	{
		ShowPopup((std::string("While adding a friend, an error occurred: ")
			+e.what()).c_str());
		LOG("***** Error adding friend: %s", e.what());
	}
	mir_free(name);
}

HANDLE TwitterProto::AddToList(int flags, PROTOSEARCHRESULT *result)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 0;

	ForkThread(&TwitterProto::AddToListWorker, this, mir_tstrdup(result->nick));
	return AddToClientList(result->nick, _T(""));
}

// *************************

void TwitterProto::UpdateInfoWorker(void *hContact)
{
	twitter_user user;
	std::tstring username;
	
	DBVARIANT dbv;
	if ( !DBGetContactSettingTString(hContact, m_szModuleName, TWITTER_KEY_UN, &dbv))
	{
		username = dbv.ptszVal;
		DBFreeVariant(&dbv);
	}
	else
		return;

	{
		ScopedLock s(twitter_lock_);
		twit_.get_info(username, &user);
	}

	UpdateAvatar(hContact, user.profile_image_url, true);
	ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, 0, 0);
}

int TwitterProto::GetInfo(HANDLE hContact, int info_type)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 1;

	if (!IsMyContact(hContact)) // Do nothing for chat rooms
		return 1;

	if (info_type == 0) // From clicking "Update" in the Userinfo dialog
	{
		ForkThread(&TwitterProto::UpdateInfoWorker, this, hContact);
		return 0;
	}

	return 1;
}

// *************************

struct search_query
{
	search_query(const std::tstring &query, bool by_email) : query(query), by_email(by_email)
	{}
	std::tstring query;
	bool by_email;
};

void TwitterProto::DoSearch(void *p)
{
	if (p == 0)
		return;
	search_query *query = static_cast<search_query*>(p);

	twitter_user info;
	PROTOSEARCHRESULT psr = {sizeof(psr)};

	bool found;
	try
	{
		ScopedLock s(twitter_lock_);
		if (query->by_email)
			found = twit_.get_info_by_email(query->query, &info);
		else
			found = twit_.get_info(query->query, &info);
	}
	catch(const std::exception &e)
	{
		ShowPopup( (std::string("While searching for contacts, an error occurred: ")
			+e.what()).c_str());
		LOG("***** Error searching for contacts: %s", e.what());
	}

	if (found)
	{
		psr.nick      = ( TCHAR* )info.username. c_str();
		psr.firstName = ( TCHAR* )info.real_name.c_str();

		ProtoBroadcastAck(m_szModuleName, 0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, 
			(LPARAM)&psr);
		ProtoBroadcastAck(m_szModuleName, 0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	}
	else
	{
		ProtoBroadcastAck(m_szModuleName, 0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	}

	delete query;
}

HANDLE TwitterProto::SearchBasic(const TCHAR *username)
{
	ForkThread(&TwitterProto::DoSearch, this, new search_query(username, false));
	return (HANDLE)1;
}

HANDLE TwitterProto::SearchByEmail(const TCHAR *email)
{
	ForkThread(&TwitterProto::DoSearch, this, new search_query(email, true));
	return (HANDLE)1;
}

// *************************

void TwitterProto::GetAwayMsgWorker(void *hContact)
{
	if (hContact == 0)
		return;

	DBVARIANT dbv;
	if ( !DBGetContactSettingTString(hContact, "CList", "StatusMsg", &dbv)) {
		ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)dbv.ptszVal);
		DBFreeVariant(&dbv);
	}
	else ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_AWAYMSG, ACKRESULT_FAILED, (HANDLE)1, 0);
}

HANDLE TwitterProto::GetAwayMsg(HANDLE hContact)
{
	ForkThread(&TwitterProto::GetAwayMsgWorker, this, hContact);
	return (HANDLE)1;
}

int TwitterProto::OnContactDeleted(WPARAM wParam, LPARAM lParam)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 0;

	const HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	if (!IsMyContact(hContact))
		return 0;

	DBVARIANT dbv;
	if ( !DBGetContactSettingTString(hContact, m_szModuleName, TWITTER_KEY_UN, &dbv)) {
		if ( in_chat_ )
			DeleteChatContact(dbv.ptszVal);

		ScopedLock s(twitter_lock_);
		twit_.remove_friend(dbv.ptszVal); // Be careful about this until Miranda is fixed
		DBFreeVariant(&dbv);
	}
	return 0;
}

// *************************

bool TwitterProto::IsMyContact(HANDLE hContact, bool include_chat)
{
	const char *proto = reinterpret_cast<char*>( CallService(MS_PROTO_GETCONTACTBASEPROTO, 
		reinterpret_cast<WPARAM>(hContact), 0));

	if (proto && strcmp(m_szModuleName, proto) == 0)
	{
		if (include_chat)
			return true;
		else
			return DBGetContactSettingByte(hContact, m_szModuleName, "ChatRoom", 0) == 0;
	}
	else
		return false;
}

HANDLE TwitterProto::UsernameToHContact(const TCHAR *name)
{
	for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		hContact;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
	{
		if (!IsMyContact(hContact))
			continue;

		DBVARIANT dbv;
		if ( !DBGetContactSettingTString(hContact, m_szModuleName, TWITTER_KEY_UN, &dbv)) {
			if ( lstrcmp(name, dbv.ptszVal) == 0) {
				DBFreeVariant(&dbv);
				return hContact;
			}
			DBFreeVariant(&dbv);
		}
	}

	return 0;
}

HANDLE TwitterProto::AddToClientList(const TCHAR *name, const TCHAR *status)
{
	// First, check if this contact exists
	HANDLE hContact = UsernameToHContact(name);
	if (hContact)
		return hContact;

	if (in_chat_)
		AddChatContact(name);

	// If not, make a new contact!
	hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if (hContact ) {
		if (CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName) == 0) {
			DBWriteContactSettingTString(hContact, m_szModuleName, TWITTER_KEY_UN, name);
			DBWriteContactSettingWord(hContact, m_szModuleName, "Status", ID_STATUS_ONLINE);
			DBWriteContactSettingTString(hContact, "CList", "StatusMsg", status);

			std::string url = profile_base_url(twit_.get_base_url()) + http::url_encode((char*)_T2A(name));
			DBWriteContactSettingString(hContact, m_szModuleName, "Homepage", url.c_str());

			return hContact;
		}

		CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
	}

	return 0;
}

void TwitterProto::SetAllContactStatuses(int status)
{
	for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		hContact;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
	{
		if (!IsMyContact(hContact))
			continue;

		DBWriteContactSettingWord(hContact, m_szModuleName, "Status", status);
	}

	SetChatStatus(status);
}