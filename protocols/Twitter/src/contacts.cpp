/*
Copyright © 2012-19 Miranda NG team
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

void TwitterProto::AddToListWorker(void *pArg)
{
	// TODO: what happens if there is an error?
	if (pArg == nullptr)
		return;

	char *name = static_cast<char*>(pArg);

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

MCONTACT TwitterProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	if (m_iStatus != ID_STATUS_ONLINE)
		return 0;

	ForkThread(&TwitterProto::AddToListWorker, mir_utf8encodeW(psr->nick.w));
	return AddToClientList(_T2A(psr->nick.w), "");
}

// *************************

void TwitterProto::UpdateInfoWorker(void *arg)
{
	MCONTACT hContact = (MCONTACT)(DWORD_PTR)arg;
	twitter_user user;

	ptrA username(getStringA(hContact, TWITTER_KEY_UN));
	if (username == NULL)
		return;

	{
		mir_cslock s(twitter_lock_);
		twit_.get_info(std::string(username), &user);
	}

	UpdateAvatar(hContact, user.profile_image_url, true);
	ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, nullptr);
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
	search_query(const std::wstring &_query, bool _by_email) : query(_query), by_email(_by_email)
	{
	}
	std::wstring query;
	bool by_email;
};

void TwitterProto::DoSearch(void *pArg)
{
	if (pArg == nullptr)
		return;

	search_query *query = static_cast<search_query*>(pArg);
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
		psr.flags = PSR_UNICODE;
		psr.nick.w = mir_a2u(info.username.c_str());
		psr.firstName.w = mir_a2u(info.real_name.c_str());

		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);

		mir_free(psr.nick.w);
		mir_free(psr.firstName.w);
	}
	else ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);

	delete query;
}

HANDLE TwitterProto::SearchBasic(const wchar_t *username)
{
	ForkThread(&TwitterProto::DoSearch, new search_query(username, false));
	return (HANDLE)1;
}

HANDLE TwitterProto::SearchByEmail(const wchar_t *email)
{
	ForkThread(&TwitterProto::DoSearch, new search_query(email, true));
	return (HANDLE)1;
}

// *************************

void TwitterProto::GetAwayMsgWorker(void *arg)
{
	MCONTACT hContact = (MCONTACT)(DWORD_PTR)arg;
	if (hContact == 0)
		return;

	ptrW wszMsg(db_get_wsa(hContact, "CList", "StatusMsg"));
	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, wszMsg);
}

HANDLE TwitterProto::GetAwayMsg(MCONTACT hContact)
{
	ForkThread(&TwitterProto::GetAwayMsgWorker, (void*)hContact);
	return (HANDLE)1;
}

int TwitterProto::OnContactDeleted(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (m_iStatus != ID_STATUS_ONLINE)
		return 0;

	if (!IsMyContact(hContact))
		return 0;

	DBVARIANT dbv;
	if (!getString(hContact, TWITTER_KEY_UN, &dbv)) {
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
	for (auto &hContact : AccContacts()) {
		if (getByte(hContact, "ChatRoom"))
			continue;

		if (getMStringA(hContact, TWITTER_KEY_UN) == name)
			return hContact;
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
	hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	setString(hContact, TWITTER_KEY_UN, name);
	setString(hContact, "Homepage", "https://twitter.com/" + CMStringA(name));
	setWord(hContact, "Status", ID_STATUS_ONLINE);
	db_set_utf(hContact, "CList", "StatusMsg", status);

	Skin_PlaySound("TwitterNewContact");
	Clist_SetGroup(hContact, getMStringW(TWITTER_KEY_GROUP));
	return hContact;
}
