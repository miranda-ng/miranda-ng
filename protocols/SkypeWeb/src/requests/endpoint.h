#ifndef _SKYPE_REQUEST_ENDPOINT_H_
#define _SKYPE_REQUEST_ENDPOINT_H_

class GetEndpointRequest : public HttpsRequest
{
public:
	GetEndpointRequest(const char *regToken, const char *endpointURL) :
		HttpsRequest(REQUEST_PUT, endpointURL)
	{
		flags |= NLHRF_SSL;
		CMStringA auth = "registrationToken=";
		auth += regToken;
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Expires", "0")
			<< CHAR_VALUE("RegistrationToken", auth)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");

		Body <<
			VALUE("{\"id\":\"messagingService\",\"type\":\"EndpointPresenceDoc\",\"selfLink\":\"uri\",\"privateInfo\":{\"epname\":\"skype\"},\"publicInfo\":{\"capabilities\":\"video | audio\",\"type\":1,\"skypeNameVersion\":\"908 / 1.0.30 / swx - skype.com\",\"nodeInfo\":\"xx\",\"version\":\"908 / 1.0.30\"}}");
	}
};
#endif //_SKYPE_REQUEST_ENDPOINT_H_