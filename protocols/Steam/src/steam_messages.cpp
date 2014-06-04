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

	bool result = false;

	ptrA steamId((char*)arg);

	if (response != NULL && response->resultCode == HTTP_STATUS_OK)
	{
		JSONNODE *root = json_parse(response->pData), *node;

		node = json_get(root, "error");
		ptrA error(mir_utf8encodeW(json_as_string(node)));
		if (lstrcmpiA(error, "OK") == 0)
			result = true;
		else
			debugLogA("CSteamProto::OnMessageSent: failed to send message for %s (%s)", steamId, error);
	}
	else
		debugLogA("CSteamProto::OnMessageSent: failed to send message for %s", steamId);

	int status = result ? ACKRESULT_SUCCESS : ACKRESULT_FAILED;

	ProtoBroadcastAck(
		param->hContact,
		ACKTYPE_MESSAGE,
		status,
		param->hMessage, 0);

	mir_free(param);
}