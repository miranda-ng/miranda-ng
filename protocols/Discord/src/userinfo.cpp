/*
Copyright © 2016-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void CDiscordProto::OnReceiveProfile(MHttpResponse *response, AsyncHttpRequest *pReq)
{
	JsonReply reply(response);
	if (reply) {
		auto &user = reply.data()["user"];
		CMStringW wszBio = user["bio"].as_mstring();
		if (wszBio.IsEmpty())
			delSetting(pReq->hContact, "About");
		else
			setWString(pReq->hContact, "About", wszBio);

		ProtoBroadcastAck(pReq->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, 0);
	}
	else ProtoBroadcastAck(pReq->hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, 0);
}

int CDiscordProto::GetInfo(MCONTACT hContact, int /*infoType*/)
{
	SnowFlake id = getId(hContact, DB_KEY_ID);
	if (id != 0) {
		CMStringA szUrl(FORMAT, "/users/%lld/profile?type=modal&with_mutual_guilds=true&with_mutual_friends=true", id);
		auto *pReq = new AsyncHttpRequest(this, REQUEST_GET, szUrl, &CDiscordProto::OnReceiveProfile);
		pReq->hContact = hContact;
		Push(pReq);
	}
	return 0;
}
