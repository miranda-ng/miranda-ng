#ifndef _DROPBOX_API_ACCOUNT_H_
#define _DROPBOX_API_ACCOUNT_H_

class GetAccessTokenRequest : public HttpRequest
{
public:
	GetAccessTokenRequest(const char *requestToken) :
		HttpRequest(REQUEST_POST, DROPBOX_API_URL "/oauth2/token")
	{
		AddBasicAuthHeader(DROPBOX_APP_KEY, DROPBOX_API_SECRET);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");

		CMStringA data(CMStringDataFormat::FORMAT, "grant_type=authorization_code&code=%s", requestToken);
		SetData(data.GetBuffer(), data.GetLength());
	}
};

class DisableAccessTokenRequest : public HttpRequest
{
public:
	DisableAccessTokenRequest() :
		HttpRequest(REQUEST_POST, DROPBOX_API_URL "/disable_access_token")
	{
	}
};

class GetAccountInfoRequest : public HttpRequest
{
public:
	GetAccountInfoRequest(const char *token) :
		HttpRequest(REQUEST_GET, DROPBOX_API_URL "/account/info")
	{
		AddBearerAuthHeader(token);
	}
};

#endif //_DROPBOX_API_ACCOUNT_H_
