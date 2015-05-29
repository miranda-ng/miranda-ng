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

void CSkypeProto::ParsePollData(const JSONNode &data)
{
	debugLogA("CSkypeProto::ParsePollData");

	const JSONNode &node = data["eventMessages"];
	if (!node)
		return;

	const JSONNode &messages = node.as_array();
	for (size_t i = 0; i < messages.size(); i++)
	{
		const JSONNode &message = messages.at(i);
		const JSONNode &resType = message["resourceType"];
		const JSONNode &resource = message["resource"];

		std::string resourceType = resType.as_string();
		if (!mir_strcmpi(resourceType.c_str(), "NewMessage"))
		{
			ProcessNewMessageRes(resource);
		}
		else if (!mir_strcmpi(resourceType.c_str(), "UserPresence"))
		{
			ProcessUserPresenceRes(resource);
		}
		else if (!mir_strcmpi(resourceType.c_str(), "EndpointPresence"))
		{
			ProcessEndpointPresenceRes(resource);
		}
		else if (!mir_strcmpi(resourceType.c_str(), "ConversationUpdate"))
		{
			ProcessConversationUpdateRes(resource);
		}
		else if (!mir_strcmpi(resourceType.c_str(), "ThreadUpdate"))
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
		PollRequest *request = new PollRequest(RegToken, Server);
		request->nlc = m_pollingConnection;
		NETLIBHTTPREQUEST *response = request->Send(m_hNetlibUser);

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
				JSONNode root = JSONNode::parse(response->pData);
				if (root["eventMessages"])
				{
					ParsePollData(root);
				}
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
						SendRequest(new CreateEndpointRequest(TokenSecret), &CSkypeProto::OnEndpointCreated);
						CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
						delete request;
						break;
					}
				}
			}
		}

		m_pollingConnection = response->nlc;
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
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