#pragma once

#define MAX_SETTINGS_ID "MaxID"
#define MAX_SETTINGS_TOKEN "Token"
#define MAX_SETTINGS_DEVICE_ID "DeviceId"
#define MAX_SETTINGS_CHAT_ID "ChatID"

class CMaxProto : public PROTO<CMaxProto>
{
public:
	CMaxProto(const char *szModuleName, const wchar_t *ptszUserName);
	~CMaxProto();

	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	int      SendMsg(MCONTACT hContact, MEVENT, const char *msg) override;
	int      SetStatus(int iNewStatus) override;
	HANDLE   SearchBasic(const wchar_t *id) override;
	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	MWindow  OnCreateAccMgrUI(MWindow) override;

private:
	struct MaxFrame
	{
		uint8_t ver = 11;
		uint8_t cmd = 0;
		uint8_t seq = 0;
		uint16_t opcode = 0;
		JSONNode payload;
	};

	struct PendingRequest
	{
		HANDLE hEvent = nullptr;
		JSONNode payload;
		bool ok = false;
	};

	volatile LONG m_iTerminated;
	volatile LONG m_msgId;
	volatile LONG m_seq;
	HNETLIBUSER m_hNetlibUser;
	HNETLIBCONN m_hConnection;
	mir_cs m_csNet;
	mir_cs m_csPending;
	std::map<uint8_t, PendingRequest*> m_pending;

	void InitNetlib();
	void ShutdownConnection();

	void __cdecl WorkerThread(void *);
	void __cdecl SearchThread(void *arg);

	bool Connect();
	bool SendFrame(const MaxFrame &frame);
	bool ReadFrame(MaxFrame &frame);
	bool ReadExact(void *buf, int cbSize);
	bool LoginWithToken();
	bool SendKeepAlive();

	bool SendAndWait(uint16_t opcode, const JSONNode &payload, JSONNode &outPayload, uint8_t cmd = 0);
	bool SendAndWaitBlocking(uint16_t opcode, const JSONNode &payload, JSONNode &outPayload, uint8_t cmd = 0);
	uint8_t NextSeq();

	void DispatchFrame(const MaxFrame &frame);
	void DispatchIncomingMessage(const JSONNode &payload);

	bool EncodePayload(const JSONNode &payload, MBinBuffer &out) const;
	bool EncodeNode(cmp_ctx_t *ctx, const JSONNode &node) const;
	bool DecodePayload(const uint8_t *data, size_t cbData, JSONNode &outPayload) const;
	bool DecodeNode(cmp_ctx_t *ctx, JSONNode &outNode) const;

	JSONNode BuildHandshakePayload();
	JSONNode BuildSyncPayload();
};

struct CMPlugin : public ACCPROTOPLUGIN<CMaxProto>
{
	CMPlugin();
};
