#include "common.h"

void CSteamProto::PollStatus(const char *token, const char *sessionId, UINT32 messageId, SteamWebApi::PollApi::PollResult *pollResult)
{
	SteamWebApi::PollApi::PollStatus(m_hNetlibUser, token, sessionId, messageId, pollResult);

	if (!pollResult->IsSuccess())
		return;

	for (int i = 0; i < pollResult->GetItemCount(); i++)
	{
		const SteamWebApi::PollApi::PoolItem *item = pollResult->operator[](i);
		switch (item->GetType())
		{
		case SteamWebApi::PollApi::POOL_TYPE_TYPING:
			break;

		case SteamWebApi::PollApi::POOL_TYPE_MESSAGE:
			{
				SteamWebApi::PollApi::Message *message = (SteamWebApi::PollApi::Message*)item;

				MCONTACT hContact = FindContact(message->GetSteamId());
				if (hContact)
				{
					const wchar_t *text = message->GetText();

					PROTORECVEVENT recv = { 0 };
					recv.flags = PREF_UTF;
					recv.timestamp = message->GetTimestamp();
					recv.szMessage = mir_utf8encodeW(text);

					ProtoChainRecvMsg(hContact, &recv);
				}
			}
			break;

		case SteamWebApi::PollApi::POOL_TYPE_MYMESSAGE:
			{
				SteamWebApi::PollApi::Message *message = (SteamWebApi::PollApi::Message*)item;

				MCONTACT hContact = FindContact(message->GetSteamId());
				if (hContact)
				{
					const wchar_t *text = message->GetText();

					DBEVENTINFO dbei = { sizeof(dbei) };
					dbei.szModule = this->m_szModuleName;
					dbei.timestamp = message->GetTimestamp();
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.cbBlob = lstrlen(text);
					dbei.pBlob = (BYTE*)mir_utf8encodeW(text);
					dbei.flags = DBEF_UTF | DBEF_SENT;

					db_event_add(hContact, &dbei);
				}
			}
			break;

		case SteamWebApi::PollApi::POOL_TYPE_STATE:
			{
				SteamWebApi::PollApi::State *state = (SteamWebApi::PollApi::State*)item;

				WORD status = CSteamProto::SteamToMirandaStatus(state->GetStatus());
				const char *steamId = state->GetSteamId();
				const wchar_t *nickname = state->GetNickname();

				if (IsMe(steamId))
				{
					const wchar_t *oldNickname = getWStringA("Nick");
					if (lstrcmp(oldNickname, nickname) && lstrlen(nickname))
						setWString("Nick", nickname);

					SetStatus(status);
				}
				else
				{
					MCONTACT hContact = FindContact(steamId);
					if (hContact)
					{
						const wchar_t *oldNickname = getWStringA(hContact, "Nick");
						if (lstrcmp(oldNickname, nickname) && lstrlen(nickname))
							setWString(hContact, "Nick", nickname);

						SetContactStatus(hContact, status);
					}
				}
			}
			break;
		}
	}
}

void CSteamProto::PollingThread(void*)
{
	debugLogA("CSteamProto::PollingThread: entering");

	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	UINT32 messageId = getDword("MessageID", 0);

	SteamWebApi::PollApi::PollResult pollResult;
	while (!m_bTerminated)
	{
		PollStatus(token, sessionId, messageId, &pollResult);
		
		if (pollResult.IsNeedRelogin())
			debugLogA("CSteamProto::PollingThread: need to relogin");
		/*{
			SteamWebApi::LoginApi::LoginResult loginResult;
			SteamWebApi::LoginApi::Logon(m_hNetlibUser, token, &loginResult);

			if (!loginResult.IsSuccess())
				break;

			sessionId = mir_strdup(loginResult.GetSessionId());
			setString("SessionID", sessionId);
		}*/

		if (!pollResult.IsSuccess())
			break;

		messageId = pollResult.GetMessageId();
	}

	if (pollResult.IsSuccess())
		setDword("MessageID", messageId);
	else
		SetStatus(ID_STATUS_OFFLINE);

	m_hPollingThread = NULL;
	debugLogA("CSteamProto::PollingThread: leaving");
}