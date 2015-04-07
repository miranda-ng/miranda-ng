#ifndef _SKYPE_REQUEST_CAPS_H_
#define _SKYPE_REQUEST_CAPS_H_

class SendCapabilitiesRequest : public HttpRequest
{
public:
	SendCapabilitiesRequest(const char *regToken, const char *endpointID, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/endpoints/%s/presenceDocs/messagingService", server, ptrA(mir_urlEncode(endpointID)))
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");
		CMStringA data;

		int bitness = 32;
#ifdef _WIN64
		bitness = 64;
#endif
		data.AppendFormat("{\"id\":\"messagingService\",\"type\":\"EndpointPresenceDoc\",\"selfLink\":\"uri\",\"privateInfo\":{\"epname\":\"Miranda\"},\"publicInfo\":{\"capabilities\":\"\",\"typ\":125,\"skypeNameVersion\":\"Miranda NG Skype\",\"nodeInfo\":\"xx\",\"version\":\"%s x%d\"}}", MIRANDA_VERSION_STRING, bitness);
		Body <<
			VALUE(data);
	}
};
#endif //_SKYPE_REQUEST_CAPS_H_