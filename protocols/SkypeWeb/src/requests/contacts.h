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

class CreateContactsRequest : public HttpRequest
{
public:
	CreateContactsRequest(const char *regToken, const LIST<char> &skypenames, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/contacts", server)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken);

		CMStringA data = "{\"contacts\":[";
		for (int i = 0; i < skypenames.getCount(); i++)
			data.AppendFormat("{\"id\":\"8:%s\"},", skypenames[i]);
		data.Truncate(data.GetLength() - 1);
		data.Append("]}");

		Body << VALUE(data);
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