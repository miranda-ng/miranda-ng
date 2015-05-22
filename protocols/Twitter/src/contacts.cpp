/*
Copyright © 2012-15 Miranda NG team
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

void TwitterProto::AddToListWorker(void *p)
{
	// TODO: what happens if there is an error?
	if (p == 0)
		return;

	char *name = static_cast<char*>(p);

	try {
		twitter_user user;
		{
			mir_cslock s(twitter_lock_);
			user = twit_.add_friend(name);
		}

		MCONTACT hContact = UsernameToHContact(name);
		UpdateAvatar(hContact, user.profile_image_url);
	}
	catch (const std::exception &e) {
		ShowPopup((std::string("While adding a friend, an error occurred: ") + e.what()).c_str());
		debugLogA("***** Error adding friend: %s", e.what());
	}
	mir_free(name);
}

MCONTACT TwitterProto::AddToList(int, PROTOSEARCHRESULT *result)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 0;

	ForkThread(&TwitterProto::AddToListWorker, mir_utf8encodeT(result->nick));
	return AddToClientList(_T2A(result->nick), "");
}

// *************************

void TwitterProto::UpdateInfoWorker(void *hContact)
{
	twitter_user user;

	ptrA username(db_get_sa((MCONTACT)hContact, m_szModuleName, TWITTER_KEY_UN));
	if (username == NULL)
		return;

	{	mir_cslock s(twitter_lock_);
		twit_.get_info(std::string(username), &user);
	}

	UpdateAvatar((MCONTACT)hContact, user.profile_image_url, true);
	ProtoBroadcastAck((MCONTACT)hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, 0, 0);
}

int TwitterProto::GetInfo(MCONTACT hContact, int info_type)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 1;

	if (!IsMyContact(hContact)) // Do nothing for chat rooms
		return 1;

	if (info_type == 0) { // From clicking "Update" in the Userinfo dialog
		ForkThread(&TwitterProto::UpdateInfoWorker, (void*)hContact);
		return 0;
	}

	return 1;
}

// *************************

struct search_query
{
	search_query(const std::tstring &_query, bool _by_email) : query(_query), by_email(_by_email)
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

	bool found = false;
	try {
		T2Utf p(query->query.c_str());

		mir_cslock s(twitter_lock_);
		if (query->by_email)
			found = twit_.get_info_by_email(p.str(), &info);
		else
			found = twit_.get_info(p.str(), &info);
	}
	catch (const std::exception &e) {
		ShowPopup((std::string("While searching for contacts, an error occurred: ") + e.what()).c_str());
		debugLogA("***** Error searching for contacts: %s", e.what());
		found = false;
	}

	if (found) {
		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_TCHAR;
		psr.nick = mir_a2t(info.username.c_str());
		psr.firstName = mir_a2t(info.real_name.c_str());

		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);

		mir_free(psr.nick);
		mir_free(psr.firstName);
	}
	else ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);

	delete query;
}

HANDLE TwitterProto::SearchBasic(const TCHAR *username)
{
	ForkThread(&TwitterProto::DoSearch, new search_query(username, false));
	return (HANDLE)1;
}

HANDLE TwitterProto::SearchByEmail(const TCHAR *email)
{
	ForkThread(&TwitterProto::DoSearch, new search_query(email, true));
	return (HANDLE)1;
}

// *************************

void TwitterProto::GetAwayMsgWorker(void *hContact)
{
	if (hContact == 0)
		return;

	DBVARIANT dbv;
	if (!db_get_ts((MCONTACT)hContact, "CList", "StatusMsg", &dbv)) {
		ProtoBroadcastAck((MCONTACT)hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)dbv.ptszVal);
		db_free(&dbv);
	}
	else ProtoBroadcastAck((MCONTACT)hContact, ACKTYPE_AWAYMSG, ACKRESULT_FAILED, (HANDLE)1, 0);
}

HANDLE TwitterProto::GetAwayMsg(MCONTACT hContact)
{
	ForkThread(&TwitterProto::GetAwayMsgWorker, (void*)hContact);
	return (HANDLE)1;
}

int TwitterProto::OnContactDeleted(WPARAM hContact, LPARAM)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 0;

	if (!IsMyContact(hContact))
		return 0;

	DBVARIANT dbv;
	if (!db_get_s(hContact, m_szModuleName, TWITTER_KEY_UN, &dbv)) {
		if (in_chat_)
			DeleteChatContact(dbv.pszVal);

		mir_cslock s(twitter_lock_);
		twit_.remove_friend(dbv.pszVal); // Be careful about this until Miranda is fixed
		db_free(&dbv);
	}
	return 0;
}

// *************************

bool TwitterProto::IsMyContact(MCONTACT hContact, bool include_chat)
{
	char *proto = GetContactProto(hContact);
	if (proto && mir_strcmp(m_szModuleName, proto) == 0) {
		if (include_chat)
			return true;
		return !isChatRoom(hContact);
	}
	else return false;
}

MCONTACT TwitterProto::UsernameToHContact(const char *name)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (db_get_b(hContact, m_szModuleName, "ChatRoom", 0))
			continue;

		DBVARIANT dbv;
		if (!db_get_s(hContact, m_szModuleName, TWITTER_KEY_UN, &dbv)) {
			if (mir_strcmp(name, dbv.pszVal) == 0) {
				db_free(&dbv);
				return hContact;
			}
			else
				db_free(&dbv);
		}
	}

	return 0;
}

MCONTACT TwitterProto::AddToClientList(const char *name, const char *status)
{
	// First, check if this contact exists
	MCONTACT hContact = UsernameToHContact(name);
	if (hContact)
		return hContact;

	if (in_chat_)
		AddChatContact(name);

	// If not, make a new contact!
	hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if (hContact) {
		if (CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName) == 0) {
			db_set_s(hContact, m_szModuleName, TWITTER_KEY_UN, name);
			db_set_w(hContact, m_szModuleName, "Status", ID_STATUS_ONLINE);
			db_set_utf(hContact, "CList", "StatusMsg", status);

			std::string url = profile_base_url(twit_.get_base_url()) + http::url_encode(name);
			db_set_s(hContact, m_szModuleName, "Homepage", url.c_str());

			DBVARIANT dbv;
			if (!db_get_ts(NULL, m_szModuleName, TWITTER_KEY_GROUP, &dbv)) {
				db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
				db_free(&dbv);
			}

			return hContact;
		}
		CallService(MS_DB_CONTACT_DELETE, hContact, 0);
	}

	return 0;
}

void TwitterProto::SetAllContactStatuses(int status)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		if (!db_get_b(hContact, m_szModuleName, "ChatRoom", 0))
			db_set_w(hContact, m_szModuleName, "Status", (WORD)status);

	SetChatStatus(status);
}
