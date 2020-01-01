/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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

RequestQueue::RequestQueue(HNETLIBUSER _nlu) :
	nlu(_nlu), requests(1)
{
	isTerminated = true;
	hRequestQueueThread = nullptr;
}

RequestQueue::~RequestQueue()
{
	if (hRequestQueueThread) {
		WaitForSingleObject(hRequestQueueThread, INFINITE);
		hRequestQueueThread = nullptr;
	}

	requests.destroy();
}

void RequestQueue::Start()
{
	if (!isTerminated)
		return;

	isTerminated = false;
	if (hRequestQueueThread == nullptr)
		hRequestQueueThread = mir_forkThread<RequestQueue>(&RequestQueue::WorkerThread, this);
}

void RequestQueue::Stop()
{
	if (isTerminated)
		return;

	isTerminated = true;
	hRequestQueueEvent.Set();
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
	hRequestQueueEvent.Set();
}

void RequestQueue::Send(HttpRequest *request, HttpResponseCallback response, void *arg)
{
	RequestQueueItem *item = new RequestQueueItem(request, response, arg);
	mir_forkthreadowner(&RequestQueue::AsyncSendThread, this, item, nullptr);
}

void RequestQueue::Execute(RequestQueueItem *item)
{
	NLHR_PTR response(item->request->Send(nlu));
	if (item->responseCallback != nullptr)
		item->responseCallback(response, item->arg);
	requests.remove(item);
	delete item;
}

unsigned RequestQueue::AsyncSendThread(void *owner, void *arg)
{
	RequestQueue *that = (RequestQueue*)owner;
	RequestQueueItem *item = (RequestQueueItem*)arg;

	that->Execute(item);
	return 0;
}

void RequestQueue::WorkerThread(RequestQueue *queue)
{
	while (true) {
		queue->hRequestQueueEvent.Wait();
		if (queue->isTerminated)
			break;

		while (true) {
			RequestQueueItem *item = nullptr;
			{
				mir_cslock lock(queue->requestQueueLock);

				if (queue->requests.getCount() == 0)
					break;

				item = queue->requests[0];
				queue->requests.remove(0);
			}
			if (item != nullptr)
				queue->Execute(item);
		}
	}

	queue->hRequestQueueThread = nullptr;
}
