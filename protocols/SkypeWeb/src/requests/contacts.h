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

class GetContactListRequest : public HttpRequest
{
public:
	GetContactListRequest(CSkypeProto *ppro, const char *filter) :
	  HttpRequest(REQUEST_GET, FORMAT, "contacts.skype.com/contacts/v1/users/%s/contacts", ppro->m_szSkypename.MakeLower().GetBuffer())
	{
		if (filter != NULL)
		{
			Url
				<< CHAR_VALUE ("filter", filter); //?filter=contacts[?(@.type="skype" or @.type="msn")]
		}

		Headers 
			<< CHAR_VALUE("X-SkypeToken", ppro->m_szApiToken);
	}
};

class GetContactsAuthRequest : public HttpRequest
{
public:
	GetContactsAuthRequest(CSkypeProto *ppro) :
		HttpRequest(REQUEST_GET, FORMAT, "contacts.skype.com/contacts/v2/users/SELF/invites")
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", ppro->m_szApiToken)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

class AddContactRequest : public HttpRequest
{
public:
	AddContactRequest(CSkypeProto *ppro, const char *who, const char *greeting = "") :
		HttpRequest(REQUEST_PUT, "contacts.skype.com/contacts/v2/users/SELF/contacts")
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", ppro->m_szApiToken)
			<< CHAR_VALUE("Accept", "application/json")
			<< CHAR_VALUE("Content-type", "application/x-www-form-urlencoded");

		JSONNode node;
		node 
			<< JSONNode("mri", CMStringA(::FORMAT, "8:", who).GetString())
			<< JSONNode("greeting", greeting);

		Body << VALUE(node.write().c_str());
	}
};

class DeleteContactRequest : public HttpRequest
{
public:
	DeleteContactRequest(CSkypeProto *ppro, const char *who) :
		HttpRequest(REQUEST_DELETE, FORMAT, "contacts.skype.com/contacts/v2/users/SELF/contacts/8:%s", who)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", ppro->m_szApiToken)
			<< CHAR_VALUE("Accept", "application/json")
			<< CHAR_VALUE("Content-type", "application/x-www-form-urlencoded");
	}
};

class AuthAcceptRequest : public HttpRequest
{
public:
	AuthAcceptRequest(CSkypeProto *ppro, const char *who) :
		HttpRequest(REQUEST_PUT, FORMAT, "contacts.skype.com/contacts/v2/users/SELF/invites/8:%s/accept", who)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", ppro->m_szApiToken)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

class AuthDeclineRequest : public HttpRequest
{
public:
	AuthDeclineRequest(CSkypeProto *ppro, const char *who) :
		HttpRequest(REQUEST_PUT, FORMAT, "contacts.skype.com/contacts/v2/users/SELF/invites/8:%s/decline", who)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", ppro->m_szApiToken)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

class BlockContactRequest : public HttpRequest
{
public:
	BlockContactRequest(CSkypeProto *ppro, const char *who) :
		HttpRequest(REQUEST_PUT, FORMAT, "contacts.skype.com/contacts/v2/users/SELF/contacts/blocklist/8:%s", who)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", ppro->m_szApiToken)
			<< CHAR_VALUE("Accept", "application/json")
			<< CHAR_VALUE("Content-type", "application/x-www-form-urlencoded");

		Body << VALUE("{\"report_abuse\":\"false\",\"ui_version\":\"skype.com\"}");
	}
};

class UnblockContactRequest : public HttpRequest
{
public:
	UnblockContactRequest(CSkypeProto *ppro, const char *who) :
		HttpRequest(REQUEST_DELETE, FORMAT, "contacts.skype.com/contacts/v2/users/SELF/contacts/blocklist/8:%s", who)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", ppro->m_szApiToken)
			<< CHAR_VALUE("Accept", "application/json")
			<< CHAR_VALUE("Content-type", "application/x-www-form-urlencoded");

		Body
			<< CHAR_VALUE("reporterIp", "123.123.123.123") //TODO: user ip address
			<< CHAR_VALUE("uiVersion", g_szMirVer);
	}
};

#endif //_SKYPE_REQUEST_CONTACTS_H_