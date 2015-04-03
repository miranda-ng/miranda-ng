#ifndef _SKYPE_REQUEST_ENDPOINT_H_
#define _SKYPE_REQUEST_ENDPOINT_H_

class GetEndpointRequest : public HttpRequest
{
public:
	GetEndpointRequest(const char *regToken, const char *endpointID, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/endpoints/%s/presenceDocs/messagingService", server, ptrA(mir_urlEncode(endpointID)))
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Expires", "0")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< CHAR_VALUE("BehaviorOverride", "redirectAs404")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");
		CMStringA data;
#ifdef _WIN64
		data.AppendFormat ("{\"id\":\"messagingService\",\"type\":\"EndpointPresenceDoc\",\"selfLink\":\"uri\",\"privateInfo\":{\"epname\":\"Miranda\"},\"publicInfo\":{\"capabilities\":\"\",\"typ\":125,\"skypeNameVersion\":\"Miranda NG Skype\",\"nodeInfo\":\"xx\",\"version\":\"%s x64\"}}", MIRANDA_VERSION_STRING);
#else
		data.AppendFormat("{\"id\":\"messagingService\",\"type\":\"EndpointPresenceDoc\",\"selfLink\":\"uri\",\"privateInfo\":{\"epname\":\"Miranda\"},\"publicInfo\":{\"capabilities\":\"\",\"typ\":125,\"skypeNameVersion\":\"Miranda NG Skype\",\"nodeInfo\":\"xx\",\"version\":\"%s x86\"}}", MIRANDA_VERSION_STRING);
#endif
		Body <<
			VALUE(data);
	}
};
#endif //_SKYPE_REQUEST_ENDPOINT_H_