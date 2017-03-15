#ifndef _SLACK_API_IM_H_
#define _SLACK_API_IM_H_

class OpenImChannelRequest : public HttpRequest
{
public:
	OpenImChannelRequest(const char *token, const char *user) :
		HttpRequest(HttpMethod::HttpPost, SLACK_API_URL "/im.open")
	{
		Headers
			<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded;charset=utf-8");
		Content
			<< CHAR_VALUE("token", token)
			<< CHAR_VALUE("user", user);
	}
};

#endif //_SLACK_API_IM_H_