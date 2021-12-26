#include "stdafx.h"

void CSteamProto::ParsePollData(const JSONNode &data)
{
	std::string steamIds;

	for (auto &item : data) {
		json_string steamId = item["steamid_from"].as_string();
		json_string type = item["type"].as_string();
		time_t timestamp = _wtol(item["utc_timestamp"].as_mstring());

		bool bIsMe = IsMe(steamId.c_str());
		MCONTACT hContact;
		if (!bIsMe) {
			hContact = GetContact(steamId.c_str());
			if (hContact == 0)
				continue;
		}
		else hContact = 0;

		if (type == "personarelationship") {
			PersonaRelationshipAction state = (PersonaRelationshipAction)item["persona_state"].as_int();
			switch (state) {
			case PersonaRelationshipAction::Remove:
				if (hContact)
					ContactIsRemoved(hContact);
				break;

			case PersonaRelationshipAction::Ignore:
				if (hContact)
					ContactIsBlocked(hContact);
				break;

			case PersonaRelationshipAction::AuthRequest:
				hContact = AddContact(steamId.c_str());
				if (hContact)
					ContactIsAskingAuth(hContact);
				break;

			case PersonaRelationshipAction::AuthRequested:
				if (hContact)
					ContactIsFriend(hContact);
				break;
			}
			continue;
		}

		// probably this is a packet from random player playing on the same server, so we ignore it
		if (!bIsMe && !hContact)
			continue;

		if (type == "my_saytext" || type =="my_emote") {
			json_string text = item["text"].as_string();

			if (timestamp > getDword(hContact, DB_KEY_LASTMSGTS))
				setDword(hContact, DB_KEY_LASTMSGTS, timestamp);

			PROTORECVEVENT recv = { 0 };
			recv.timestamp = timestamp;
			recv.szMessage = (char*)text.c_str();
			recv.flags = PREF_SENT;
			RecvMsg(hContact, &recv);
			continue;
		}

		if (type == "saytext" || type == "emote") {
			json_string text = item["text"].as_string();

			if (timestamp > getDword(hContact, DB_KEY_LASTMSGTS))
				setDword(hContact, DB_KEY_LASTMSGTS, timestamp);

			PROTORECVEVENT recv = { 0 };
			recv.timestamp = timestamp;
			recv.szMessage = (char*)text.c_str();
			ProtoChainRecvMsg(hContact, &recv);

			CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
			m_typingTimestamps[steamId] = 0;
			continue;
		}

		if (type == "typing") {
			auto it = m_typingTimestamps.find(steamId);
			if (it != m_typingTimestamps.end()) {
				if ((timestamp - it->second) < STEAM_TYPING_TIME)
					continue;
			}
			CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)STEAM_TYPING_TIME);
			m_typingTimestamps[steamId] = timestamp;
			continue;
		}

		if (type == "personastate") {
			if (!IsMe(steamId.c_str())) {
				// there no sense to change own status
				const JSONNode &node = item["persona_state"];
				if (node) {
					int status = SteamToMirandaStatus((PersonaState)node.as_int());
					SetContactStatus(hContact, status);
				}
			}
			steamIds.append(steamId).append(",");
			continue;
		}

		if (type == "leftconversation") {
			if (!getBool("ShowChatEvents", true))
				continue;

			uint8_t bEventType = STEAM_DB_EVENT_CHATSTATES_GONE;
			DBEVENTINFO dbei = {};
			dbei.pBlob = &bEventType;
			dbei.cbBlob = 1;
			dbei.eventType = EVENTTYPE_STEAM_CHATSTATES;
			dbei.flags = DBEF_READ;
			dbei.timestamp = now();
			dbei.szModule = m_szModuleName;
			db_event_add(hContact, &dbei);
			continue;
		}

		debugLogA(__FUNCTION__ ": Unknown event type \"%s\"", type.c_str());
	}

	if (!steamIds.empty()) {
		steamIds.pop_back();
		PushRequest(new GetUserSummariesRequest(this, steamIds.c_str()), &CSteamProto::OnGotUserSummaries);
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
	if (!response) {
		// bad response
		debugLogA(__FUNCTION__ ": server returns bad response (%d)", response.GetStatusCode());
		param->errors++;
		Sleep(STEAM_API_TIMEOUT * 1000);
		return;
	}

	// handling of known errors
	if (!response.IsSuccess()) {
		switch (response.GetStatusCode()) {
		case HTTP_CODE_SERVICE_UNAVAILABLE:
			// server on maintenance
			SetAllContactStatuses(ID_STATUS_OFFLINE);
			Sleep(STEAM_API_TIMEOUT * 1000);
			return;

		case HTTP_CODE_UNAUTHORIZED:
			// token has expired
			debugLogA(__FUNCTION__ ": access is denied");
			delSetting("TokenSecret");
			param->errors = param->errorsLimit;
			return;

		default:
			debugLogA(__FUNCTION__ ": server returns unexpected status code (%d)", response.GetStatusCode());
			param->errors++;
			Sleep(STEAM_API_TIMEOUT * 1000);
			return;
		}
	}

	JSONNode root = JSONNode::parse(response.Content);
	if (!root) {
		debugLogA(__FUNCTION__ ": could not recognize a response");
		param->errors++;
		return;
	}

	json_string error = root["error"].as_string();
	if (error == "Timeout") {
		// too low timeout?
		int timeout = root["sectimeout"].as_int();
		if (timeout < STEAM_API_TIMEOUT) {
			debugLogA(__FUNCTION__ ": Timeout is too low (%d)", timeout);
			// perhaps it will break connection
			Sleep(STEAM_API_TIMEOUT * 1000);
		}

		// do nothing as this is not necessarily an error
		return;
	}

	if (error == "OK") {
		long messageId = root["messagelast"].as_int();
		setDword("MessageID", messageId);

		ParsePollData(root["messages"]);

		// Reset error counter only when we've got OK
		param->errors = 0;
		return;
	}

	if (error == "Not Logged On") {
		// need to relogin
		debugLogA(__FUNCTION__ ": not logged on");
		param->errors = param->errorsLimit;
		// try to reconnect only when we're actually online (during normal logout we will still got this error anyway, but in that case our status is already offline)
		if (IsOnline()) {
			ptrA token(getStringA("TokenSecret"));
			SendRequest(new LogonRequest(token), &CSteamProto::OnReLogin);
		}
		return;
	}

	// something wrong
	debugLogA(__FUNCTION__ ": %s (%d)", error.c_str(), response.GetStatusCode());

	// let it jump out of further processing
	param->errors = param->errorsLimit;
}

void CSteamProto::PollingThread(void*)
{
	debugLogA(__FUNCTION__ ": entering");

	PollParam param;
	param.errors = 0;
	param.errorsLimit = getByte("PollingErrorsLimit", STEAM_API_POLLING_ERRORS_LIMIT);
	while (IsOnline() && param.errors < param.errorsLimit)
		SendRequest(new PollRequest(this), &CSteamProto::OnGotPoll, &param);

	if (IsOnline()) {
		debugLogA(__FUNCTION__ ": unexpected termination; switching protocol to offline");
		SetStatus(ID_STATUS_OFFLINE);
	}

	m_hPollingThread = nullptr;
	debugLogA(__FUNCTION__ ": leaving");
}
