#pragma once

#include <vector>

class CMaxProto;
struct MHttpResponse;

template<> void WebSocket<CMaxProto>::process(const uint8_t *buf, size_t cbLen);

class CMaxProto : public PROTO<CMaxProto>
{
	friend void WebSocket<CMaxProto>::process(const uint8_t *buf, size_t cbLen);
	friend class CMaxQRDlg;

	WebSocket<CMaxProto> *m_pGateway = nullptr;
	bool m_bGatewayConnected = false;
	bool m_bInitialSyncOk = false;
	bool m_bTerminated = true;
	HANDLE m_hConnThread = nullptr;
	DWORD m_dwConnThreadId = 0;
	HANDLE m_hWsRunThread = nullptr;
	HANDLE m_hPingThread = nullptr;
	HANDLE m_hWaitEvent = nullptr;
	uint64_t m_seq = 0;
	uint64_t m_waitSeq = 0;
	mir_cs m_csWait;
	mir_cs m_csSend;
	mir_cs m_csCid;
	mir_cs m_csContactBook;
	/// UIDs from last server `contacts` list (non-REMOVED), after ApplyServerContactBookSnapshot.
	std::vector<CMStringA> m_contactBookUids;
	/// Set when ApplyServerContactBookSnapshot ran after a sync (book may be empty). Used before chat-only auto-prune.
	bool m_bContactBookSnapshotApplied = false;
	volatile LONG m_iSendMsgSeq = 0;
	uint64_t m_lastClientCidMs = 0;
	CMStringA m_szPendingResponse;
	z_stream m_wsInflate = {};
	bool m_wsInflateInited = false;
	/// Set from HTTP 101 `Sec-WebSocket-Extensions` when server sends `server_no_context_takeover`.
	bool m_bWsPmDeflateIndependent = false;
	/// Last locale we asked the server for ("ru" or "en") on the current WS session.
	CMStringA m_wsLocale;
	bool m_bAvatarWebPrimed = false;

	struct
	{
		WebSocket<CMaxProto> *ws = nullptr;
	} m_wsRun;

	void __cdecl ConnectionWorker(void *);
	void __cdecl WsRunThread(void *param);
	void __cdecl PingWorker(void *);
	void InterruptibleSleepMs(DWORD msTotal, DWORD sliceMs = 200);
	void __cdecl LoadHistoryWorker(void *param);
	void __cdecl MessageAckWorker(void *param);
	void __cdecl PhoneSearchWorker(void *param);
	void __cdecl OfflineFileWorker(void *param);
	void __cdecl FileSendWorker(void *param);
	void EnsureDeviceId();
	/// Parse HTTP 101 `Sec-WebSocket-Extensions` (permessage-deflate parameters).
	void ApplyWsExtensionsFromHttp(MHttpResponse *pReply);
	bool SendHandshake(WebSocket<CMaxProto> *ws);
	bool SendJsonAndWait(WebSocket<CMaxProto> *ws, uint16_t opcode, JSONNode &payload, uint8_t cmd = 0, bool acceptPayloadError = false);
	void OnGatewayPush(const JSONNode &payload, int opcode);
	/// Opcode 135: server removed/hid a chat — clear local history for matching contact.
	void OnMaxPushChatRemoved(const JSONNode &payload);
	void TryIngestNotifMessagePayload(const JSONNode &payload);
	void TryIngestTypingPayload(const JSONNode &payload);
	void TryIngestPresencePayload(const JSONNode &payload, int opcode = -1);
	void ApplyPresenceToContact(MCONTACT hContact, const JSONNode &src);
	void TryMergeContactsFromPayload(const JSONNode &payload);
	void TryApplySyncPayloadFromPush(const JSONNode &payload);
	void IngestChatHistoryPayload(const JSONNode &payload, const char *szChatId, bool bMarkRead = false);
	uint64_t GetLastLocalMessageTimeMs(MCONTACT hContact);
	bool ApiPing(WebSocket<CMaxProto> *ws);
	bool ApiRequestQrCode(WebSocket<CMaxProto> *ws, CMStringA &outTrackId, CMStringA &outQrText);
	bool ApiPollQrStatus(WebSocket<CMaxProto> *ws, const char *szTrackId, bool &outApproved, bool &outExpired);
	bool ApiLoginByQrTrack(WebSocket<CMaxProto> *ws, const char *szTrackId, CMStringA &outToken);
	bool RunQrLoginFlow(WebSocket<CMaxProto> *ws);
	bool ApiSendTelemetryColdStart(WebSocket<CMaxProto> *ws);
	/// Optional web session bootstrap (requests.md): ping, token refresh ping, folders — before opcode 19 sync.
	bool ApiWebSessionBootstrap(WebSocket<CMaxProto> *ws);
	void DisconnectGateway();

	bool WaitForGatewayReady();
	bool HasLoginToken();
	CMStringW FormatLastError();
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnUserInfoInit(WPARAM, LPARAM);
	INT_PTR __cdecl SvcLoadServerHistory(WPARAM, LPARAM);
	INT_PTR __cdecl SvcOfflineFile(WPARAM, LPARAM);
	INT_PTR __cdecl SvcCanEmptyHistory(WPARAM hContact, LPARAM lParam);
	INT_PTR __cdecl SvcEmptyServerHistory(WPARAM hContact, LPARAM lParam);
	int __cdecl OnLangpackChanged(WPARAM, LPARAM);

public:
	CMaxProto(const char *szModuleName, const wchar_t *ptszUserName);
	~CMaxProto();

	INT_PTR GetCaps(int type, MCONTACT hContact = 0) override;
	int SetStatus(int iNewStatus) override;
	int SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char *msg) override;
	HANDLE SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	int UserIsTyping(MCONTACT hContact, int type) override;
	HANDLE SearchBasic(const wchar_t *id) override;
	HWND CreateExtendedSearchUI(HWND owner) override;
	HANDLE SearchAdvanced(HWND owner) override;
	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	void OnEventEdited(MCONTACT hContact, MEVENT, const DBEVENTINFO &dbei) override;
	void OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent, int flags) override;
	void OnSendOfflineFile(DB::EventInfo &dbei, DB::FILE_BLOB &blob, void *hTransfer) override;

	MWindow OnCreateAccMgrUI(MWindow hwndParent) override;
	void OnModulesLoaded() override;
	void OnShutdown(void) override;
	bool OnContactDeleted(MCONTACT hContact, uint32_t flags) override;

	void NotifyUser(const wchar_t *title, const wchar_t *text);
	void ShowQrCode(const CMStringA &qrText);
	void CloseQrDialog(bool bSuccess = false);
	void InitWsInflater();
	void FreeWsInflater();
	bool InflateWsFrame(const uint8_t *pData, size_t cbData, CMStringA &out);
	/// True when server sent `server_no_context_takeover` (each RSV1 frame is standalone deflate).
	bool IsWsPmDeflateIndependent() const { return m_bWsPmDeflateIndependent; }

	bool ApiSync(WebSocket<CMaxProto> *ws);
	bool ApiFetchContactsBatch(WebSocket<CMaxProto> *ws, const CMStringA *pUids, size_t nUids, bool bMarkAsContactsRoster = true);
	bool ApiFetchChatsByIds(WebSocket<CMaxProto> *ws, const CMStringA *pChatIds, size_t nIds);
	/// Opcode 49: message window around anchor `fromMs` (ms). Use forward>0 for newer-only gap fill; backward for older history.
	bool ApiFetchChatMessages(WebSocket<CMaxProto> *ws, const char *szChatId, int64_t fromMs, int forward, int backward, bool bMarkRead = false);
	bool ApiSendMessage(WebSocket<CMaxProto> *ws, const char *szChatId, const char *szText, CMStringA *pOutMsgId = nullptr);
	bool ApiSendFileMessage(WebSocket<CMaxProto> *ws, const char *szChatId, int64_t fileId, bool bPhoto, const char *szPhotoToken = nullptr, const char *szText = nullptr, CMStringA *pOutMsgId = nullptr);
	bool ApiSendMultiPhotoMessage(WebSocket<CMaxProto> *ws, const char *szChatId, const std::vector<CMStringA> &photoTokens, const char *szText = nullptr, CMStringA *pOutMsgId = nullptr);
	bool ApiSendTyping(WebSocket<CMaxProto> *ws, const char *szChatId, bool bTyping);
	bool ApiEditMessage(WebSocket<CMaxProto> *ws, const char *szChatId, const char *szMsgId, const char *szText);
	/// Opcode 80: request photo upload slots (url/token/fileId).
	bool ApiRequestPhotoUpload(WebSocket<CMaxProto> *ws, CMStringA &outUrl, CMStringA &outToken, int64_t &outFileId);
	bool ApiRequestFileUpload(WebSocket<CMaxProto> *ws, CMStringA &outUrl, CMStringA &outToken, int64_t &outFileId);
	bool ApiGetFileDownloadUrl(WebSocket<CMaxProto> *ws, const char *szChatId, const char *szMsgId, int64_t fileId, CMStringA &outUrl);
	/// Opcode 66: delete message on server. `bForMe` true = only this user; false = for everyone (if allowed).
	bool ApiDeleteMessages(WebSocket<CMaxProto> *ws, const char *szChatId, const char *szMsgId, bool bForMe);
	/// Opcode 46: resolve user by phone (E.164, UTF-8). On success sets outContact to contact node or empty if not found.
	bool ApiSearchByPhone(WebSocket<CMaxProto> *ws, const char *szPhoneUtf8, JSONNode &outContact);
	/// Opcode 34: add user to server-side contacts (roster).
	bool ApiAddContactOnServer(WebSocket<CMaxProto> *ws, const char *szUidDecimal);
	/// Opcode 34: remove user from server-side contacts (`action` mirrors ADD).
	bool ApiRemoveContactFromServer(WebSocket<CMaxProto> *ws, const char *szUidDecimal);
	/// Opcode 52: remove dialog/chat from server roster (PyMax `CHAT_DELETE` — not opcode 75 subscribe).
	bool ApiDeleteServerDialog(WebSocket<CMaxProto> *ws, const char *szChatId);
	/// Opcode 58: leave group/channel (PyMax `CHAT_LEAVE` — payload is chatId only; not 75).
	bool ApiChatLeave(WebSocket<CMaxProto> *ws, const char *szChatId);
	/// Opcode 16: update own profile (PyMax `PROFILE` / `ChangeProfilePayload`).
	bool ApiUpdateMyProfile(WebSocket<CMaxProto> *ws, const char *szFirstNameUtf8, const char *szLastNameUtf8, const char *szDescriptionUtf8);
	/// Push first/last name and biography to server (requires online); updates local settings on success.
	bool SaveMyProfile(const wchar_t *pwszFirstName, const wchar_t *pwszLastName, const wchar_t *pwszBio);

	void RegisterChatModule();
	void ApplySyncPayload(const JSONNode &payload, WebSocket<CMaxProto> *ws);
	CMStringW GetDefaultGroupW();
	MCONTACT FindContactByMaxUid(const char *szUid);
	/// Clear local message history and drop stored MaxChatId; keep the hContact (official: chat deleted but contact stays in address book).
	void ClearMaxDialogLocalHistory(MCONTACT hContact);
	/// Drop a 1:1 Max user from the local DB (e.g. removed on server / dialog gone from sync).
	void RemoveMaxUserContact(const char *szUid);
	/// If this peer is not in the last server address-book snapshot, remove local contact (chat-only ghost).
	void RemoveLocalPeerIfChatOnly(MCONTACT hContact);
	void ResetServerContactBookCache();
	void ApplyServerContactBookSnapshot(const std::vector<CMStringA> &uids);
	bool IsMaxUidInServerContactBook(const char *szUid);
	MCONTACT FindContactByDialogChatId(const char *szChatId);
	/// Opcode 49/64/67 need dialog `chatId`. Uses DB `MaxChatId` or derives 1:1 id as myUid XOR peer `MaxUid`.
	CMStringA GetOrResolveDialogChatId(MCONTACT hContact, bool bPersistIfDerived = true);
	/// 1:1 dialog message: match by chatId / derived id. If sender is own Max id (echo from another client), never use sender — use XOR to find peer.
	MCONTACT ResolveContactForDialogMessage(const char *szChatId, const char *senderUid);
	/// Ingest one USER message JSON (same shape as opcode 128 `payload.message` or chat `lastMessage`).
	void IngestMaxMessageJson(const JSONNode &message, const char *szChatId, bool bMarkRead = false);
	/// Opcode-128: merge `payload.chat` into 1:1 contact (title) before ingesting the message.
	void SyncLiveDialogFromPushPayload(const JSONNode &payload);
	bool ContactNeedsServerDisplayFetch(MCONTACT hContact);
	void QueueLiveNotifIngest(const JSONNode &payload);
	void __cdecl LiveNotifIngestWorker(void *param);

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
	void MergeContactJson(const JSONNode &c, const char *szRequestedUid = nullptr, bool bMarkAsContactsRoster = true);
	/// Read display name fields from a Max `contact` JSON object (same helpers as roster merge).
	void FillNameFromMaxContactJson(const JSONNode &c, CMStringW &outFn, CMStringW &outLn);

	class CMaxQRDlg *m_pQRDlg = nullptr;
};

struct CMPlugin : public ACCPROTOPLUGIN<CMaxProto>
{
	CMPlugin();
	int Load() override;
};

extern CMPlugin g_plugin;
