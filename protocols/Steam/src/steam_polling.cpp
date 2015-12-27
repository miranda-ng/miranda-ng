#include "stdafx.h"

#define POLLING_ERRORS_LIMIT 5

void CSteamProto::ParsePollData(JSONNode *data)
{
	JSONNode *node, *item = NULL;

	std::string steamIds;
	for (size_t i = 0; i < json_size(data); i++)
	{
		item = json_at(data, i);
		if (item == NULL)
			break;

		node = json_get(item, "steamid_from");
		ptrA steamId(mir_t2a(ptrT(json_as_string(node))));

		node = json_get(item, "utc_timestamp");
		time_t timestamp = atol(ptrA(mir_t2a(ptrT(json_as_string(node)))));

		node = json_get(item, "type");
		ptrT type(json_as_string(node));
		if (!lstrcmpi(type, _T("saytext")) || !lstrcmpi(type, _T("emote")) ||
			!lstrcmpi(type, _T("my_saytext")) || !lstrcmpi(type, _T("my_emote")))
		{
			MCONTACT hContact = FindContact(steamId);
			if (!hContact)
				continue;

			node = json_get(item, "text");
			ptrT text(json_as_string(node));
			T2Utf szMessage(text);

			PROTORECVEVENT recv = { 0 };
			recv.timestamp = timestamp;
			recv.szMessage = szMessage;
			if (_tcsstr(type, _T("my_")) == NULL)
			{
				ProtoChainRecvMsg(hContact, &recv);
			}
			else
			{
				recv.flags = PREF_SENT;
				Proto_RecvMessage(hContact, &recv);
			}
		}
		else if (!lstrcmpi(type, _T("typing")))
		{
			MCONTACT hContact = FindContact(steamId);
			if (hContact)
			{
				CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)STEAM_TYPING_TIME);
			}
		}
		else if (!lstrcmpi(type, _T("personastate")))
		{
			node = json_get(item, "persona_state");
			int status = node ? SteamToMirandaStatus(json_as_int(node)) : -1;

			if (IsMe(steamId))
			{
				node = json_get(item, "persona_name");
				setTString("Nick", ptrT(json_as_string(node)));

				if (status == -1 || status == ID_STATUS_OFFLINE)
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
				continue; // probably this is info about random player playing on same server, so we ignore it

			if (status != -1)
				SetContactStatus(hContact, status);

			node = json_get(item, "persona_name");
			setTString(hContact, "Nick", ptrT(json_as_string(node)));

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
						ContactIsRemoved(hContact);
					}
				}
				break;

			case 1:
				{// ignored
					MCONTACT hContact = FindContact(steamId);
					if (hContact)
					{
						ContactIsIgnored(hContact);
					}
				}
				break;

			case 2:
				{// auth request
					/*MCONTACT hContact = FindContact(steamId);
					if (!hContact)
						hContact = AddContact(steamId, true);*/

					//RaiseAuthRequestThread((void*)hContact);

					ptrA token(getStringA("TokenSecret"));

					PushRequest(
						new GetUserSummariesRequest(token, steamId),
						&CSteamProto::OnAuthRequested,
						mir_strdup(steamId),
						MirFreeArg);
				}
				break;

			case 3:
				// add to list
				// todo
				break;

			default: continue;
			}
		}
		else if (!lstrcmpi(type, _T("leftconversation")))
		{
			// chatstates gone event
			MCONTACT hContact = FindContact(steamId);
			if (hContact)
			{
				BYTE bEventType = STEAM_DB_EVENT_CHATSTATES_GONE;
				DBEVENTINFO dbei = { sizeof(dbei) };
				dbei.pBlob = &bEventType;
				dbei.cbBlob = 1;
				dbei.eventType = EVENTTYPE_STEAM_CHATSTATES;
				dbei.flags = DBEF_READ;
				dbei.timestamp = time(NULL);
				dbei.szModule = m_szModuleName;
				db_event_add(hContact, &dbei);
			}
		}
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
			new GetUserSummariesRequest(token, steamIds.c_str()),
			&CSteamProto::OnGotUserSummaries);
	}
}

void CSteamProto::PollingThread(void*)
{
	debugLog(_T("CSteamProto::PollingThread: entering"));

	ptrA token(getStringA("TokenSecret"));
	ptrA umqId(getStringA("UMQID"));
	UINT32 messageId = getDword("MessageID", 0);

	int errors = 0;
	int errorsLimit = getByte("PollingErrorsLimit", POLLING_ERRORS_LIMIT);
	while (IsOnline() && errors < errorsLimit)
	{
		PollRequest *request = new PollRequest(token, umqId, messageId, IdleSeconds());
		// request->nlc = m_pollingConnection;
		HttpResponse *response = request->Send(m_hNetlibUser);
		delete request;

		if (!ResponseHttpOk(response))
		{
			errors++;
		}
		else
		{
			ptrA body((char*)mir_calloc(response->dataLength + 2));
			mir_strncpy(body, response->pData, response->dataLength + 1);
			JSONROOT root(body);
			if (root == NULL)
			{
				errors++;
			}
			else
			{
				JSONNode *node = json_get(root, "error");
				if (node) {
					ptrT error(json_as_string(node));

					if (!lstrcmpi(error, _T("OK")))
					{
						// Remember last message timestamp
						node = json_get(root, "utc_timestamp");
						time_t timestamp = _ttoi64(ptrT(json_as_string(node)));
						if (timestamp > getDword("LastMessageTS", 0))
							setDword("LastMessageTS", timestamp);

						node = json_get(root, "messagelast");
						messageId = json_as_int(node);

						node = json_get(root, "messages");
						JSONNode *nroot = json_as_array(node);

						if (nroot != NULL)
						{
							ParsePollData(nroot);
							json_delete(nroot);
						}

						// Reset error counter only when we've got OK
						errors = 0;

						// m_pollingConnection = response->nlc;
					}
					else if (!lstrcmpi(error, _T("Timeout")))
					{
						// Do nothing as this is not necessarily an error
					}
					else if (!lstrcmpi(error, _T("Not Logged On"))) // 'else' below will handle this error, we don't need this particular check right now
					{
						// need to relogin
						debugLog(_T("CSteamProto::PollingThread: Not Logged On"));

						// try to reconnect only when we're actually online (during normal logout we will still got this error anyway, but in that case our status is already offline)
						if (IsOnline() && Relogin())
						{
							// load umqId and messageId again
							umqId = getStringA("UMQID"); // it's ptrA so we can assign it without fearing a memory leak
							messageId = getDword("MessageID", 0);
						}
						else
						{
							// let it jump out of further processing
							errors = errorsLimit;
						}
					}
					else
					{
						// something wrong
						debugLog(_T("CSteamProto::PollingThread: %s (%d)"), error, response->resultCode);

						// token has expired
						if (response->resultCode == HTTP_CODE_UNAUTHORIZED)
							delSetting("TokenSecret");

						// too low timeout?
						node = json_get(root, "sectimeout");
						int timeout = json_as_int(node);
						if (timeout < STEAM_API_TIMEOUT)
							debugLog(_T("CSteamProto::PollingThread: Timeout is too low (%d)"), timeout);

						// let it jump out of further processing
						errors = errorsLimit;
					}
				}
			}
		}

		delete response;
	}

	setDword("MessageID", messageId);

	if (IsOnline())
	{
		debugLog(_T("CSteamProto::PollingThread: unexpected termination; switching protocol to offline"));
		SetStatus(ID_STATUS_OFFLINE);
	}

	m_hPollingThread = NULL;
	debugLog(_T("CSteamProto::PollingThread: leaving"));
}