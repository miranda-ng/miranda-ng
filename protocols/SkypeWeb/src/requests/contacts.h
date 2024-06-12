/*
Copyright (c) 2015-24 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SKYPE_REQUEST_CONTACTS_H_
#define _SKYPE_REQUEST_CONTACTS_H_

struct GetContactListRequest : public AsyncHttpRequest
{
	GetContactListRequest() :
	  AsyncHttpRequest(REQUEST_GET, HOST_CONTACTS, "/users/SELF/contacts", &CSkypeProto::LoadContactList)
	{
	}
};

struct GetContactsAuthRequest : public AsyncHttpRequest
{
	GetContactsAuthRequest() :
		AsyncHttpRequest(REQUEST_GET, HOST_CONTACTS, "/users/SELF/invites", &CSkypeProto::LoadContactsAuth)
	{
		AddHeader("Accept", "application/json");
	}
};

struct AddContactRequest : public AsyncHttpRequest
{
	AddContactRequest(const char *who, const char *greeting = "") :
		AsyncHttpRequest(REQUEST_PUT, HOST_CONTACTS, "/users/SELF/contacts")
	{
		AddHeader("Accept", "application/json");

		JSONNode node;
		node << CHAR_PARAM("mri", who) << CHAR_PARAM("greeting", greeting);
		m_szParam = node.write().c_str();
	}
};

struct DeleteContactRequest : public AsyncHttpRequest
{
	DeleteContactRequest(CSkypeProto *ppro, const char *who) :
		AsyncHttpRequest(REQUEST_DELETE, HOST_CONTACTS)
	{
		m_szUrl.AppendFormat("/users/%s/contacts/%s", ppro->m_szSkypename.c_str(), who);

		AddHeader("Accept", "application/json");
	}
};

struct AuthAcceptRequest : public AsyncHttpRequest
{
	AuthAcceptRequest(CSkypeProto *ppro, const char *who) :
		AsyncHttpRequest(REQUEST_PUT, HOST_CONTACTS)
	{
		m_szUrl.AppendFormat("/users/%s/invites/%s/accept", ppro->m_szSkypename.c_str(), who);

		AddHeader("Accept", "application/json");
	}
};

struct AuthDeclineRequest : public AsyncHttpRequest
{
	AuthDeclineRequest(CSkypeProto *ppro, const char *who) :
		AsyncHttpRequest(REQUEST_PUT, HOST_CONTACTS)
	{
		m_szUrl.AppendFormat("/users/%s/invites/%s/decline", ppro->m_szSkypename.c_str(), who);

		AddHeader("Accept", "application/json");
	}
};

struct BlockContactRequest : public AsyncHttpRequest
{
	BlockContactRequest(CSkypeProto *ppro, MCONTACT hContact) :
		AsyncHttpRequest(REQUEST_PUT, HOST_CONTACTS, "/users/SELF/contacts/blocklist/" + ppro->getId(hContact), &CSkypeProto::OnBlockContact)
	{
		m_szParam = "{\"report_abuse\":\"false\",\"ui_version\":\"skype.com\"}";
		pUserInfo = (void *)hContact;

		AddHeader("Accept", "application/json");
	}
};

struct UnblockContactRequest : public AsyncHttpRequest
{
	UnblockContactRequest(CSkypeProto *ppro, MCONTACT hContact) :
		AsyncHttpRequest(REQUEST_DELETE, HOST_CONTACTS, 0, &CSkypeProto::OnUnblockContact)
	{
		m_szUrl.AppendFormat("/users/SELF/contacts/blocklist/%s", ppro->getId(hContact).c_str());
		pUserInfo = (void *)hContact;

		AddHeader("Accept", "application/json");

		// TODO: user ip address
		this << CHAR_PARAM("reporterIp", "123.123.123.123") << CHAR_PARAM("uiVersion", g_szMirVer);
	}
};

#endif //_SKYPE_REQUEST_CONTACTS_H_