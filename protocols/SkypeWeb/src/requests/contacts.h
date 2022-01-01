/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

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
	GetContactListRequest(CSkypeProto *ppro, const char *filter) :
	  AsyncHttpRequest(REQUEST_GET, HOST_CONTACTS, 0, &CSkypeProto::LoadContactList)
	{
		m_szUrl.AppendFormat("/contacts/v1/users/%s/contacts", ppro->m_szSkypename.MakeLower().GetBuffer());

		// ?filter=contacts[?(@.type="skype" or @.type="msn")]
		if (filter != NULL)
			this << CHAR_PARAM("filter", filter);
	}
};

struct GetContactsAuthRequest : public AsyncHttpRequest
{
	GetContactsAuthRequest() :
		AsyncHttpRequest(REQUEST_GET, HOST_CONTACTS, "/contacts/v2/users/SELF/invites", &CSkypeProto::LoadContactsAuth)
	{
		AddHeader("Accept", "application/json");
	}
};

struct AddContactRequest : public AsyncHttpRequest
{
	AddContactRequest(const char *who, const char *greeting = "") :
		AsyncHttpRequest(REQUEST_PUT, HOST_CONTACTS, "/contacts/v2/users/SELF/contacts")
	{
		AddHeader("Accept", "application/json");

		JSONNode node;
		node << CHAR_PARAM("mri", who) << CHAR_PARAM("greeting", greeting);
		m_szParam = node.write().c_str();
	}
};

struct DeleteContactRequest : public AsyncHttpRequest
{
	DeleteContactRequest(const char *who) :
		AsyncHttpRequest(REQUEST_DELETE, HOST_CONTACTS, "/contacts/v2/users/SELF/contacts/" + mir_urlEncode(who))
	{
		AddHeader("Accept", "application/json");
	}
};

struct AuthAcceptRequest : public AsyncHttpRequest
{
	AuthAcceptRequest(const char *who) :
		AsyncHttpRequest(REQUEST_PUT, HOST_CONTACTS)
	{
		m_szUrl.AppendFormat("/contacts/v2/users/SELF/invites/%s/accept", who);

		AddHeader("Accept", "application/json");
	}
};

struct AuthDeclineRequest : public AsyncHttpRequest
{
	AuthDeclineRequest(const char *who) :
		AsyncHttpRequest(REQUEST_PUT, HOST_CONTACTS)
	{
		m_szUrl.AppendFormat("/contacts/v2/users/SELF/invites/%s/decline", who);

		AddHeader("Accept", "application/json");
	}
};

struct BlockContactRequest : public AsyncHttpRequest
{
	BlockContactRequest(CSkypeProto *ppro, MCONTACT hContact) :
		AsyncHttpRequest(REQUEST_PUT, HOST_CONTACTS, 0, &CSkypeProto::OnBlockContact)
	{
		m_szUrl.AppendFormat("/contacts/v2/users/SELF/contacts/blocklist/%s", ppro->getId(hContact).c_str());
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
		m_szUrl.AppendFormat("/contacts/v2/users/SELF/contacts/blocklist/%s", ppro->getId(hContact).c_str());
		pUserInfo = (void *)hContact;

		AddHeader("Accept", "application/json");

		// TODO: user ip address
		this << CHAR_PARAM("reporterIp", "123.123.123.123") << CHAR_PARAM("uiVersion", g_szMirVer);
	}
};

#endif //_SKYPE_REQUEST_CONTACTS_H_