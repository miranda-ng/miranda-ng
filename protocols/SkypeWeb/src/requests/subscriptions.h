#ifndef _SKYPE_REQUEST_SUBSCIPTIONS_H_
#define _SKYPE_REQUEST_SUBSCIPTIONS_H_

class SubscriptionsRequest : public HttpRequest
{
public:
	SubscriptionsRequest(const char *regToken, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints/SELF/subscriptions", server)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		const char *data = "{\"channelType\":\"httpLongPoll\",\"template\":\"raw\",\"interestedResources\":[\"/v1/users/ME/conversations/ALL/properties\",\"/v1/users/ME/conversations/ALL/messages\",\"/v1/users/ME/contacts/ALL\",\"/v1/threads/ALL\"]}";
		Body << VALUE(data);
	}
};

#endif //_SKYPE_REQUEST_SUBSCIPTIONS_H_
