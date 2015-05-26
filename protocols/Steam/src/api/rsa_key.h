#ifndef _STEAM_REQUEST_RSA_KEY_H_
#define _STEAM_REQUEST_RSA_KEY_H_

class RsaKeyRequest : public HttpRequest
{
public:
	RsaKeyRequest(const char *username) :
		HttpRequest(REQUEST_GET, STEAM_WEB_URL "/mobilelogin/getrsakey")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		AddParameter("username", (char*)username);
	}
};

#endif //_STEAM_REQUEST_RSA_KEY_H_
