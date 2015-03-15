#include "common.h"

void CSteamProto::InitQueue()
{
	InitializeCriticalSection(&requests_queue_lock);
	m_evRequestsQueue = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void CSteamProto::UninitQueue()
{
	requestsQueue.destroy();
	CloseHandle(m_evRequestsQueue);
	DeleteCriticalSection(&requests_queue_lock);
}

void CSteamProto::StartQueue()
{
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
}

void CSteamProto::StopQueue()
{
	isTerminated = true;

	{
		mir_cslock lock(requests_queue_lock);

		while (requestsQueue.getCount() > 0)
		{
			QueueItem *item = requestsQueue[0];
			requestsQueue.remove(0);

			// QueueItem's destructor properly free request and arg
			delete item;
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
	// Always prepare QueueItem so we can use it's destructor to free request and arg
	QueueItem *item = new QueueItem(request, response);
	item->arg = arg;
	item->arg_free_type = arg_free_type;

	if (isTerminated)
	{
		// QueueItem's destructor properly free request and arg
		delete item;
		return;
	}

	{
		mir_cslock lock(requests_queue_lock);
		requestsQueue.insert(item);
	}

	SetEvent(m_evRequestsQueue);
}

void CSteamProto::ExecuteRequest(QueueItem *item)
{
	if (isTerminated)
	{
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
}

void CSteamProto::QueueThread(void*)
{
	debugLog(_T("CSteamProto::QueueThread: entering"));

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

	debugLog(_T("CSteamProto::QueueThread: leaving"));
}