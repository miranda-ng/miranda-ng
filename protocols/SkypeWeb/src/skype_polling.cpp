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

#define POLLING_ERRORS_LIMIT 3

void CSkypeProto::ParsePollData(JSONNODE *data)
{
	debugLogA("CSkypeProto::ParsePollData");
	JSONNODE *node = json_get(data, "eventMessages");
	if (node != NULL)
	{
		JSONNODE *messages = json_as_array(node);
		for (size_t i = 0; i < json_size(messages); i++)
		{
			JSONNODE *message = json_at(messages, i);
			JSONNODE *resType = json_get(message, "resourceType");
			TCHAR *resourceType = json_as_string(resType);
			JSONNODE *resource = json_get(message, "resource");

			if (!mir_tstrcmpi(resourceType, L"NewMessage"))
			{
				ProcessNewMessageRes(resource);
			}
			else if (!mir_tstrcmpi(resourceType, L"UserPresence"))
			{
				ProcessUserPresenceRes(resource);
			}
			else if (!mir_tstrcmpi(resourceType, L"EndpointPresence"))
			{
				ProcessEndpointPresenceRes(resource);
			}
			else if (!mir_tstrcmpi(resourceType, L"ConversationUpdate"))
			{
				ProcessConversationUpdateRes(resource);
			} 
			else if (!mir_tstrcmpi(resourceType, L"ThreadUpdate"))
			{
				ProcessThreadUpdateRes(resource);
			}
		}
	}
}

void CSkypeProto::PollingThread(void*)
{
	debugLogA(__FUNCTION__": entering");

	ptrA regToken(getStringA("registrationToken"));
	ptrA server(getStringA("Server"));

	int errors = 0;
	isTerminated = false;
	while (!isTerminated && errors < POLLING_ERRORS_LIMIT)
	{
		PollRequest *request = new PollRequest(regToken, server);
		request->nlc = m_pollingConnection;
		NETLIBHTTPREQUEST *response = request->Send(m_hNetlibUser);

		if (response)
		{
			if (response->pData)
			{
				JSONROOT root(response->pData);
				if (json_get(root, "errorCode") != NULL)
				{
					errors++;
					continue;
				}
				ParsePollData(root);
			}
			m_pollingConnection = response->nlc;
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
		}

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