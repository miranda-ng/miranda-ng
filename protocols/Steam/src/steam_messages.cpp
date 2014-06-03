#include "common.h"

void CSteamProto::SendMessageThread(void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	ptrA token(getStringA("TokenSecret"));
	ptrA umqId(getStringA("UMQID"));
	ptrA steamId(getStringA(param->hContact, "SteamID"));

	SteamWebApi::MessageApi::SendResult sendResult;
	debugLogA("CSteamProto::SendMessageThread: call SteamWebApi::PollApi::SteamWebApi::MessageApi::SendMessage");
	SteamWebApi::MessageApi::SendMessage(m_hNetlibUser, token, umqId, steamId, param->text, &sendResult);

	ProtoBroadcastAck(
		param->hContact,
		ACKTYPE_MESSAGE,
		sendResult.IsSuccess() ? ACKRESULT_SUCCESS : ACKRESULT_FAILED,
		param->hMessage, 0);

	mir_free((void*)param->text);
	mir_free(param);
}

void CSteamProto::SendTypingThread(void *arg)
{
	MCONTACT hContact = (MCONTACT)arg;

	ptrA token(getStringA("TokenSecret"));
	ptrA umqId(getStringA("UMQID"));
	ptrA steamId(getStringA(hContact, "SteamID"));

	SteamWebApi::MessageApi::SendResult sendResult;
	debugLogA("CSteamProto::SendTypingThread: call SteamWebApi::PollApi::SteamWebApi::MessageApi::SendMessage");
	SteamWebApi::MessageApi::SendTyping(m_hNetlibUser, token, umqId, steamId, &sendResult);
}

void CSteamProto::OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	int status = response->resultCode == HTTP_STATUS_OK ? ACKRESULT_SUCCESS : ACKRESULT_FAILED;

	ProtoBroadcastAck(
		param->hContact,
		ACKTYPE_MESSAGE,
		status,
		param->hMessage, 0);

	mir_free(param);
}