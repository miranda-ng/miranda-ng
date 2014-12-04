#include "common.h"

#define POLLING_ERRORS_LIMIT 3

void CSteamProto::ParsePollData(JSONNODE *data)
{
	JSONNODE *node, *item = NULL;

	std::string steamIds;
	for (size_t i = 0; i < json_size(data); i++)
	{
		item = json_at(data, i);
		if (item == NULL)
			break;

		node = json_get(item, "steamid_from");
		ptrA steamId(mir_t2a(json_as_string(node)));

		node = json_get(item, "utc_timestamp");
		time_t timestamp = atol(ptrA(mir_t2a(json_as_string(node))));

		node = json_get(item, "type");
		ptrT type(json_as_string(node));
		if (!lstrcmpi(type, _T("saytext")) || !lstrcmpi(type, _T("emote")) ||
			!lstrcmpi(type, _T("my_saytext")) || !lstrcmpi(type, _T("my_emote")))
		{
			node = json_get(item, "text");
			const TCHAR *text = json_as_string(node);

			if (_tcsstr(type, _T("my_")) == NULL)
			{
				MCONTACT hContact = FindContact(steamId);
				if (hContact)
				{
					ptrA szMessage(mir_utf8encodeT(text));

					PROTORECVEVENT recv = { 0 };
					recv.flags = PREF_UTF;
					recv.timestamp = timestamp;
					recv.szMessage = szMessage;

					ProtoChainRecvMsg(hContact, &recv);
				}
			}
			else
			{
				MCONTACT hContact = FindContact(steamId);
				if (hContact)
				{
					ptrA szMessage(mir_utf8encodeT(text));

					AddDBEvent(hContact, EVENTTYPE_MESSAGE, timestamp, DBEF_UTF | DBEF_SENT, mir_strlen(szMessage) + 1, (PBYTE)(char*)szMessage);
				}
			}
		}
		/*else if (!lstrcmpi(type, _T("typing")))
		{
		}*/
		else if (!lstrcmpi(type, _T("personastate")))
		{
			node = json_get(item, "persona_state");
			int status = SteamToMirandaStatus(json_as_int(node));

			if (IsMe(steamId))
			{
				node = json_get(item, "persona_name");
				setTString("Nick", json_as_string(node));

				if (status == ID_STATUS_OFFLINE)
					continue;

				if (status != m_iStatus)
				{
					debugLog(_T("CSteamProto::ParsePollData: Change own status to %i"), status);
					int oldStatus = m_iStatus;
					m_iStatus = m_iDesiredStatus = status;
					ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
				}

				continue;
			}
			
			MCONTACT hContact = FindContact(steamId);
			if (hContact == NULL)
				hContact = AddContact(steamId);

			setWord(hContact, "Status", status);

			node = json_get(item, "persona_name");
			setTString(hContact, "Nick", json_as_string(node));

			// todo: find difference between state changing and info changing
			steamIds.append(steamId).append(",");
		}
		else if (!lstrcmpi(type, _T("personarelationship")))
		{
			node = json_get(item, "persona_state");
			int state = json_as_int(node);

			switch (state)
			{
			case 0:
				{// removed
					MCONTACT hContact = FindContact(steamId);
					if (hContact)
					{
						setByte(hContact, "Auth", 1);

						TCHAR message[MAX_PATH];
						mir_sntprintf(
							message, MAX_PATH,
							TranslateT("%s has been removed from your contact list"),
							ptrT(mir_a2t(steamId)));

						ShowNotification(_T("Steam"), message);
					}
				}
				break;

			case 1:
				// ignored
				// todo
				{
					MCONTACT hContact = FindContact(steamId);
					if (hContact)
					{
						setByte(hContact, "Block", 1);
					}
				}
				break;

			case 2:
				{	// auth request
					/*MCONTACT hContact = FindContact(steamId);
					if (!hContact)
						hContact = AddContact(steamId, true);*/

					//RaiseAuthRequestThread((void*)hContact);

					ptrA token(getStringA("TokenSecret"));

					PushRequest(
						new SteamWebApi::GetUserSummariesRequest(token, steamId),
						&CSteamProto::OnAuthRequested,
						mir_strdup(steamId));
				}
				break;

			case 3:
				// add to list
				// todo
				break;

			default: continue;
			}
		}
		/*else if (!lstrcmpi(type, _T("leftconversation")))
		{
		}*/
		else
		{
			continue;
		}
	}

	if (!steamIds.empty())
	{
		steamIds.pop_back();
		ptrA token(getStringA("TokenSecret"));

		PushRequest(
			new SteamWebApi::GetUserSummariesRequest(token, steamIds.c_str()),
			&CSteamProto::OnGotUserSummaries);
	}
}

void CSteamProto::PollingThread(void*)
{
	debugLog(_T("CSteamProto::PollingThread: entering"));

	ptrA token(getStringA("TokenSecret"));
	ptrA umqId(getStringA("UMQID"));
	UINT32 messageId = getDword("MessageID", 0);

	//SteamWebApi::PollApi::PollResult pollResult;
	int errors = 0;
	bool breaked = false;
	while (!isTerminated && !breaked && errors < POLLING_ERRORS_LIMIT)
	{
		SteamWebApi::PollRequest *request = new SteamWebApi::PollRequest(token, umqId, messageId);
		debugLogA("CSteamProto::PollingThread: %s", request->szUrl);
		request->szUrl = (char*)request->url.c_str();
		request->nlc = m_pollingConnection;
		NETLIBHTTPREQUEST *response = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)request);
		delete request;

		if (response == NULL || response->resultCode != HTTP_STATUS_OK)
		{
			if (response != NULL)
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);

			errors++;
			continue;
		}
		else
			errors = 0;

		JSONROOT root(response->pData);
		JSONNODE *node = json_get(root, "error");
		ptrT error(json_as_string(node));

		if (!lstrcmpi(error, _T("OK")))
		{
			node = json_get(root, "messagelast");
			messageId = json_as_int(node);

			node = json_get(root, "messages");
			JSONNODE *nroot = json_as_array(node);

			if (nroot != NULL)
				ParsePollData(nroot);

			m_pollingConnection = response->nlc;
		}
		else if (!lstrcmpi(error, _T("Timeout")))
		{
			continue;
		}
		/*else if (!lstrcmpi(error, _T("Not Logged On"))) // 'else' below will handle this error, we don't need this particular check right now
		{
			if (!IsOnline())
			{
				// need to relogin
				debugLog(_T("CSteamProto::PollingThread: not logged on"));

				SetStatus(ID_STATUS_OFFLINE);
			}

			breaked = true;
		}*/
		else
		{
			// something wrong
			debugLog(_T("CSteamProto::PollingThread: %s (%d)"), error, response->resultCode);

			// token has expired
			if (response->resultCode == HTTP_STATUS_UNAUTHORIZED)
				delSetting("TokenSecret");

			breaked = true;
		}

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
	}

	setDword("MessageID", messageId);

	m_hPollingThread = NULL;
	debugLog(_T("CSteamProto::PollingThread: leaving"));

	if (!isTerminated)
	{
		debugLog(_T("CSteamProto::PollingThread: unexpected termination; switching protocol to offline"));
		SetStatus(ID_STATUS_OFFLINE);
	}
}