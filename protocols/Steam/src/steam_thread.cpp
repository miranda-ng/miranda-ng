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


int CSteamProto::PollStatus()
{
	ptrA steamId(getStringA("SteamID"));
	ptrA sessionId(getStringA("SessionID"));
	ptrA messageId(getStringA("MessageID"));

	SteamWebApi::PollApi::Poll poll;
	SteamWebApi::PollApi::PollStatus(m_hNetlibUser, sessionId, steamId, messageId, &poll);

	if (!poll.IsSuccess())
		return -1;

	//setString

	for (int i = 0; i < poll.GetItemCount(); i++)
	{
		switch (poll[i]->GetType())
		{
		case SteamWebApi::PollApi::POOL_TYPE::TYPING:
			break;

		case SteamWebApi::PollApi::POOL_TYPE::MESSAGE:
			{
				const wchar_t *text = ((SteamWebApi::PollApi::Message*)poll[i])->GetText();
			}
			break;

		case SteamWebApi::PollApi::POOL_TYPE::STATE:
			{
				int status = ((SteamWebApi::PollApi::State*)poll[i])->GetStatus();
				const wchar_t *nickname = ((SteamWebApi::PollApi::State*)poll[i])->GetNickname();
			}
			break;
		}
	}

	return 0;
}

void CSteamProto::PollingThread(void*)
{
	debugLogA("CSteamProto::PollingThread: entering");

	while (!m_bTerminated)
		if (PollStatus() == -1)
			break;

	m_hPollingThread = NULL;
	debugLogA("CSteamProto::PollingThread: leaving");
}