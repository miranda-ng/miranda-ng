#include "common.h"

void CSteamProto::InitQueue()
{
	debugLogA("%s: entering", __FUNCTION__);

	m_evRequestsQueue = CreateEvent(NULL, FALSE, FALSE, NULL);

	debugLogA("%s: leaving", __FUNCTION__);
}

void CSteamProto::UninitQueue()
{
	debugLogA("%s: entering", __FUNCTION__);

	requestsQueue.destroy();
	CloseHandle(m_evRequestsQueue);

	debugLogA("%s: leaving", __FUNCTION__);
}

void CSteamProto::StartQueue()
{
	debugLogA("%s: entering", __FUNCTION__);

	isTerminated = false;

	if (m_hQueueThread == NULL)
	{
		ptrA token(getStringA("TokenSecret"));
		if (token && token[0] != '\0')
		{
			PushRequest(
				new SteamWebApi::LogonRequest(token),
				&CSteamProto::OnLoggedOn);
		}
		else
		{
			ptrA username(mir_urlEncode(ptrA(mir_utf8encodeT(getTStringA("Username")))));
			if (username == NULL || username[0] == '\0')
				return;

			PushRequest(
				new SteamWebApi::RsaKeyRequest(username),
				&CSteamProto::OnGotRsaKey);
		}

		m_hQueueThread = ForkThreadEx(&CSteamProto::QueueThread, 0, NULL);
	}

	debugLogA("%s: leaving", __FUNCTION__);
}

void CSteamProto::StopQueue()
{
	debugLogA("%s: entering", __FUNCTION__);

	isTerminated = true;

	{
		mir_cslock lock(requests_queue_lock);

		debugLogA("%s: requestsQueue contains %d items", __FUNCTION__, requestsQueue.getCount());

		while (requestsQueue.getCount() > 0)
		{
			QueueItem *item = requestsQueue[0];
			requestsQueue.remove(0);

			// QueueItem's destructor properly free request and arg
			delete item;

			debugLogA("%s: removed item from requestsQueue, %d items remaining", __FUNCTION__, requestsQueue.getCount());
		}
	}

	// logoff
	ptrA token(getStringA("TokenSecret"));
	ptrA umqid(getStringA("UMQID"));

	SteamWebApi::HttpRequest *request = new SteamWebApi::LogoffRequest(token, umqid);
	NETLIBHTTPREQUEST *response = request->Send(m_hNetlibUser);
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
	delete request;

	m_hQueueThread = NULL;

	debugLogA("%s: leaving", __FUNCTION__);
}

void CSteamProto::PushRequest(SteamWebApi::HttpRequest *request)
{
	PushRequest(request, NULL, NULL, ARG_NO_FREE);
}

void CSteamProto::PushRequest(SteamWebApi::HttpRequest *request, RESPONSE response)
{
	PushRequest(request, response, NULL, ARG_NO_FREE);
}

void CSteamProto::PushRequest(SteamWebApi::HttpRequest *request, RESPONSE response, void *arg, ARG_FREE_TYPE arg_free_type)
{
	debugLogA("%s: entering", __FUNCTION__);

	// Always prepare QueueItem so we can use it's destructor to free request and arg
	QueueItem *item = new QueueItem(request, response);
	item->arg = arg;
	item->arg_free_type = arg_free_type;

	if (isTerminated)
	{
		debugLogA("%s: leaving (isTerminated)", __FUNCTION__);

		// QueueItem's destructor properly free request and arg
		delete item;
		return;
	}

	{
		mir_cslock lock(requests_queue_lock);
		requestsQueue.insert(item);
	}

	SetEvent(m_evRequestsQueue);

	debugLogA("%s: leaving", __FUNCTION__);
}

void CSteamProto::ExecuteRequest(QueueItem *item)
{
	debugLogA("%s: entering", __FUNCTION__);

	if (isTerminated)
	{
		debugLogA("%s: leaving (isTerminated)", __FUNCTION__);

		// QueueItem's destructor properly free request and arg
		delete item;
		return;
	}

	NETLIBHTTPREQUEST *response = item->request->Send(m_hNetlibUser);

	if (item->responseCallback != NULL)
		(this->*(item->responseCallback))(response, item->arg);

	// todo: add succeed and failed handlers if need
	if (response != NULL/* && response->resultCode != 200*/)
	{
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
	}
	/*else if (requestItem->responseFailedCallback != NULL)
	{
	(this->*(requestItem->responseFailedCallback))(response);
	}*/

	delete item;

	debugLogA("%s: leaving", __FUNCTION__);
}

void CSteamProto::QueueThread(void*)
{
	debugLogA("%s: entering", __FUNCTION__);

	while (!isTerminated)
	{
		WaitForSingleObject(m_evRequestsQueue, 1000);

		while (true)
		{
			QueueItem *item = NULL;

			{
				mir_cslock lock(requests_queue_lock);

				if (requestsQueue.getCount() == 0)
					break;

				item = requestsQueue[0];
				requestsQueue.remove(0);
			}

			if (item != NULL)
				ExecuteRequest(item);
		}
	}

	debugLogA("%s: leaving", __FUNCTION__);
}