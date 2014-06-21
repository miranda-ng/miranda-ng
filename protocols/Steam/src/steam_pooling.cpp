#include "common.h"

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
		ptrA steamId(mir_u2a(json_as_string(node)));

		node = json_get(item, "utc_timestamp");
		time_t timestamp = atol(ptrA(mir_u2a(json_as_string(node))));

		node = json_get(item, "type");
		ptrW type(json_as_string(node));
		if (!lstrcmpi(type, L"saytext") || !lstrcmpi(type, L"emote") ||
			!lstrcmpi(type, L"my_saytext") || !lstrcmpi(type, L"my_emote"))
		{
			node = json_get(item, "text");
			const wchar_t *text = json_as_string(node);

			if (_tcsstr(type, L"my_") == NULL)
			{
				MCONTACT hContact = FindContact(steamId);
				if (hContact)
				{
					PROTORECVEVENT recv = { 0 };
					recv.flags = PREF_UTF;
					recv.timestamp = timestamp;
					recv.szMessage = mir_utf8encodeW(text);

					ProtoChainRecvMsg(hContact, &recv);
				}
			}
			else
			{
				MCONTACT hContact = FindContact(steamId);
				if (hContact)
					AddDBEvent(hContact, EVENTTYPE_MESSAGE, timestamp, DBEF_UTF | DBEF_SENT, lstrlen(text), (BYTE*)mir_utf8encodeW(text));
			}
		}
		/*else if (!lstrcmpi(type, L"typing"))
		{
		}*/
		else if (!lstrcmpi(type, L"personastate"))
		{
			node = json_get(item, "persona_state");
			int status = SteamToMirandaStatus(json_as_int(node));

			if (IsMe(steamId))
			{
				node = json_get(item, "persona_name");
				setWString("Nick", json_as_string(node));

				if (status == ID_STATUS_OFFLINE)
					continue;

				if (status != m_iStatus)
				{
					debugLogA("CSteamProto::ParsePollData: Change own status to %i", status);
					WORD oldStatus = m_iStatus;
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
			setWString(hContact, "Nick", json_as_string(node));

			// todo: find difference between state changing and info changing
			steamIds.append(steamId).append(",");
		}
		else if (!lstrcmpi(type, L"personarelationship"))
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

						wchar_t message[MAX_PATH];
						mir_sntprintf(
							message, MAX_PATH,
							TranslateT("%s has been removed from your contact list"),
							ptrW(mir_a2u(steamId)));

						ShowNotification(L"Steam", message);
					}
				}
				break;

			case 1:
				// ignored
				// todo
				MCONTACT hContact = FindContact(steamId);
				if (hContact)
				{
					setByte(hContact, "Block", 1);
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
		/*else if (!lstrcmpi(type, L"leftconversation"))
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
	debugLogA("CSteamProto::PollingThread: entering");

	ptrA token(getStringA("TokenSecret"));
	ptrA umqId(getStringA("UMQID"));
	UINT32 messageId = getDword("MessageID", 0);

	//SteamWebApi::PollApi::PollResult pollResult;
	bool breaked = false;
	while (!isTerminated && !breaked)
	{
		SteamWebApi::PollRequest *request = new SteamWebApi::PollRequest(token, umqId, messageId);
		debugLogA("CSteamProto::PollingThread: %s", request->szUrl);
		request->szUrl = (char*)request->url.c_str();
		request->nlc = m_pollingConnection;
		NETLIBHTTPREQUEST *response = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)request);
		delete request;

		if (response == NULL || response->resultCode != HTTP_STATUS_OK)
			return;

		JSONNODE *root = json_parse(response->pData), *node;
		node = json_get(root, "error");
		ptrW error(json_as_string(node));

		if (!lstrcmpi(error, L"OK"))
		{
			node = json_get(root, "messagelast");
			messageId = json_as_int(node);

			node = json_get(root, "messages");
			root = json_as_array(node);

			if (root != NULL)
				ParsePollData(root);

			m_pollingConnection = response->nlc;
		}
		else if (!lstrcmpi(error, L"Timeout"))
		{
			continue;
		}
		else if (!lstrcmpi(error, L"Not Logged On"))
		{
			if (!IsOnline())
			{
				// need to relogin
				debugLogA("CSteamProto::PollingThread: not logged on");

				SetStatus(ID_STATUS_OFFLINE);
			}

			breaked = true;
		}
		else if (lstrcmpi(error, L"Timeout"))
		{
			// something wrong
			debugLogA("CSteamProto::PollingThread: error (%d)", response->resultCode);

			// token has expired
			if (response->resultCode == HTTP_STATUS_UNAUTHORIZED)
				delSetting("TokenSecret");

			breaked = true;
		}

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)response);
	}

	setDword("MessageID", messageId);

	m_hPollingThread = NULL;
	debugLogA("CSteamProto::PollingThread: leaving");
}