/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

void __cdecl CMTProto::ServerThread(void *)
{
	m_bRunning = true;
	m_bTerminated = m_bAuthorized = false;

	while (!m_bTerminated) {
		ProcessResponse(m_pClientMmanager->receive(10));
	}

	m_bRunning = false;
}

///////////////////////////////////////////////////////////////////////////////

void CMTProto::ProcessResponse(td::ClientManager::Response response)
{
	if (!response.object)
		return;

	if (response.client_id) {
		auto *p = m_arRequests.find((TG_REQUEST *)&response.client_id);
		if (p) {
			(this->*p->pHandler)(response);
			m_arRequests.remove(p);
		}
	}
}

void CMTProto::SendQuery(td::td_api::Function *pFunc, TG_QUERY_HANDLER pHandler)
{
	int queryId = ++m_iQueryId;
	m_pClientMmanager->send(m_iClientId, queryId, td::td_api::object_ptr<td::td_api::Function>(pFunc));

	if (pHandler)
		m_arRequests.insert(new TG_REQUEST(queryId, pHandler));
}
