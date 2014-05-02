#include "common.h"

void CSteamProto::PollServer(const char *token, const char *umqId, UINT32 messageId, SteamWebApi::PollApi::PollResult *pollResult)
{
	debugLogA("CSteamProto::PollServer: call SteamWebApi::PollApi::Poll");
	SteamWebApi::PollApi::Poll(m_hNetlibUser, token, umqId, messageId, pollResult);

	if (!pollResult->IsSuccess())
		return;

	CMStringA updatedIds;
	for (size_t i = 0; i < pollResult->GetItemCount(); i++)
	{
		const SteamWebApi::PollApi::PoolItem *item = pollResult->GetAt(i);
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

					AddDBEvent(hContact, EVENTTYPE_MESSAGE, time(NULL), DBEF_UTF | DBEF_SENT, lstrlen(text), (BYTE*)mir_utf8encodeW(text));
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
					if (status == ID_STATUS_OFFLINE)
						continue;

					if (status != m_iStatus)
					{
						debugLogA("Change own status to %i", status);
						WORD oldStatus = m_iStatus;
						m_iStatus = m_iDesiredStatus = status;
						ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
					}
				}
				/*else
				{
					MCONTACT hContact = FindContact(steamId);
					if (hContact)
						SetContactStatus(hContact, status);
				}*/

				if (updatedIds.IsEmpty())
					updatedIds.Append(steamId);
				else
					updatedIds.AppendFormat(",%s", steamId);
			}
			break;

		case SteamWebApi::PollApi::POOL_TYPE_CONTACT_ADD:
			{
				SteamWebApi::PollApi::Relationship *crs = (SteamWebApi::PollApi::Relationship*)item;

				const char *steamId = crs->GetSteamId();
				if (updatedIds.IsEmpty())
					updatedIds.Append(steamId);
				else
					updatedIds.AppendFormat(",%s", steamId);
			}
			break;

		case SteamWebApi::PollApi::POOL_TYPE_CONTACT_REMOVE:
			{
				SteamWebApi::PollApi::Relationship *crs = (SteamWebApi::PollApi::Relationship*)item;

				const char *steamId = crs->GetSteamId();
				MCONTACT hContact = FindContact(steamId);
				if (hContact)
					CallService(MS_DB_CONTACT_DELETE, hContact, 0);
			}
			break;

		case SteamWebApi::PollApi::POOL_TYPE_CONTACT_REQUEST:
			{
				SteamWebApi::PollApi::Relationship *crs = (SteamWebApi::PollApi::Relationship*)item;

				const char *steamId = crs->GetSteamId();
				
				MCONTACT hContact = FindContact(steamId);
				if (!hContact)
					hContact = AddContact(steamId, true);

				RaiseAuthRequestThread((void*)hContact);
			}
			break;
		}
	}

	if (!updatedIds.IsEmpty())
		UpdateContactsThread(mir_strdup(updatedIds));
}

void CSteamProto::PollingThread(void*)
{
	debugLogA("CSteamProto::PollingThread: entering");

	ptrA token(getStringA("TokenSecret"));
	ptrA umqId(getStringA("UMQID"));
	UINT32 messageId = getDword("MessageID", 0);

	SteamWebApi::PollApi::PollResult pollResult;
	while (!m_bTerminated)
	{
		PollServer(token, umqId, messageId, &pollResult);
		
		if (pollResult.IsNeedRelogin())
		{
			debugLogA("CSteamProto::PollingThread: need to relogin");
			SetStatus(ID_STATUS_OFFLINE);
		}

		if (!pollResult.IsSuccess())
		{
			debugLogA("CSteamProto::PollServer: call SteamWebApi::PollApi::Poll");
			SetStatus(ID_STATUS_OFFLINE);

			// token has expired
			if (pollResult.GetStatus() == HTTP_STATUS_UNAUTHORIZED)
			{
				delSetting("TokenSecret");
				//delSetting("Cookie");
			}

			break;
		}

		messageId = pollResult.GetMessageId();
		setDword("MessageID", messageId);
	}

	m_hPollingThread = NULL;
	debugLogA("CSteamProto::PollingThread: leaving");
}