#include "common.h"

int CSteamProto::PollStatus(const char *token, const char *sessionId, UINT32 messageId)
{
	SteamWebApi::PollApi::PollResult pollResult;
	SteamWebApi::PollApi::PollStatus(m_hNetlibUser, token, sessionId, messageId, &pollResult);

	if (!pollResult.IsSuccess())
		return 0;

	if (pollResult.IsNeedRelogin())
	{
		SteamWebApi::LoginApi::LoginResult loginResult;
		SteamWebApi::LoginApi::Logon(m_hNetlibUser, token, &loginResult);

		if (!loginResult.IsSuccess())
			return 0;

		return messageId;
	}

	for (int i = 0; i < pollResult.GetItemCount(); i++)
	{
		switch (pollResult[i]->GetType())
		{
		case SteamWebApi::PollApi::POOL_TYPE::TYPING:
			break;

		case SteamWebApi::PollApi::POOL_TYPE::MESSAGE:
			{
				SteamWebApi::PollApi::Message *message = (SteamWebApi::PollApi::Message*)pollResult[i];

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

		case SteamWebApi::PollApi::POOL_TYPE::MYMESSAGE:
			{
				SteamWebApi::PollApi::Message *message = (SteamWebApi::PollApi::Message*)pollResult[i];

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

		case SteamWebApi::PollApi::POOL_TYPE::STATE:
			{
				SteamWebApi::PollApi::State *state = (SteamWebApi::PollApi::State*)pollResult[i];

				WORD status = CSteamProto::SteamToMirandaStatus(state->GetStatus());
				const char *cSteamId = state->GetSteamId();
				const wchar_t *nickname = state->GetNickname();

				ptrA steamId(getStringA("SteamID"));
				if (!lstrcmpA(steamId, cSteamId))
				{
					const wchar_t *oldNickname = getWStringA("Nick");
					if (lstrcmp(oldNickname, nickname))
						setWString("Nick", nickname);
						SetStatus(status);
						
				}
				else
				{
					MCONTACT hContact = FindContact(cSteamId);
					if (hContact)
					{
						const wchar_t *oldNickname = getWStringA(hContact, "Nick");
						if (lstrcmp(oldNickname, nickname))
							setWString(hContact, "Nick", nickname);
						SetContactStatus(hContact, status);
					}
				}
			}
			break;
		}
	}

	return pollResult.GetMessageId();
}

void CSteamProto::PollingThread(void*)
{
	debugLogA("CSteamProto::PollingThread: entering");

	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	UINT32 messageId = getDword("MessageID", 0);

	while (!m_bTerminated)
	{
		messageId = PollStatus(token, sessionId, messageId);
		if (messageId == 0)
			break;
	}

	if (messageId > 0)
		setDword("MessageID", messageId);

	m_hPollingThread = NULL;
	debugLogA("CSteamProto::PollingThread: leaving");
}