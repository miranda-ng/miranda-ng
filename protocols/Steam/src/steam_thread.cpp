#include "common.h"

int SteamToMirandaStatus(int state)
{
	switch (state)
	{
	case 0: //Offline
		return ID_STATUS_OFFLINE;
	case 2: //Busy
		return ID_STATUS_DND;
	case 3: //Away
		return ID_STATUS_AWAY;
		/*case 4: //Snoozing
			prim = PURPLE_STATUS_EXTENDED_AWAY;
			break;
			case 5: //Looking to trade
			return "trade";
			case 6: //Looking to play
			return "play";*/
		//case 1: //Online
	default:
		return ID_STATUS_ONLINE;
	}
}


int CSteamProto::PollStatus(const char *sessionId, const char *steamId, UINT32 messageId)
{
	SteamWebApi::PollApi::PollResult pollResult;
	SteamWebApi::PollApi::PollStatus(m_hNetlibUser, sessionId, steamId, messageId, &pollResult);

	if (!pollResult.IsSuccess())
		return 0;

	for (int i = 0; i < pollResult.GetItemCount(); i++)
	{
		switch (pollResult[i]->GetType())
		{
		case SteamWebApi::PollApi::POOL_TYPE::TYPING:
			break;

		case SteamWebApi::PollApi::POOL_TYPE::MESSAGE:
			{
				const wchar_t *text = ((SteamWebApi::PollApi::Message*)pollResult[i])->GetText();
			}
			break;

		case SteamWebApi::PollApi::POOL_TYPE::STATE:
			{
				int status = ((SteamWebApi::PollApi::State*)pollResult[i])->GetStatus();
				const wchar_t *nickname = ((SteamWebApi::PollApi::State*)pollResult[i])->GetNickname();
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