/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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
	GetContactListRequest(const char *token, const char *skypename = "self") :
		HttpRequest(REQUEST_GET, FORMAT, "api.skype.com/users/%s/contacts", skypename)
	{
		Url << CHAR_VALUE("hideDetails", "true");

		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

class GetContactsInfoRequest : public HttpRequest
{
public:
	GetContactsInfoRequest(const char *token, const LIST<char> &skypenames, const char *skypename = "self") :
		HttpRequest(REQUEST_POST, FORMAT, "api.skype.com/users/%s/contacts/profiles", skypename)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");

		for (int i = 0; i < skypenames.getCount(); i++)
			Body << CHAR_VALUE("contacts[]", skypenames[i]);
	}
};

class GetContactStatusRequest : public HttpRequest
{
public:
	GetContactStatusRequest(const char *regToken, const char *skypename, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_GET, FORMAT, "%s/v1/users/ME/contacts/8:%s/presenceDocs/messagingService", server, skypename)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken);
	}
};

class GetContactsAuthRequest : public HttpRequest
{
public:
	GetContactsAuthRequest(const char *token, const char *skypename = "self") :
		HttpRequest(REQUEST_GET, FORMAT, "api.skype.com/users/%s/contacts/auth-request", skypename)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

class AddContactRequest : public HttpRequest
{
public:
	AddContactRequest(const char *token, const char *who, const char *greeting = "", const char *skypename = "self") :
		HttpRequest(REQUEST_PUT, FORMAT, "api.skype.com/users/%s/contacts/auth-request/%s", skypename, who)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json")
			<< CHAR_VALUE("Content-type", "application/x-www-form-urlencoded");
		CMStringA data;
		data.AppendFormat("greeting=%s", ptrA(mir_urlEncode(greeting)));
		Body << VALUE(data);
	}
};

class DeleteContactRequest : public HttpRequest
{
public:
	DeleteContactRequest(const char *token, const char *who, const char *skypename = "self") :
		HttpRequest(REQUEST_DELETE, FORMAT, "api.skype.com/users/%s/contacts/%s", skypename, who)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json")
			<< CHAR_VALUE("Content-type", "application/x-www-form-urlencoded");
	}
};

class AuthAcceptRequest : public HttpRequest
{
public:
	AuthAcceptRequest(const char *token, const char *who, const char *skypename = "self") :
		HttpRequest(REQUEST_PUT, FORMAT, "api.skype.com/users/%s/contacts/auth-request/%s/accept", skypename, who)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

class AuthDeclineRequest : public HttpRequest
{
public:
	AuthDeclineRequest(const char *token, const char *who, const char *skypename = "self") :
		HttpRequest(REQUEST_PUT, FORMAT, "api.skype.com/users/%s/contacts/auth-request/%s/decline", skypename, who)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

#endif //_SKYPE_REQUEST_CONTACTS_H_