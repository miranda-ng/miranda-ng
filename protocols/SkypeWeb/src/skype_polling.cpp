#include "common.h"

#define POLLING_ERRORS_LIMIT 3

void CSkypeProto::ParsePollData(JSONNODE *data)
{
	debugLogA("CSkypeProto::ParsePollData");
	JSONNODE *node, *item = NULL;
	node = json_get(data, "eventMessages");
	if (node != NULL)
	{
		int index, length;
		JSONNODE *messages = json_as_array(node);
		for (int i = 0; i < json_size(messages); i++)
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
	debugLog(_T("CSkypeProto::PollingThread: entering"));

	ptrA regToken(getStringA("registrationToken"));
	const char *server = getStringA("Server");

	SubscriptionsRequest *request = new SubscriptionsRequest(regToken, server);
		request->Send(m_hNetlibUser);
		delete request;

	int errors = 0;
	bool breaked = false;
	while (!isTerminated && !breaked && errors < POLLING_ERRORS_LIMIT)
	{
		PollRequest *request = new PollRequest(regToken, server);
			NETLIBHTTPREQUEST *response = request->Send(m_hNetlibUser);
			delete request;

		if (response != NULL)
		{
			m_pollingConnection = response->nlc;
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
	debugLog(_T("CSkypeProto::PollingThread: leaving"));

	if (!isTerminated)
	{
		debugLog(_T("CSkypeProto::PollingThread: unexpected termination; switching protocol to offline"));
		SetStatus(ID_STATUS_OFFLINE);
	}
}