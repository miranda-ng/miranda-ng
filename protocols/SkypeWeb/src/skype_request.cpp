#include "common.h"

class SkypeResponseDelegate
{
private:
	CSkypeProto *proto;
	SkypeResponseCallback responseCallback;

public:
	SkypeResponseDelegate(CSkypeProto *proto, SkypeResponseCallback responseCallback)
		: proto(proto), responseCallback(responseCallback) { }

	void Invoke(const NETLIBHTTPREQUEST *response)
	{
		(proto->*(responseCallback))(response);
	}
};

static void SkypeHttpResponse(const NETLIBHTTPREQUEST *response, void *arg)
{
	((SkypeResponseDelegate*)arg)->Invoke(response);
}

void CSkypeProto::PushRequest(HttpRequest *request, SkypeResponseCallback response)
{
	if (!cookies.empty())
	{
		CMStringA allCookies;
		for (std::map<std::string, std::string>::iterator cookie = cookies.begin(); cookie != cookies.end(); ++cookie)
			allCookies.AppendFormat("%s=%s; ", cookie->first.c_str(), cookie->second.c_str());
		request->SetCookie(allCookies);
	}

	if (response == NULL)
	{
		requestQueue->Push(request);
		return;
	}

	SkypeResponseDelegate *delegate = new SkypeResponseDelegate(this, response);
	requestQueue->Push(request, SkypeHttpResponse, delegate);
}