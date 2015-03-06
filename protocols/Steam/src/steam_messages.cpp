#include "common.h"

//void CSteamProto::SendTypingThread(void *arg)
//{
//	MCONTACT hContact = (MCONTACT)arg;
//
//	ptrA token(getStringA("TokenSecret"));
//	ptrA umqId(getStringA("UMQID"));
//	ptrA steamId(getStringA(hContact, "SteamID"));
//
//	SteamWebApi::MessageApi::SendResult sendResult;
//	debugLogA("CSteamProto::SendTypingThread: call SteamWebApi::PollApi::SteamWebApi::MessageApi::SendMessage");
//	SteamWebApi::MessageApi::SendTyping(m_hNetlibUser, token, umqId, steamId, &sendResult);
//}

void CSteamProto::OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	int status = ACKRESULT_FAILED;
	ptrT error;

	ptrT steamId(getTStringA(param->hContact, "SteamID"));

	if (response != NULL && response->resultCode == HTTP_STATUS_OK)
	{
		JSONROOT root(response->pData);
		JSONNODE *node = json_get(root, "error");
		error = json_as_string(node);
	}

	if (!mir_tstrcmpi(error, _T("OK")))
	{
		status = ACKRESULT_SUCCESS;
		error = NULL;
	}
	else
	{
		if (!error)
			error = mir_tstrdup(IsOnline() ? TranslateT("Unknown error") : TranslateT("You cannot send messages when you are offline."));
		
		debugLog(_T("CSteamProto::OnMessageSent: failed to send message for %s (%s)"), steamId, error);
	}

	ProtoBroadcastAck(
		param->hContact,
		ACKTYPE_MESSAGE,
		status,
		param->hMessage,
		error);
}