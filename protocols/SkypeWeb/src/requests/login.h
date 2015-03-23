#ifndef _SKYPE_REQUEST_LOGIN_H_
#define _SKYPE_REQUEST_LOGIN_H_

class LoginRequest : public HttpsPostRequest
{
public:
	LoginRequest() :
		HttpsPostRequest("login.skype.com/login")
	{
		//flags = NLHRF_SSL | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;
		Url
			<< INT_VALUE("client_id", 578134)
			<< CHAR_VALUE("redirect_uri", "https%3A%2F%2Fweb.skype.com");

		Headers
			<< CHAR_VALUE("Host", "login.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");
	}

	LoginRequest(const char *skypename, const char *password, const char *pie, const char *etm) :
		HttpsPostRequest("login.skype.com/login")
	{
		//flags = NLHRF_SSL | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;
		Url
			<< INT_VALUE("client_id", 578134)
			<< CHAR_VALUE("redirect_uri", "https%3A%2F%2Fweb.skype.com");

		Headers
			<< CHAR_VALUE("Host", "login.skype.com")
			<< CHAR_VALUE("Referer", "https://login.skype.com/login?method=skype&client_id=578134&redirect_uri=https%3A%2F%2Fweb.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");

		LPTIME_ZONE_INFORMATION tzi = tmi.getTziByContact(NULL);
		char sign = tzi->Bias > 0 ? '-' : '+';
		int hours = tzi->Bias / -60;
		int minutes = tzi->Bias % -60;

		Body
			<< CHAR_VALUE("username", skypename)
			<< CHAR_VALUE("password", password)
			<< CHAR_VALUE("pie", ptrA(mir_urlEncode(pie)))
			<< CHAR_VALUE("etm", ptrA(mir_urlEncode(etm)))
			<< FORMAT_VALUE("timezone_field", "%c%02d|%02d", sign, hours, minutes)
			<< FORMAT_VALUE("js_time", "%d.00", time(NULL))
			<< INT_VALUE("client_id", 578134)
			<< CHAR_VALUE("redirect_uri", "https%3A%2F%2Fweb.skype.com");
	}
};

#endif //_SKYPE_REQUEST_LOGIN_H_
