#include "stdafx.h"

void CSlackProto::SendRequest(HttpRequest *request)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, (NETLIBHTTPREQUEST*)request);
	HttpResponse response(request, pResp);
	delete request;
}

void CSlackProto::SendRequest(HttpRequest *request, HttpCallback callback, void *param)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, (NETLIBHTTPREQUEST*)request);
	HttpResponse response(request, pResp);
	if (callback)
		(this->*callback)(response, param);
	delete request;
}

void CSlackProto::SendRequest(HttpRequest *request, JsonCallback callback, void *param)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, (NETLIBHTTPREQUEST*)request);
	HttpResponse response(request, pResp);
	if (callback)
	{
		JSONNode root = JSONNode::parse(response.Content);
		(this->*callback)(root, param);
	}
	delete request;
}

void CSlackProto::PushRequest(HttpRequest *request)
{
	RequestQueueItem *item = new RequestQueueItem();
	item->request = request;
	{
		mir_cslock lock(requestQueueLock);
		requestQueue.insert(item);
	}
	SetEvent(hRequestsQueueEvent);
}

void CSlackProto::PushRequest(HttpRequest *request, HttpCallback callback, void *param)
{
	RequestQueueItem *item = new RequestQueueItem();
	item->request = request;
	item->httpCallback = callback;
	item->param = param;
	{
		mir_cslock lock(requestQueueLock);
		requestQueue.insert(item);
	}
	SetEvent(hRequestsQueueEvent);
}

void CSlackProto::PushRequest(HttpRequest *request, JsonCallback callback, void *param)
{
	RequestQueueItem *item = new RequestQueueItem();
	item->request = request;
	item->jsonCallback = callback;
	item->param = param;
	{
		mir_cslock lock(requestQueueLock);
		requestQueue.insert(item);
	}
	SetEvent(hRequestsQueueEvent);
}

void CSlackProto::RequestQueueThread(void*)
{
	Login();

	do
	{
		RequestQueueItem *item;
		while (true)
		{
			{
				mir_cslock lock(requestQueueLock);
				if (!requestQueue.getCount())
					break;

				item = requestQueue[0];
				requestQueue.remove(0);
			}
			if (item->httpCallback)
				SendRequest(item->request, item->httpCallback, item->param);
			else if (item->jsonCallback)
				SendRequest(item->request, item->jsonCallback, item->param);
			else
				SendRequest(item->request);
			delete item;
		}
		WaitForSingleObject(hRequestsQueueEvent, 1000);
	} while (!isTerminated);

	hRequestQueueThread = NULL;
}