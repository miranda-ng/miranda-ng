#include "common.h"

void CSteamProto::SendMessageThread(void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	ptrA steamId(getStringA(param->hContact, "SteamID"));

	SteamWebApi::MessageApi::SendResult sendResult;
	SteamWebApi::MessageApi::SendMessage(m_hNetlibUser, token, sessionId, steamId, param->text, &sendResult);

	ProtoBroadcastAck(
		param->hContact,
		ACKTYPE_MESSAGE,
		sendResult.IsSuccess() ? ACKRESULT_SUCCESS : ACKRESULT_FAILED,
		param->hMessage, 0);

	if (sendResult.IsSuccess())
	{
		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.szModule = this->m_szModuleName;
		dbei.timestamp = sendResult.GetTimestamp();
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.cbBlob = strlen(param->text);
		dbei.pBlob = (BYTE*)param->text;
		dbei.flags = DBEF_UTF | DBEF_SENT;
		
		db_event_add(param->hContact, &dbei);
	}
	
	//mir_free((void*)param->text);
	mir_free(param);
}

void CSteamProto::SendTypingThread(void *arg)
{
	MCONTACT hContact = (MCONTACT)arg;

	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));
	ptrA steamId(getStringA(hContact, "SteamID"));

	SteamWebApi::MessageApi::SendResult sendResult;
	SteamWebApi::MessageApi::SendTyping(m_hNetlibUser, token, sessionId, steamId, &sendResult);
}