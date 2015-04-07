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
	bool breaked = false;
	isTerminated = false;
	while (!isTerminated && !breaked && errors < POLLING_ERRORS_LIMIT)
	{
		PollRequest *request = new PollRequest(regToken);
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
	m_hPollingThread = NULL;
	m_pollingConnection = NULL;
	debugLogA(__FUNCTION__": leaving");

	if (!isTerminated)
	{
		debugLogA(__FUNCTION__": unexpected termination; switching protocol to offline");
		SetStatus(ID_STATUS_OFFLINE);
	}
}