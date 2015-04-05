#include "common.h"

#define POLLING_ERRORS_LIMIT 3

void CSkypeProto::ParsePollData(JSONNODE *data)
{
	debugLogA("CSkypeProto::ParsePollData");
	JSONNODE *node;
	node = json_get(data, "eventMessages");
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
				continue;
			} 
			else if (!mir_tstrcmpi(resourceType, L"ThreadUpdate"))
			{
				continue;
			}
		}
	}
}

void CSkypeProto::PollingThread(void*)
{
	debugLogA(__FUNCTION__": entering");

	ptrA regToken(getStringA("registrationToken"));
	ptrA server(getStringA("Server"));

	SubscriptionsRequest *request = new SubscriptionsRequest(regToken, server);
	request->Send(m_hNetlibUser);
	delete request;

	int errors = 0;
	bool breaked = false;
	while (!isTerminated && !breaked && errors < POLLING_ERRORS_LIMIT)
	{
		PollRequest *request = new PollRequest(regToken, server);
			NETLIBHTTPREQUEST *response = request->Send(m_hNetlibUser);
			m_pollingConnection = request->nlc;
			delete request;
			

		if (response != NULL)
		{
			JSONROOT root(response->pData);
			if (json_get(root, "errorCode") != NULL)
			{
				errors++;
				continue;
			}
			ParsePollData (root);
		}
		/*if (response->resultCode != 200)
		{
			errors++;
			continue;
		}
		else 
			errors = 0;*/
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