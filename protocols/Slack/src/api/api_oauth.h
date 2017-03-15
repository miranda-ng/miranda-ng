#ifndef _SLACK_API_OAUTH_H_
#define _SLACK_API_OAUTH_H_

class OAuhtAccessRequest : public HttpRequest
{
public:
	OAuhtAccessRequest(const char *code, const char *state = NULL) :
		HttpRequest(HttpMethod::HttpPost, SLACK_API_URL "/oauth.access")
	{
		Headers
			<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded;charset=utf-8");
		Content
			<< CHAR_VALUE("client_id", SLACK_CLIENT_ID)
			<< CHAR_VALUE("client_secret", SLACK_CLIENT_SECRET)
			<< ENCODED_VALUE("code", code)
			<< ENCODED_VALUE("state", state)
			<< ENCODED_VALUE("redirect_uri", SLACK_REDIRECT_URL);
	}
};

class AuthRevokeRequest : public HttpRequest
{
public:
	AuthRevokeRequest(const char *token) :
		HttpRequest(HttpMethod::HttpPost, SLACK_API_URL "/auth.revoke")
	{
		timeout = 1; // in seconds?
		Headers
			<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded;charset=utf-8");
		Content
			<< CHAR_VALUE("token", token);
	}
};

#endif //_SLACK_API_OAUTH_H_