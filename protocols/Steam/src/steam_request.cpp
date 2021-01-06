#include "stdafx.h"

void CSteamProto::SendRequest(HttpRequest *request)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, (NETLIBHTTPREQUEST*)request);
	HttpResponse response(request, pResp);
	delete request;
}

void CSteamProto::SendRequest(HttpRequest *request, HttpCallback callback, void *param)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, (NETLIBHTTPREQUEST*)request);
	HttpResponse response(request, pResp);
	if (callback)
		(this->*callback)(response, param);
	delete request;
}

void CSteamProto::SendRequest(HttpRequest *request, JsonCallback callback, void *param)
{
	NETLIBHTTPREQUEST *pResp = Netlib_HttpTransaction(m_hNetlibUser, (NETLIBHTTPREQUEST*)request);
	HttpResponse response(request, pResp);
	if (callback) {
		JSONNode root = JSONNode::parse(response.Content);
		(this->*callback)(root, param);
	}
	delete request;
}

void CSteamProto::PushRequest(HttpRequest *request)
{
	RequestQueueItem *item = new RequestQueueItem(request);
	PushToRequestQueue(item);
}

void CSteamProto::PushRequest(HttpRequest *request, HttpCallback callback, void *param)
{
	RequestQueueItem *item = new RequestQueueItem(request, callback, param);
	PushToRequestQueue(item);
}

void CSteamProto::PushRequest(HttpRequest *request, JsonCallback callback, void *param)
{
	RequestQueueItem *item = new RequestQueueItem(request, callback, param);
	PushToRequestQueue(item);
}

/////////////////////////////////////////////////////////////////////////////////

void CSteamProto::PushToRequestQueue(RequestQueueItem *item)
{
	if (m_isTerminated)
		return;
	{
		mir_cslock lock(m_requestQueueLock);
		m_requestQueue.insert(item);
	}
	SetEvent(m_hRequestsQueueEvent);
}

RequestQueueItem *CSteamProto::PopFromRequestQueue()
{
	mir_cslock lock(m_requestQueueLock);
	if (!m_requestQueue.getCount())
		return nullptr;

	RequestQueueItem *item = m_requestQueue[0];
	m_requestQueue.remove(0);
	return item;
}

void CSteamProto::ProcessRequestQueue()
{
	while (true) {
		RequestQueueItem *item = PopFromRequestQueue();
		if (item == nullptr)
			break;
		if (item->httpCallback)
			SendRequest(item->request, item->httpCallback, item->param);
		else if (item->jsonCallback)
			SendRequest(item->request, item->jsonCallback, item->param);
		else
			SendRequest(item->request);
		delete item;
	}
}

void CSteamProto::RequestQueueThread(void*)
{
	do {
		ProcessRequestQueue();
		WaitForSingleObject(m_hRequestsQueueEvent, 1000);
	} while (!m_isTerminated);

	m_hRequestQueueThread = nullptr;

	mir_cslock lock(m_requestQueueLock);
	for (auto &it : m_requestQueue)
		delete it;
	m_requestQueue.destroy();
}
