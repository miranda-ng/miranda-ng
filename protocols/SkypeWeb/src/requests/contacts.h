/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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
	  AsyncHttpRequest(REQUEST_GET, 0, &CSkypeProto::LoadContactList)
	{
		m_szUrl.Format("contacts.skype.com/contacts/v1/users/%s/contacts", ppro->m_szSkypename.MakeLower().GetBuffer());

		// ?filter=contacts[?(@.type="skype" or @.type="msn")]
		if (filter != NULL)
			this << CHAR_PARAM("filter", filter);

		AddHeader("X-SkypeToken", ppro->m_szApiToken);
	}
};

struct GetContactsAuthRequest : public AsyncHttpRequest
{
	GetContactsAuthRequest(CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_GET, "contacts.skype.com/contacts/v2/users/SELF/invites", &CSkypeProto::LoadContactsAuth)
	{
		AddHeader("X-Skypetoken", ppro->m_szApiToken);
		AddHeader("Accept", "application/json");
	}
};

struct AddContactRequest : public AsyncHttpRequest
{
	AddContactRequest(CSkypeProto *ppro, const char *who, const char *greeting = "") :
		AsyncHttpRequest(REQUEST_PUT, "contacts.skype.com/contacts/v2/users/SELF/contacts")
	{
		AddHeader("X-Skypetoken", ppro->m_szApiToken);
		AddHeader("Accept", "application/json");
		AddHeader("Content-type", "application/x-www-form-urlencoded");

		JSONNode node;
		node << JSONNode("mri", CMStringA(::FORMAT, "8:", who).GetString())
			<< JSONNode("greeting", greeting);

		m_szParam = node.write().c_str();
	}
};

struct DeleteContactRequest : public AsyncHttpRequest
{
	DeleteContactRequest(CSkypeProto *ppro, const char *who) :
		AsyncHttpRequest(REQUEST_DELETE)
	{
		m_szUrl.Format("contacts.skype.com/contacts/v2/users/SELF/contacts/8:%s", who);

		AddHeader("X-Skypetoken", ppro->m_szApiToken);
		AddHeader("Accept", "application/json");
	}
};

struct AuthAcceptRequest : public AsyncHttpRequest
{
	AuthAcceptRequest(CSkypeProto *ppro, const char *who) :
		AsyncHttpRequest(REQUEST_PUT)
	{
		m_szUrl.Format("contacts.skype.com/contacts/v2/users/SELF/invites/8:%s/accept", who);

		AddHeader("X-Skypetoken", ppro->m_szApiToken);
		AddHeader("Accept", "application/json");
	}
};

struct AuthDeclineRequest : public AsyncHttpRequest
{
	AuthDeclineRequest(CSkypeProto *ppro, const char *who) :
		AsyncHttpRequest(REQUEST_PUT)
	{
		m_szUrl.Format("contacts.skype.com/contacts/v2/users/SELF/invites/8:%s/decline", who);

		AddHeader("X-Skypetoken", ppro->m_szApiToken);
		AddHeader("Accept", "application/json");
	}
};

struct BlockContactRequest : public AsyncHttpRequest
{
	BlockContactRequest(CSkypeProto *ppro, MCONTACT hContact) :
		AsyncHttpRequest(REQUEST_PUT, 0, &CSkypeProto::OnBlockContact)
	{
		m_szUrl.Format("contacts.skype.com/contacts/v2/users/SELF/contacts/blocklist/8:%s", ppro->getId(hContact).c_str());
		m_szParam = "{\"report_abuse\":\"false\",\"ui_version\":\"skype.com\"}";
		pUserInfo = (void *)hContact;

		AddHeader("X-Skypetoken", ppro->m_szApiToken);
		AddHeader("Accept", "application/json");
		AddHeader("Content-type", "application/x-www-form-urlencoded");
	}
};

struct UnblockContactRequest : public AsyncHttpRequest
{
	UnblockContactRequest(CSkypeProto *ppro, MCONTACT hContact) :
		AsyncHttpRequest(REQUEST_DELETE, 0, &CSkypeProto::OnUnblockContact)
	{
		m_szUrl.Format("contacts.skype.com/contacts/v2/users/SELF/contacts/blocklist/8:%s", ppro->getId(hContact).c_str());
		pUserInfo = (void *)hContact;

		AddHeader("X-Skypetoken", ppro->m_szApiToken);
		AddHeader("Accept", "application/json");
		AddHeader("Content-type", "application/x-www-form-urlencoded");

		this << CHAR_PARAM("reporterIp", "123.123.123.123") // TODO: user ip address
			<< CHAR_PARAM("uiVersion", g_szMirVer);
	}
};

#endif //_SKYPE_REQUEST_CONTACTS_H_