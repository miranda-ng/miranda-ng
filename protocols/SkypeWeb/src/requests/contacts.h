#ifndef _SKYPE_REQUEST_CONTACTS_H_
#define _SKYPE_REQUEST_CONTACTS_H_

class GetContactsRequest : public HttpRequest
{
public:
	GetContactsRequest(const char *token) :
		HttpRequest(REQUEST_GET, "api.skype.com/users/self/contacts")
	{
		flags |= NLHRF_SSL;

		AddHeader("X-Skypetoken", token);
		AddHeader("Accept", "application/json");
	}
};

#endif //_SKYPE_REQUEST_CONTACTS_H_
