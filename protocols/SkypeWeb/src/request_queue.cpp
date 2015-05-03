/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

RequestQueue::RequestQueue(HANDLE hConnection) : 
	hConnection(hConnection), requests(1)
{
	isTerminated = true;
	hRequestQueueThread = NULL;
	hRequestQueueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

RequestQueue::~RequestQueue()
{
	requests.destroy();
	CloseHandle(hRequestQueueEvent);
}

void RequestQueue::Start()
{
	if (!isTerminated)
		return;

	isTerminated = false;
	if (hRequestQueueThread == NULL)
		hRequestQueueThread = mir_forkthread((pThreadFunc)&RequestQueue::WorkerThread, this);
}

void RequestQueue::Stop()
{
	if (isTerminated)
		return;

	isTerminated = true;
	SetEvent(hRequestQueueEvent);
}

void RequestQueue::Push(HttpRequest *request, HttpResponseCallback response, void *arg)
{
	if (isTerminated)
		return;

	RequestQueueItem *item = new RequestQueueItem(request, response, arg);
	{
		mir_cslock lock(requestQueueLock);

		requests.insert(item);
	}
	SetEvent(hRequestQueueEvent);
}

void RequestQueue::Send(HttpRequest *request, HttpResponseCallback response, void *arg)
{
	RequestQueueItem *item = new RequestQueueItem(request, response, arg);
	mir_forkthreadowner((pThreadFuncOwner)&RequestQueue::AsyncSendThread, this, item, 0);
}

void RequestQueue::Execute(RequestQueueItem *item)
{
	NETLIBHTTPREQUEST *response = item->request->Send(hConnection);
	if (item->responseCallback != NULL)
		item->responseCallback(response, item->arg);
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
	requests.remove(item);
	delete item;
}

unsigned int RequestQueue::AsyncSendThread(void *owner, void *arg)
{
	RequestQueue *that = (RequestQueue*)owner;
	RequestQueueItem *item = (RequestQueueItem*)arg;

	that->Execute(item);

	return 0;
}

unsigned int RequestQueue::WorkerThread(void *arg)
{
	RequestQueue *queue = (RequestQueue*)arg;

	while (!queue->isTerminated)
	{
		WaitForSingleObject(queue->hRequestQueueEvent, INFINITE);
		while (true)
		{
			RequestQueueItem *item = NULL;
			{
				mir_cslock lock(queue->requestQueueLock);

				if (queue->requests.getCount() == 0)
					break;

				item = queue->requests[0];
				queue->requests.remove(0);
			}
			if (item != NULL)
				queue->Execute(item);
		}
	}

	queue->hRequestQueueThread = NULL;
	return 0;
}