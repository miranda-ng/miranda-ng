#ifndef _STEAM_MESSAGE_H_
#define _STEAM_MESSAGE_H_

namespace SteamWebApi
{
	//class MessageApi : public BaseApi
	//{
	//public:
	//	class SendResult : public Result
	//	{
	//		friend MessageApi;

	//	private:
	//		DWORD timestamp;

	//	public:
	//		SendResult() : timestamp(0) { }

	//		const DWORD GetTimestamp() const { return timestamp; }
	//	};

	//	static void SendStatus(HANDLE hConnection, const char *token, const char *umqId, int state, SendResult *sendResult)
	//	{
	//		sendResult->success = false;

	//		char data[256];
	//		mir_snprintf(data, SIZEOF(data),
	//			"access_token=%s&umqid=%s&type=personastate&persona_state=%i",
	//			token,
	//			umqId,
	//			state);

	//		SecureHttpPostRequest request(hConnection, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Message/v0001");
	//		request.SetData(data, strlen(data));

	//		mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
	//		if (!response)
	//			return;

	//		if ((sendResult->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
	//			return;

	//		sendResult->success = true;
	//	}

	//	static void SendMessage(HANDLE hConnection, const char *token, const char *umqId, const char *steamId, const char *text, SendResult *sendResult)
	//	{
	//		sendResult->success = false;

	//		char data[1024];
	//		mir_snprintf(data, SIZEOF(data),
	//			"access_token=%s&umqid=%s&steamid_dst=%s&type=saytext&text=%s",
	//			token,
	//			umqId,
	//			steamId,
	//			ptrA(mir_urlEncode(text)));

	//		SecureHttpPostRequest request(hConnection, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Message/v0001");
	//		request.SetData(data, strlen(data));

	//		mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
	//		if (!response)
	//			return;

	//		if ((sendResult->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
	//			return;

	//		JSONNODE *root = json_parse(response->pData), *node;

	//		node = json_get(root, "error");
	//		ptrW error(json_as_string(node));

	//		if (lstrcmp(error, L"OK"))
	//			return;

	//		node = json_get(root, "utc_timestamp");
	//		sendResult->timestamp = atol(ptrA(mir_u2a(json_as_string(node))));

	//		sendResult->success = true;
	//	}

	//	static void SendTyping(HANDLE hConnection, const char *token, const char *umqId, const char *steamId, SendResult *sendResult)
	//	{
	//		sendResult->success = false;

	//		char data[256];
	//		mir_snprintf(data, SIZEOF(data),
	//			"access_token=%s&umqid=%s&steamid_dst=%s&type=typing",
	//			token,
	//			umqId,
	//			steamId);

	//		SecureHttpPostRequest request(hConnection, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Message/v0001");
	//		request.SetData(data, strlen(data));

	//		mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
	//		if (!response)
	//			return;

	//		if ((sendResult->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
	//			return;

	//		sendResult->success = true;
	//	}
	//};

	class SendMessageRequest : public HttpsPostRequest
	{
	public:
		SendMessageRequest(const char *token, const char *umqId, const char *steamId, const char *text) :
			HttpsPostRequest(STEAM_API_URL "/ISteamWebUserPresenceOAuth/Message/v0001")
		{
			char data[1024];
			mir_snprintf(data, SIZEOF(data),
				"access_token=%s&umqid=%s&steamid_dst=%s&type=saytext&text=%s",
				token,
				umqId,
				steamId,
				ptrA(mir_urlEncode(text)));

			SetData(data, strlen(data));
		}
	};
}


#endif //_STEAM_MESSAGE_H_