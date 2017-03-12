#ifndef _SLACK_API_USERS_H_
#define _SLACK_API_USERS_H_

class GetUserListRequest : public HttpRequest
{
public:
	GetUserListRequest(const char *token) :
		HttpRequest(HttpMethod::HttpPost, SLACK_API_URL "/users.list")
	{
		Headers
			<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded;charset=utf-8");
		Content
			<< CHAR_VALUE("token", token)
			<< CHAR_VALUE("presence", "true");
	}
};

#endif //_SLACK_API_USERS_H_