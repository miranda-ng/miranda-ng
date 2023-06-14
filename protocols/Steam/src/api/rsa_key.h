#ifndef _STEAM_REQUEST_RSA_KEY_H_
#define _STEAM_REQUEST_RSA_KEY_H_

class GetRsaKeyRequest : public HttpRequest
{
public:
	GetRsaKeyRequest(const char *username) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/mobilelogin/getrsakey/")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		this
			<< CHAR_PARAM("username", username)
			<< INT64_PARAM("donotcache", now());
	}
};

#endif //_STEAM_REQUEST_RSA_KEY_H_
