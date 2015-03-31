#ifndef _SKYPE_REQUEST_SUBSCIPTIONS_H_
#define _SKYPE_REQUEST_SUBSCIPTIONS_H_

class SubscriptionsRequest : public HttpsPostRequest
{
public:
	SubscriptionsRequest(const char *regToken) :
		HttpsPostRequest("client-s.gateway.messenger.live.com/v1/users/ME/endpoints/SELF/subscriptions")
	{
		CMStringA auth = "registrationToken=";
		auth += regToken;
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Expires", "0")
			<< CHAR_VALUE("RegistrationToken", auth)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8")
			<< CHAR_VALUE("BehaviorOverride", "redirectAs404")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");

			const char *data = "{\"channelType\":\"httpLongPoll\",\"template\":\"raw\",\"interestedResources\":[\"/v1/users/ME/conversations/ALL/properties\",\"/v1/users/ME/conversations/ALL/messages\",\"/v1/users/ME/contacts/ALL\",\"/v1/threads/ALL\"]}";
			Body << VALUE(data);
	}
};

#endif //_SKYPE_REQUEST_SUBSCIPTIONS_H_
