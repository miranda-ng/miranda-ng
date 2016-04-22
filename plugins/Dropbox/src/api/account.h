#ifndef _DROPBOX_API_ACCOUNT_H_
#define _DROPBOX_API_ACCOUNT_H_

class GetAccessTokenRequest : public HttpRequest
{
public:
	GetAccessTokenRequest(const char *requestToken) :
		HttpRequest(REQUEST_POST, DROPBOX_API "/oauth2/token")
	{
		AddHeader("Content-Type", "application/x-www-form-urlencoded");

		CMStringA data(CMStringDataFormat::FORMAT,
			"client_id=%s&client_secret=%s&grant_type=authorization_code&code=%s",
			DROPBOX_APP_KEY, DROPBOX_API_SECRET, requestToken);
		SetData(data.GetBuffer(), data.GetLength());
	}
};

class RevokeAccessTokenRequest : public HttpRequest
{
public:
	RevokeAccessTokenRequest() :
		HttpRequest(REQUEST_POST, DROPBOX_API "/oauth2/token/revoke")
	{
	}
};

class GetCurrentAccountRequest : public HttpRequest
{
public:
	GetCurrentAccountRequest(const char *token) :
		HttpRequest(REQUEST_POST, DROPBOX_API_RPC "/users/get_current_account")
	{
		AddBearerAuthHeader(token);
	}
};

#endif //_DROPBOX_API_ACCOUNT_H_
