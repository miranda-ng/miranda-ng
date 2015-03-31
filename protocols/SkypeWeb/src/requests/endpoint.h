#ifndef _SKYPE_REQUEST_ENDPOINT_H_
#define _SKYPE_REQUEST_ENDPOINT_H_

class GetEndpointRequest : public HttpsRequest
{
public:
	GetEndpointRequest(const char *regToken, const char *endpointID) :
		HttpsRequest(REQUEST_PUT, "client-s.gateway.messenger.live.com/v1/users/ME/endpoints/%s/presenceDocs/messagingService", mir_urlEncode(endpointID))
	{
		flags |= NLHRF_SSL;
		CMStringA auth = "registrationToken=";
		auth += regToken;
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Expires", "0")
			<< CHAR_VALUE("RegistrationToken", auth)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< CHAR_VALUE("BehaviorOverride", "redirectAs404")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");

		Body <<
			VALUE("{\"id\":\"messagingService\",\"type\":\"EndpointPresenceDoc\",\"selfLink\":\"uri\",\"privateInfo\":{\"epname\":\"Miranda\"},\"publicInfo\":{\"capabilities\":\"\",\"type\":1,\"skypeNameVersion\":\"0/0.95.4//\",\"nodeInfo\":\"xx\",\"version\":\"0/0.95.4\"}}");
	}
};
#endif //_SKYPE_REQUEST_ENDPOINT_H_