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

void CSkypeProto::StartQueue()
{
	if (!m_isTerminated)
		return;

	m_isTerminated = false;
	if (m_hRequestQueueThread == nullptr)
		m_hRequestQueueThread = ForkThreadEx(&CSkypeProto::WorkerThread, 0, 0);
}

void CSkypeProto::StopQueue()
{
	if (m_isTerminated)
		return;

	m_isTerminated = true;
	m_hRequestQueueEvent.Set();
}

void CSkypeProto::PushRequest(AsyncHttpRequest *request)
{
	if (m_isTerminated)
		return;
	{
		mir_cslock lock(m_requestQueueLock);
		m_requests.insert(request);
	}
	m_hRequestQueueEvent.Set();
}

void CSkypeProto::SendRequest(AsyncHttpRequest *request)
{
	mir_forkthreadowner(&CSkypeProto::AsyncSendThread, this, request, nullptr);
}

void CSkypeProto::Execute(AsyncHttpRequest *item)
{
	NLHR_PTR response(DoSend(item));
	if (item->m_pFunc != nullptr)
		(this->*item->m_pFunc)(response, item);
	m_requests.remove(item);
	delete item;
}

unsigned CSkypeProto::AsyncSendThread(void *owner, void *arg)
{
	CSkypeProto *that = (CSkypeProto*)owner;
	AsyncHttpRequest *item = (AsyncHttpRequest*)arg;

	that->Execute(item);
	return 0;
}

void CSkypeProto::WorkerThread(void*)
{
	while (true) {
		m_hRequestQueueEvent.Wait();
		if (m_isTerminated)
			break;

		while (true) {
			AsyncHttpRequest *item = nullptr;
			{
				mir_cslock lock(m_requestQueueLock);

				if (m_requests.getCount() == 0)
					break;

				item = m_requests[0];
				m_requests.remove(0);
			}
			if (item != nullptr)
				Execute(item);
		}
	}

	m_hRequestQueueThread = nullptr;
}
