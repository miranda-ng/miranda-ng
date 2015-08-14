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

void CSkypeProto::ParsePollData(void *pData)
{
	debugLogA("CSkypeProto::ParsePollData");

	JSONNode data = JSONNode::parse(ptrA((char*)pData));
	if (!data) return;

	const JSONNode &node = data["eventMessages"];
	if (!node) return;

	for (auto it = node.begin(); it != node.end(); ++it)
	{
		const JSONNode &message = *it;
		const JSONNode &resType = message["resourceType"];
		const JSONNode &resource = message["resource"];

		std::string resourceType = resType.as_string();
		if (resourceType == "NewMessage")
		{
			ProcessNewMessageRes(resource);
		}
		else if (resourceType == "UserPresence")
		{
			ProcessUserPresenceRes(resource);
		}
		else if (resourceType == "EndpointPresence")
		{
			ProcessEndpointPresenceRes(resource);
		}
		else if (resourceType == "ConversationUpdate")
		{
			ProcessConversationUpdateRes(resource);
		}
		else if (resourceType == "ThreadUpdate")
		{
			ProcessThreadUpdateRes(resource);
		}
	}
}

void CSkypeProto::PollingThread(void*)
{
	debugLogA(__FUNCTION__": entering");

	int errors = 0;
	isTerminated = false;
	while (!isTerminated && errors < POLLING_ERRORS_LIMIT)
	{
		PollRequest *request = new PollRequest(m_szRegToken, m_szServer);
		request->nlc = m_pollingConnection;
		NLHR_PTR response(request->Send(m_hNetlibUser));

		if (response == NULL)
		{
			errors++;
			delete request;
			continue;
		}

		if (response->resultCode == 200)
		{
			if (response->pData)
			{
				void *pData = mir_alloc(response->dataLength);
				memcpy(pData, response->pData, response->dataLength);
				ForkThread(&CSkypeProto::ParsePollData, pData);
			}
		}
		else
		{
			errors++;

			if (response->pData)
			{
				JSONNode root = JSONNode::parse(response->pData);
				const JSONNode &error = root["errorCode"];
				if (error != NULL)
				{
					int errorCode = error.as_int();
					if (errorCode == 729)
					{
						delete request;
						break;
					}
				}
			}
		}

		m_pollingConnection = response->nlc;
		delete request;
	}

	if (!isTerminated)
	{
		debugLogA(__FUNCTION__": unexpected termination; switching protocol to offline");
		SetStatus(ID_STATUS_OFFLINE);
	}
	m_hPollingThread = NULL;
	m_pollingConnection = NULL;
	debugLogA(__FUNCTION__": leaving");
}