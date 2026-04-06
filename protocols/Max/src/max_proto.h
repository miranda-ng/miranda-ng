#pragma once

class CMaxProto;

template<> void WebSocket<CMaxProto>::process(const uint8_t *buf, size_t cbLen);

class CMaxProto : public PROTO<CMaxProto>
{
	friend void WebSocket<CMaxProto>::process(const uint8_t *buf, size_t cbLen);

	WebSocket<CMaxProto> *m_pGateway = nullptr;
	bool m_bGatewayConnected = false;
	bool m_bInitialSyncOk = false;
	bool m_bTerminated = true;
	HANDLE m_hConnThread = nullptr;
	HANDLE m_hWsRunThread = nullptr;
	HANDLE m_hPingThread = nullptr;
	HANDLE m_hWaitEvent = nullptr;
	uint64_t m_seq = 0;
	uint64_t m_waitSeq = 0;
	mir_cs m_csWait;
	mir_cs m_csSend;
	CMStringA m_szPendingResponse;
	z_stream m_wsInflate = {};
	bool m_wsInflateInited = false;
	bool m_bAvatarWebPrimed = false;

	struct
	{
		WebSocket<CMaxProto> *ws = nullptr;
	} m_wsRun;

	void __cdecl ConnectionWorker(void *);
	void __cdecl WsRunThread(void *param);
	void __cdecl PingWorker(void *);
	void InterruptibleSleepMs(DWORD msTotal, DWORD sliceMs = 200);
	void EnsureDeviceId();
	bool SendHandshake(WebSocket<CMaxProto> *ws);
	bool SendJsonAndWait(WebSocket<CMaxProto> *ws, uint16_t opcode, JSONNode &payload, uint8_t cmd = 0);
	void OnGatewayPush(const JSONNode &payload, int opcode);
	void TryMergeContactsFromPayload(const JSONNode &payload);
	void TryApplySyncPayloadFromPush(const JSONNode &payload);
	bool ApiPing(WebSocket<CMaxProto> *ws);
	bool ApiSendTelemetryColdStart(WebSocket<CMaxProto> *ws);
	/// Optional web session bootstrap (requests.md): ping, token refresh ping, folders — before opcode 19 sync.
	bool ApiWebSessionBootstrap(WebSocket<CMaxProto> *ws);
	void DisconnectGateway();

	bool WaitForGatewayReady();
	bool HasLoginToken();
	CMStringW FormatLastError();
	int __cdecl OnOptionsInit(WPARAM, LPARAM);

public:
	CMaxProto(const char *szModuleName, const wchar_t *ptszUserName);
	~CMaxProto();

	INT_PTR GetCaps(int type, MCONTACT hContact = 0) override;
	int SetStatus(int iNewStatus) override;
	int SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char *msg) override;

	MWindow OnCreateAccMgrUI(MWindow hwndParent) override;
	void OnShutdown(void) override;

	void NotifyUser(const wchar_t *title, const wchar_t *text);
	void InitWsInflater();
	void FreeWsInflater();
	bool InflateWsFrame(const uint8_t *pData, size_t cbData, CMStringA &out);

	bool ApiSync(WebSocket<CMaxProto> *ws);
	bool ApiFetchContactsBatch(WebSocket<CMaxProto> *ws, const CMStringA *pUids, size_t nUids);
	bool ApiFetchChatsByIds(WebSocket<CMaxProto> *ws, const CMStringA *pChatIds, size_t nIds);

	void RegisterChatModule();
	void ApplySyncPayload(const JSONNode &payload, WebSocket<CMaxProto> *ws);
	CMStringW GetDefaultGroupW();
	MCONTACT FindContactByMaxUid(const char *szUid);

	// Avatars (AVS): URL from JSON, HTTP download on demand
	CMStringA ExtractAvatarUrlFromJson(const JSONNode &c);
	void SyncContactAvatarFromJson(MCONTACT hContact, const JSONNode &c);
	void GetAvatarFileName(MCONTACT hContact, wchar_t *pwszDest, size_t cchDest);
	bool DownloadAvatarToFile(MCONTACT hContact, const char *szUrl, wchar_t *wszPath, size_t cchPath);
	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl SvcGetMyAvatar(WPARAM wParam, LPARAM lParam);
	MCONTACT EnsureUserContact(const char *szUid, const wchar_t *wszFirst, const wchar_t *wszLast, const char *szDialogChatId);
	void EnsureGroupChatSession(const CMStringA &szChatId, const wchar_t *wszTitle);
	void MergeContactJson(const JSONNode &c, const char *szRequestedUid = nullptr);
};

struct CMPlugin : public ACCPROTOPLUGIN<CMaxProto>
{
	CMPlugin();
	int Load() override;
};

extern CMPlugin g_plugin;
