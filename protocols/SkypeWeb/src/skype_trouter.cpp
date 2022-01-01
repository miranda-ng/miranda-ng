/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

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

void CSkypeProto::ProcessTimer()
{
	if (!IsOnline())
		return;

	PushRequest(new GetContactListRequest(this, nullptr));
	SendPresence();
}

void CSkypeProto::OnReceiveStatus(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
{
	JsonReply reply(response);
	if (reply.error())
		return;

	auto &root = reply.data();
	for (auto &it : root["Responses"]) {
		std::string id = it["Contact"].as_string();

		MCONTACT hContact = AddContact(id.c_str(), nullptr);
		if (hContact) {
			int status = SkypeToMirandaStatus(it["Payload"]["status"].as_string().c_str());
			setWord(hContact, "Status", status);
		}
	}
}

void CSkypeProto::RefreshStatuses(void)
{
	int nRecs = 0;
	GetStatusRequest *pReq = nullptr;

	for (auto &it : AccContacts()) {
		CMStringA id(getId(it));
		if (id.IsEmpty())
			continue;

		if (pReq == nullptr) {
			pReq = new GetStatusRequest();
			nRecs = 0;
		}

		pReq << CHAR_PARAM("cMri", id);
		nRecs++;

		if (nRecs >= 10) {
			PushRequest(pReq);
			pReq = nullptr;
		}
	}

	if (pReq)
		PushRequest(pReq);
}
