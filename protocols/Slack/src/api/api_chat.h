#ifndef _SLACK_API_CHAT_H_
#define _SLACK_API_CHAT_H_

class SendMessageRequest : public HttpRequest
{
public:
	SendMessageRequest(const char *token, const char *channel, const char *message) :
		HttpRequest(HttpMethod::HttpPost, SLACK_API_URL "/chat.postMessage")
	{
		Headers
			<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded;charset=utf-8");
		Content
			<< CHAR_VALUE("token", token)
			<< CHAR_VALUE("channel", channel)
			<< CHAR_VALUE("text", message);
	}
};

class SendActionRequest : public HttpRequest
{
public:
	SendActionRequest(const char *token, const char *channel, const char *message) :
		HttpRequest(HttpMethod::HttpPost, SLACK_API_URL "/chat.meMessage")
	{
		Headers
			<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded;charset=utf-8");
		Content
			<< CHAR_VALUE("token", token)
			<< CHAR_VALUE("channel", channel)
			<< CHAR_VALUE("text", message);
	}
};

#endif //_SLACK_API_CHAT_H_