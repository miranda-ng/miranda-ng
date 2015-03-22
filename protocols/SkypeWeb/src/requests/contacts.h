#ifndef _SKYPE_REQUEST_CONTACTS_H_
#define _SKYPE_REQUEST_CONTACTS_H_

class GetContactListRequest : public HttpRequest
{
public:
	GetContactListRequest(const char *token, const char *skypename = "self") :
		HttpRequest(REQUEST_GET, "api.skype.com/users/%s/contacts?hideDetails=true", skypename)
	{
		flags |= NLHRF_SSL;

		Url << CHAR_VALUE("hideDetails", "true");

		Headers
			<< CHAR_VALUE("X-Skypetoken", "Accept")
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

class GetContactsInfoRequest : public HttpRequest
{
public:
	GetContactsInfoRequest(const char *token, const LIST<char> &skypenames, const char *skypename = "self") :
		HttpRequest(REQUEST_POST, "api.skype.com/users/%s/contacts/profiles", skypename)
	{
		flags |= NLHRF_SSL;

		Headers
			<< CHAR_VALUE("X-Skypetoken", "Accept")
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");

		for (size_t i = 0; i < skypenames.getCount(); i++)
		{
			Body << CHAR_VALUE("contacts[]", skypenames[i]);
		}
	}
};

class GetContactsAuthRequest : public HttpRequest
{
public:
	GetContactsAuthRequest(const char *token, const char *skypename = "self") :
		HttpRequest(REQUEST_GET, "api.skype.com/users/%s/contacts/auth-request", skypename)
	{
		flags |= NLHRF_SSL;

		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

class AuthAcceptRequest : public HttpRequest
{
public:
	AuthAcceptRequest(const char *token, const char *who, const char *skypename = "self") :
		HttpRequest(REQUEST_GET, "api.skype.com/users/%s/contacts/auth-request/%s/accept", skypename, who)
	{
		flags |= NLHRF_SSL;

		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

class AuthDeclineRequest : public HttpRequest
{
public:
	AuthDeclineRequest(const char *token, const char *who, const char *skypename = "self") :
		HttpRequest(REQUEST_GET, "api.skype.com/users/%s/contacts/auth-request/%s/decline", skypename)
	{
		flags |= NLHRF_SSL;

		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");
	}
};

#endif //_SKYPE_REQUEST_CONTACTS_H_
