#include "stdafx.h"

#define POLLING_ERRORS_LIMIT 5

void CSteamProto::ParsePollData(const JSONNode &data)
{
	std::string steamIds;
	for (const JSONNode &item : data)
	{
		json_string steamId = item["steamid_from"].as_string();
		time_t timestamp = _wtol(item["utc_timestamp"].as_mstring());

		MCONTACT hContact = NULL;
		if (!IsMe(steamId.c_str()) && !(hContact = FindContact(steamId.c_str())))
			// probably this is info about random player playing on same server, so we ignore it
			continue;

		json_string type = item["type"].as_string();
		if (!mir_strcmpi(type.c_str(), "my_saytext") ||
			!mir_strcmpi(type.c_str(), "my_emote"))
		{
			json_string text = item["text"].as_string();

			PROTORECVEVENT recv = { 0 };
			recv.timestamp = timestamp;
			recv.szMessage = (char*)text.c_str();
			recv.flags = PREF_SENT;
			Proto_RecvMessage(hContact, &recv);
		}
		else if (!mir_strcmpi(type.c_str(), "saytext") ||
			!mir_strcmpi(type.c_str(), "emote"))
		{
			json_string text = item["text"].as_string();

			PROTORECVEVENT recv = { 0 };
			recv.timestamp = timestamp;
			recv.szMessage = (char*)text.c_str();
			ProtoChainRecvMsg(hContact, &recv);

			CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
			m_typingTimestamps[steamId] = 0;
		}
		else if (!mir_strcmpi(type.c_str(), "typing") && hContact)
		{
			auto it = m_typingTimestamps.find(steamId);
			if (it != m_typingTimestamps.end())
			{
				if ((timestamp - it->second) < STEAM_TYPING_TIME)
					continue;
			}
			CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)STEAM_TYPING_TIME);
			m_typingTimestamps[steamId] = timestamp;
		}
		else if (!mir_strcmpi(type.c_str(), "personastate"))
		{
			JSONNode node = item["persona_state"];
			int status = !node.isnull()
				? SteamToMirandaStatus(node.as_int())
				: -1;

			if (IsMe(steamId.c_str()))
			{
				if (status == -1 || status == ID_STATUS_OFFLINE)
					continue;
				SetStatus(status);
				continue;
			}
			
			if (status != -1)
				SetContactStatus(hContact, status);

			// todo: find difference between state changing and info changing
			steamIds.append(steamId).append(",");
		}
		else if (!mir_strcmpi(type.c_str(), "personarelationship"))
		{
			int state = item["persona_state"].as_int();
			switch (state)
			{
			case 0:
				hContact = FindContact(steamId.c_str());
				if (hContact)
					ContactIsRemoved(hContact);
				break;

			case 1:
				hContact = FindContact(steamId.c_str());
				if (hContact)
					ContactIsIgnored(hContact);
				break;

			case 2:
				// auth request
				hContact = FindContact(steamId.c_str());
				if (hContact)
					ContactIsAskingAuth(hContact);
				else
				{
					// load info about this user from server
					ptrA token(getStringA("TokenSecret"));

					PushRequest(
						new GetUserSummariesRequest(token, steamId.c_str()),
						&CSteamProto::OnAuthRequested);
				}
				break;

			case 3:
				// todo: add to list
				break;

			default:
				continue;
			}
		}
		else if (!mir_strcmpi(type.c_str(), "leftconversation") && hContact)
		{
			if (!getBool("ShowChatEvents", true))
				continue;

			BYTE bEventType = STEAM_DB_EVENT_CHATSTATES_GONE;
			DBEVENTINFO dbei = {};
			dbei.pBlob = &bEventType;
			dbei.cbBlob = 1;
			dbei.eventType = EVENTTYPE_STEAM_CHATSTATES;
			dbei.flags = DBEF_READ;
			dbei.timestamp = time(nullptr);
			dbei.szModule = m_szModuleName;
			db_event_add(hContact, &dbei);
		}
		else
		{
			debugLogA(__FUNCTION__ ": Unknown event type \"%s\"", type.c_str());
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

struct PollParam
{
	int errors;
	int errorsLimit;
};

void CSteamProto::OnGotPoll(const HttpResponse &response, void *arg)
{
	PollParam *param = (PollParam*)arg;
	if (!response.IsSuccess())
	{
		param->errors++;
		return;
	}

	JSONNode root = JSONNode::parse(response.Content);
	if (root.isnull())
	{
		param->errors++;
		return;
	}

	json_string error = root["error"].as_string();
	if (!mir_strcmpi(error.c_str(), "Timeout"))
	{
		// Do nothing as this is not necessarily an error
	}
	else if (!mir_strcmpi(error.c_str(), "OK"))
	{
		// Remember last message timestamp
		time_t timestamp = _wtoi64(root["utc_timestamp"].as_mstring());
		if (timestamp > getDword("LastMessageTS", 0))
			setDword("LastMessageTS", timestamp);

		long messageId = root["messagelast"].as_int();
		setDword("MessageID", messageId);

		JSONNode data = root["messages"].as_array();
		ParsePollData(data);

		// Reset error counter only when we've got OK
		param->errors = 0;

		// m_pollingConnection = response->nlc;
	}
	else if (!mir_strcmpi(error.c_str(), "Not Logged On")) // 'else' below will handle this error, we don't need this particular check right now
	{
		// need to relogin
		debugLogA(__FUNCTION__ ": Not Logged On");

		// try to reconnect only when we're actually online (during normal logout we will still got this error anyway, but in that case our status is already offline)
		if (!IsOnline() && !Relogin())
		{
			// let it jump out of further processing
			param->errors = param->errorsLimit;
		}
	}
	else
	{
		// something wrong
		debugLogA(__FUNCTION__ ": %s (%d)", error.c_str(), response.GetStatusCode());

		// token has expired
		if (response.GetStatusCode() == HTTP_CODE_UNAUTHORIZED)
			delSetting("TokenSecret");

		// too low timeout?
		int timeout = root["sectimeout"].as_int();
		if (timeout < STEAM_API_TIMEOUT)
			debugLogA(__FUNCTION__ ": Timeout is too low (%d)", timeout);

		// let it jump out of further processing
		param->errors = param->errorsLimit;
	}
}

void CSteamProto::PollingThread(void*)
{
	debugLogA(__FUNCTION__ ": entering");

	ptrA token(getStringA("TokenSecret"));

	PollParam param;
	param.errors = 0;
	param.errorsLimit = getByte("PollingErrorsLimit", POLLING_ERRORS_LIMIT);
	while (IsOnline() && param.errors < param.errorsLimit)
	{
		// request->nlc = m_pollingConnection;
		ptrA umqId(getStringA("UMQID"));
		UINT32 messageId = getDword("MessageID", 0);
		SendRequest(
			new PollRequest(token, umqId, messageId, IdleSeconds()),
			&CSteamProto::OnGotPoll,
			(void*)&param);
	}

	if (IsOnline())
	{
		debugLogA(__FUNCTION__ ": unexpected termination; switching protocol to offline");
		SetStatus(ID_STATUS_OFFLINE);
	}

	m_hPollingThread = nullptr;
	debugLogA(__FUNCTION__ ": leaving");
}
