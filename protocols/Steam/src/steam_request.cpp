#include "stdafx.h"

class SteamResponseDelegate
{
private:
	CSteamProto *proto;
	SteamResponseCallback responseCallback;
	SteamResponseWithArgCallback responseWithArgCallback;
	HttpFinallyCallback httpFinallyCallback;

	void *arg;
	bool hasArg;

public:
	SteamResponseDelegate(CSteamProto *proto, SteamResponseCallback responseCallback)
		: proto(proto), responseCallback(responseCallback), responseWithArgCallback(NULL), arg(NULL), httpFinallyCallback(NULL), hasArg(false) {}

	SteamResponseDelegate(CSteamProto *proto, SteamResponseWithArgCallback responseCallback, void *arg, HttpFinallyCallback httpFinallyCallback)
		: proto(proto), responseCallback(NULL), responseWithArgCallback(responseCallback), arg(arg), httpFinallyCallback(httpFinallyCallback), hasArg(true) { }

	void Invoke(const NETLIBHTTPREQUEST *response)
	{
		if (hasArg)
		{
			(proto->*(responseWithArgCallback))(response, arg);
			if (httpFinallyCallback != NULL)
				httpFinallyCallback(arg);
		}
		else
			(proto->*(responseCallback))(response);
	}
};

static void SteamHttpResponse(const NETLIBHTTPREQUEST *response, void *arg)
{
	SteamResponseDelegate *delegate = (SteamResponseDelegate*)arg;
	delegate->Invoke(response);
	delete delegate;
}

void CSteamProto::PushRequest(HttpRequest *request)
{
	requestQueue->Push(request);
}

void CSteamProto::PushRequest(HttpRequest *request, SteamResponseCallback response)
{
	SteamResponseDelegate *delegate = new SteamResponseDelegate(this, response);
	requestQueue->Push(request, SteamHttpResponse, delegate);
}

void CSteamProto::PushRequest(HttpRequest *request, SteamResponseWithArgCallback response, void *arg, HttpFinallyCallback last)
{
	SteamResponseDelegate *delegate = new SteamResponseDelegate(this, response, arg, last);
	requestQueue->Push(request, SteamHttpResponse, delegate);
}

void CSteamProto::SendRequest(HttpRequest *request)
{
	requestQueue->Send(request, NULL, NULL);
}

void CSteamProto::SendRequest(HttpRequest *request, SteamResponseCallback response)
{
	SteamResponseDelegate *delegate = new SteamResponseDelegate(this, response);
	requestQueue->Send(request, SteamHttpResponse, delegate);
}

void CSteamProto::SendRequest(HttpRequest *request, SteamResponseWithArgCallback response, void *arg, HttpFinallyCallback last)
{
	SteamResponseDelegate *delegate = new SteamResponseDelegate(this, response, arg, last);
	requestQueue->Send(request, SteamHttpResponse, delegate);
}
