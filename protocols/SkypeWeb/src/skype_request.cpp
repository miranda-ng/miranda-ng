#include "common.h"

class SkypeResponseDelegate
{
private:
	CSkypeProto *proto;
	SkypeResponseCallback responseCallback;
	SkypeResponseWithArgCallback responseWithArgCallback;

	void *arg;
	bool hasArg;

public:
	SkypeResponseDelegate(CSkypeProto *proto, SkypeResponseCallback responseCallback)
		: proto(proto), responseCallback(responseCallback), arg(NULL), hasArg(false) {}

	SkypeResponseDelegate(CSkypeProto *proto, SkypeResponseWithArgCallback responseCallback, void *arg)
		: proto(proto), responseWithArgCallback(responseCallback), arg(arg), hasArg(true) { }

	void Invoke(const NETLIBHTTPREQUEST *response)
	{
		if (hasArg)
			(proto->*(responseWithArgCallback))(response, arg);
		else
			(proto->*(responseCallback))(response);
	}
};

static void SkypeHttpResponse(const NETLIBHTTPREQUEST *response, void *arg)
{
	((SkypeResponseDelegate*)arg)->Invoke(response);
}

void CSkypeProto::PushRequest(HttpRequest *request)
{
	requestQueue->Push(request);
}

void CSkypeProto::PushRequest(HttpRequest *request, SkypeResponseCallback response)
{
	SkypeResponseDelegate *delegate = new SkypeResponseDelegate(this, response);
	requestQueue->Push(request, SkypeHttpResponse, delegate);
}

void CSkypeProto::PushRequest(HttpRequest *request, SkypeResponseWithArgCallback response, void *arg)
{
	SkypeResponseDelegate *delegate = new SkypeResponseDelegate(this, response, arg);
	requestQueue->Push(request, SkypeHttpResponse, delegate);
}