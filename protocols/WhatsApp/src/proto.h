/*

WhatsApp plugin for Miranda NG
Copyright � 2019-25 George Hazan

*/

#if !defined(PROTO_H)
#define PROTO_H

#define WA_PROTO_MAJOR 2
#define WA_PROTO_MINOR 3000
#define WA_PROTO_BUILD 1027934701
#define WA_APPVERSION  WA_PROTO_MAJOR,WA_PROTO_MINOR,WA_PROTO_BUILD

#define WA_STRINGIFY_IMPL(x)      #x
#define WA_STRINGIFY(x)           WA_STRINGIFY_IMPL(x)

#define WA_APPVERSION_STRING      WA_STRINGIFY(WA_APPVERSION)

#define S_WHATSAPP_NET "@s.whatsapp.net"
#define KEY_BUNDLE_TYPE "\x05"

class WhatsAppProto;
typedef void (WhatsAppProto:: *WA_PKT_HANDLER)(const WANode &node);
typedef void (WhatsAppProto:: *WA_PKT_HANDLER_FULL)(const WANode &node, void *pUserInfo);

struct WAMSG
{
	union {
		uint32_t dwFlags = 0;
		struct {
			bool bPrivateChat : 1;
			bool bGroupChat : 1;
			bool bDirectStatus : 1;
			bool bOtherStatus : 1;
			bool bPeerBroadcast : 1;
			bool bOtherBroadcast : 1;
			bool bOffline : 1;
		};
	};
};

struct WAMediaKeys
{
	WAMediaKeys(const uint8_t *pKey, size_t keyLen, const char *pszMediaType);

	uint8_t iv[16];
	uint8_t cipherKey[32];
	uint8_t macKey[64];
};

/////////////////////////////////////////////////////////////////////////////////////////
// own requests

struct WARequestBase
{
	WARequestBase(const CMStringA &_1) :
		szPacketId(_1)
	{}
	virtual ~WARequestBase() {}

	CMStringA szPacketId;

	virtual void Execute(WhatsAppProto *ppro, const WANode &node) = 0;
};

class WARequestSimple : public WARequestBase
{
	WA_PKT_HANDLER pHandler;

public:
	WARequestSimple(const CMStringA &_1, WA_PKT_HANDLER _2) :
		WARequestBase(_1),
		pHandler(_2)
	{}

	void Execute(WhatsAppProto *ppro, const WANode &node) override
	{
		(ppro->*pHandler)(node);
	}
};

class WARequestParam : public WARequestBase
{
	WA_PKT_HANDLER_FULL pHandler;
	void *pUserInfo;

public:
	WARequestParam(const CMStringA &_1, WA_PKT_HANDLER_FULL _2, void *_3) :
		WARequestBase(_1),
		pHandler(_2),
		pUserInfo(_3)
	{}

	void Execute(WhatsAppProto *ppro, const WANode &node) override
	{
		(ppro->*pHandler)(node, pUserInfo);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

struct WAPersistentHandler
{
	WAPersistentHandler(const char *_1, const char *_2, const char *_3, const char *_4, WA_PKT_HANDLER _5) :
		pszTitle(_1), pszType(_2), pszXmlns(_3), pszChild(_4), pHandler(_5)
	{}

	const char *pszTitle, *pszType, *pszXmlns, *pszChild;
	WA_PKT_HANDLER pHandler;
};

struct WAUser
{
	WAUser(MCONTACT _1, const char *_2, bool _3 = false) :
		hContact(_1),
		szId(mir_strdup(_2)),
		bIsGroupChat(_3),
		arDevices(1)
	{
	}

	~WAUser()
	{
		mir_free(szId);
	}

	MCONTACT hContact;
	DWORD dwModifyTag = 0;
	char *szId;
	bool bInited = false, bIsGroupChat, bDeviceInit = false;
	SESSION_INFO *si = 0;
	OBJLIST<WAJid> arDevices;
	time_t m_timer1 = 0, m_timer2 = 0;
};

struct WAOwnMessage
{
	WAOwnMessage(int _1, const char *_2, const char *_3) :
		pktId(_1),
		szJid(_2),
		szMessageId(_3)
	{}

	int pktId;
	CMStringA szJid, szMessageId;
};

struct WACollection
{
	WACollection(const char *_1, int _2 = 0) :
		szName(mir_strdup(_1)),
		version(_2)
	{}

	ptrA szName;
	int  version;

	LT_HASH hash;
	std::map<std::string, std::string> indexValueMap;
};

class WANoise
{
	friend class WhatsAppProto;
	friend class WebSocket<WhatsAppProto>;

	WhatsAppProto *ppro;
	uint32_t readCounter = 0, writeCounter = 0;
	bool bInitFinished = false, bSendIntro = false;
	MBinBuffer salt, encKey, decKey;
	uint8_t hash[32];

	struct {
		MBinBuffer priv, pub;
	} noiseKeys, ephemeral;

	void deriveKey(const void *pData, size_t cbLen, MBinBuffer &write, MBinBuffer &read);
	void mixIntoKey(const void *n, const void *p);
	void updateHash(const void *pData, size_t cbLen);

public:
	WANoise(WhatsAppProto *_ppro);

	void finish();
	void init();

	MBinBuffer decrypt(const void *pData, size_t cbLen);
	MBinBuffer encrypt(const void *pData, size_t cbLen);

	size_t     decodeFrame(const uint8_t *&pData, size_t &cbLen);
	MBinBuffer encodeFrame(const void *pData, size_t cbLen);
};

class MSignalSession : public MZeroedObject
{
	friend class MSignalStore;
	signal_protocol_address address;
	session_cipher *cipher = nullptr;

public:
	CMStringA szName;
	MBinBuffer sessionData;

	MSignalSession(const CMStringA &_1, int _2);
	~MSignalSession();

	bool hasAddress(const char *name, size_t name_len) const;

	__forceinline session_cipher* getCipher(void) const { return cipher; }
	__forceinline int getDeviceId() const { return address.device_id; }
	CMStringA getSetting() const;
};

class MSignalStore
{
	void init();

	signal_context *m_pContext;
	signal_protocol_store_context *m_pStore;

	void importPublicKey(ec_public_key **result, MBinBuffer &buf);

public:
	PROTO_INTERFACE *pProto;
	const char *prefix;

	OBJLIST<MSignalSession> arSessions;

	struct
	{
		MBinBuffer priv, pub;
	}
		signedIdentity;

	struct
	{
		MBinBuffer priv, pub, signature;
		uint32_t keyid;
	}
		preKey;

	MSignalStore(PROTO_INTERFACE *_1, const char *_2);
	~MSignalStore();

	__forceinline signal_context *CTX() const { return m_pContext; }

	MSignalSession* createSession(const CMStringA &szName, int deviceId);
	MSignalSession* getSession(const signal_protocol_address *address);
	void injectSession(const char *szJid, const WANode *pNode, const WANode *pKey);

	MBinBuffer decryptSignalProto(const CMStringA &from, const char *pszType, const MBinBuffer &encrypted);
	MBinBuffer decryptGroupSignalProto(const CMStringA &from, const CMStringA &author, const MBinBuffer &encrypted);

	MBinBuffer encryptSenderKey(const WAJid &to, const CMStringA &from, const MBinBuffer &buf, MBinBuffer &skmsgKey);
	MBinBuffer encryptSignalProto(const WAJid &to, const MBinBuffer &buf, int &type);

	MBinBuffer encodeSignedIdentity(bool);
	void generatePrekeys(int count);

	void logError(int code, const char *szMessage);

	void processSenderKeyMessage(const CMStringA &author, const Wa__Message__SenderKeyDistributionMessage *msg);
};

class WhatsAppProto : public PROTO<WhatsAppProto>
{
	friend class WANoise;
	friend class CWhatsAppQRDlg;
	friend class COptionsDlg;
	friend class WebSocket<WhatsAppProto>;

	class CWhatsAppProtoImpl
	{
		friend class WhatsAppProto;
		WhatsAppProto &m_proto;

		CTimer m_keepAlive, m_resyncApp;
		void OnKeepAlive(CTimer *)
		{	m_proto.SendKeepAlive();
		}
		void OnResync(CTimer *pTimer)
		{
			pTimer->Stop();
			m_proto.ResyncAll();
		}

		CWhatsAppProtoImpl(WhatsAppProto &pro) :
			m_proto(pro),
			m_keepAlive(Miranda_GetSystemWindow(), UINT_PTR(this)),
			m_resyncApp(Miranda_GetSystemWindow(), UINT_PTR(this)+1)
		{
			m_keepAlive.OnEvent = Callback(this, &CWhatsAppProtoImpl::OnKeepAlive);
			m_resyncApp.OnEvent = Callback(this, &CWhatsAppProtoImpl::OnResync);
		}
	} m_impl;

	bool m_bTerminated, m_bRespawn, m_bUpdatedPrekeys, m_bUnregister;
	ptrW m_tszDefaultGroup;

	CMStringA m_szJid;

	EVP_PKEY *m_pKeys; // private & public keys
	WANoise *m_noise;

	void UploadMorePrekeys();

	// App state management
	OBJLIST<WACollection> m_arCollections;

	void InitSync(void);
	void ApplyPatch(const JSONNode &index, const Wa__SyncActionValue *data);
	void ParsePatch(WACollection *pColl, const Wa__SyncdRecord *rec, bool bSet);
	void ProcessHistorySync(const Wa__HistorySync *pSync);
	void RemoveCachedSettings();
	void ResyncServer(const OBJLIST<WACollection> &task);
	void ResyncAll(void);

	__forceinline WACollection *FindCollection(const char *pszName)
	{	return m_arCollections.find((WACollection *)&pszName);
	}

	// Contacts management /////////////////////////////////////////////////////////////////

	mir_cs m_csUsers;
	OBJLIST<WAUser> m_arUsers;

	mir_cs m_csOwnMessages;
	OBJLIST<WAOwnMessage> m_arOwnMsgs;

	WAUser* FindUser(const char *szId);
	WAUser* AddUser(const char *szId, bool bTemporary);

	// Group chats /////////////////////////////////////////////////////////////////////////

	void GC_RefreshMetadata();
	void GC_GetAllMetadata();
	void GC_ParseMetadata(const WANode *pGroup);

	int  __cdecl GcEventHook(WPARAM, LPARAM);
	int  __cdecl GcMenuHook(WPARAM, LPARAM);

	// UI //////////////////////////////////////////////////////////////////////////////////

	void CloseQrDialog();
	bool ShowQrCode(const CMStringA &ref);

	/// Network ////////////////////////////////////////////////////////////////////////////

	time_t m_lastRecvTime;
	WebSocket<WhatsAppProto> *m_ws;

	mir_cs m_csPacketQueue;
	OBJLIST<WARequestBase> m_arPacketQueue;

	LIST<WAPersistentHandler> m_arPersistent;
	WA_PKT_HANDLER FindPersistentHandler(const WANode &node);

	int m_iPacketId;
	uint16_t m_wMsgPrefix[2];
	CMStringA GenerateMessageId();
	CMStringA GetMessageText(const Wa__Message *pMessage);
	void GetMessageContent(CMStringA &txt, const char *szType, const char *szUrl, const char *szMimetype, const char *szDirectPath, const ProtobufCBinaryData &szMediaKey, const char *szCaption = nullptr);
	void ProcessMessage(WAMSG type, const Wa__WebMessageInfo &msg);
	bool CreateMsgParticipant(WANode *pParticipants, const WAJid &jid, const MBinBuffer &orig);

	void ProcessReceipt(MCONTACT hContact, const char *msgId, bool bRead);

	int  WSSend(const ProtobufCMessage &msg);
	int  WSSendNode(WANode &node);
	int  WSSendNode(WANode &node, WA_PKT_HANDLER);
	int  WSSendNode(WANode &node, WA_PKT_HANDLER_FULL, void *pUserInfo);

	MBinBuffer DownloadEncryptedFile(const char *url, const ProtobufCBinaryData &mediaKeys, const char *pszType);
	CMStringW  GetTmpFileName(const char *pszClass, const char *addition);

	void OnLoggedIn(void);
	void OnLoggedOut(void);
	void ProcessFailure(int code);
	void ServerThreadWorker(void);
	void ShutdownSession(void);

	void SendAck(const WANode &node);
	void SendReceipt(const char *pszTo, const char *pszParticipant, const char *pszId, const char *pszType);
	void SendKeepAlive(void);
	int  SendTextMessage(const char *jid, const char *pszMsg);
	void SendUnregister(void);
	void SendUsync(const LIST<char> &jids, void *pUserInfo);
	void SetServerStatus(int iStatus);

	void FinishTask(WASendTask *pTask);
	void SendTask(WASendTask *pTask);

	/// Popups /////////////////////////////////////////////////////////////////////////////

	HANDLE m_hPopupClass;
	CMOption<bool> m_bUsePopups;

	void InitPopups(void);
	void Popup(MCONTACT hContact, const wchar_t *szMsg, const wchar_t *szTitle);

	/// Request handlers ///////////////////////////////////////////////////////////////////

	void OnProcessHandshake(const uint8_t *pData, int cbLen);
	
	void InitPersistentHandlers();
	void OnAccountSync(const WANode &node);
	void OnIqBlockList(const WANode &node);
	void OnIqCountPrekeys(const WANode &node);
	void OnIqDoNothing(const WANode &node);
	void OnIqGcGetAllMetadata(const WANode &node);
	void OnIqGetAvatar(const WANode &node);
	void OnIqGetKeys(const WANode &node, void *pUserInfo);
	void OnIqGetUsync(const WANode &node, void *pUserInfo);
	void OnIqPairDevice(const WANode &node);
	void OnIqPairSuccess(const WANode &node);
	void OnIqResult(const WANode &node);
	void OnIqServerSync(const WANode &node);
	void OnNotifyAny(const WANode &node);
	void OnNotifyDevices(const WANode &node);
	void OnNotifyEncrypt(const WANode &node);
	void OnNotifyPicture(const WANode &node);
	void OnReceiveAck(const WANode &node);
	void OnReceiveCall(const WANode &node);
	void OnReceiveChatState(const WANode &node);
	void OnReceiveFailure(const WANode &node);
	void OnReceiveInfo(const WANode &node);
	void OnReceiveMessage(const WANode &node);
	void OnReceiveReceipt(const WANode &node);
	void OnServerSync(const WANode &node);
	void OnStreamError(const WANode &node);
	void OnSuccess(const WANode &node);

	// Signal
	MSignalStore m_signalStore;

	// Binary packets
	void ProcessBinaryPacket(const uint8_t *pData, size_t cbLen);

	/// Avatars ////////////////////////////////////////////////////////////////////////////
	CMStringW GetAvatarFileName(MCONTACT hContact);
	void      ServerFetchAvatar(const char *jid);

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

public:
	WhatsAppProto(const char *proto_name, const wchar_t *username);
	~WhatsAppProto();

	__forceinline bool isOnline() const
	{	return m_ws != 0;
	}

	__forceinline void writeStr(const char *pszSetting, const JSONNode &node)
	{
		CMStringW str(node.as_mstring());
		if (!str.IsEmpty())
			setWString(pszSetting, str);
	}

	class CWhatsAppQRDlg *m_pQRDlg;

	// PROTO_INTERFACE /////////////////////////////////////////////////////////////////////

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	HANDLE   SearchBasic(const wchar_t* id) override;
	int      SendMsg(MCONTACT hContact, MEVENT, const char* msg) override;
	int      SetStatus(int iNewStatus) override;
	int      UserIsTyping(MCONTACT hContact, int type) override;

	void     OnCacheInit() override;
	MWindow  OnCreateAccMgrUI(MWindow) override;
	void     OnErase() override;

	// Events //////////////////////////////////////////////////////////////////////////////

	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnBuildStatusMenu(WPARAM, LPARAM);

	// Options /////////////////////////////////////////////////////////////////////////////

	CMOption<wchar_t*> m_wszNick;				// your nick name in presence
	CMOption<wchar_t*> m_wszDeviceName;    // how do you see Miranda in mobile phone
	CMOption<wchar_t*> m_wszDefaultGroup;  // clist group to store contacts
	CMOption<bool>     m_bHideGroupchats;  // do not open chat windows on creation
	CMOption<bool>     m_bUseBbcodes;      // use extended markup for messages

	// Processing Threads //////////////////////////////////////////////////////////////////

	void __cdecl SearchAckThread(void*);
	void __cdecl ServerThread(void*);
};

struct CMPlugin : public ACCPROTOPLUGIN<WhatsAppProto>
{
	HNETLIBUSER hAvatarUser = nullptr;
	HNETLIBCONN hAvatarConn = nullptr;
	bool SaveFile(const char *pszUrl, PROTO_AVATAR_INFORMATION &ai);

	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif
