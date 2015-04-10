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

#include "common.h"

RequestQueue::RequestQueue(HANDLE hConnection) : 
	hConnection(hConnection), requests(1)
{
	isTerminated = false;
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

	if (hRequestQueueThread == NULL)
		hRequestQueueThread = mir_forkthread((pThreadFunc)&RequestQueue::WorkerThread, this);
}

void RequestQueue::Stop()
{
	if (isTerminated)
		return;

	isTerminated = true;
	hRequestQueueThread = NULL;
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

void RequestQueue::Execute(RequestQueueItem *item)
{
	NETLIBHTTPREQUEST *response = item->request->Send(hConnection);
	if (item->responseCallback != NULL)
		item->responseCallback(response, item->arg);
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
	requests.remove(item);
	delete item;
}

unsigned int RequestQueue::WorkerThread(void* owner, void*)
{
	RequestQueue *that = (RequestQueue*)owner;

	while (!that->isTerminated)
	{
		WaitForSingleObject(that->hRequestQueueEvent, INFINITE);
		while (true)
		{
			RequestQueueItem *item = NULL;
			{
				mir_cslock lock(that->requestQueueLock);

				if (that->requests.getCount() == 0)
					break;

				item = that->requests[0];
				that->requests.remove(0);
			}
			if (item != NULL)
				that->Execute(item);
		}
	}

	return 0;
}