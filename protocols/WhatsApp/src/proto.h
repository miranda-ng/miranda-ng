/*

WhatsApp plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#if !defined(PROTO_H)
#define PROTO_H

#define S_WHATSAPP_NET "@s.whatsapp.net"
#define APP_VERSION "2.2230.15"
#define KEY_BUNDLE_TYPE "\x05"

class WhatsAppProto;
typedef void (WhatsAppProto:: *WA_PKT_HANDLER)(const WANode &node);

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

struct WARequest
{
	WARequest(const CMStringA &_1, WA_PKT_HANDLER _2, void *_3 = nullptr) :
		szPacketId(_1),
		pHandler(_2),
		pUserInfo(_3)
	{}

	CMStringA szPacketId;
	WA_PKT_HANDLER pHandler;
	void *pUserInfo;
};

struct WADevice
{
	WADevice(const char *_1, int _2) :
		jid(_1),
		key_index(_2)
	{}

	WAJid jid;
	int key_index;
};

struct WAPersistentHandler
{
	WAPersistentHandler(const char *_1, const char *_2, const char *_3, const char *_4, WA_PKT_HANDLER _5) :
		pszTitle(_1), pszType(_2), pszXmlns(_3), pszChild(_4), pHandler(_5)
	{}

	const char *pszTitle, *pszType, *pszXmlns, *pszChild;
	WA_PKT_HANDLER pHandler;
};

struct WAHistoryMessage
{
	CMStringA jid, text;
	DWORD timestamp;
};

struct WAUser
{
	WAUser(MCONTACT _1, const char *_2, bool _3 = false) :
		hContact(_1),
		szId(mir_strdup(_2)),
		bIsGroupChat(_3),
		arHistory(1)
	{
	}

	~WAUser()
	{
		mir_free(szId);
	}

	MCONTACT hContact;
	DWORD dwModifyTag = 0;
	char *szId;
	bool bInited = false, bIsGroupChat;
	SESSION_INFO *si = 0;
	DWORD m_time1 = 0, m_time2 = 0;
	OBJLIST<WAHistoryMessage> arHistory;
};

struct WAOwnMessage
{
	WAOwnMessage(int _1, const char *_2, const char *_3) :
		pktId(_1),
		szJid(_2),
		szPrefix(_3)
	{}

	int pktId;
	CMStringA szPrefix, szJid;
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

	WhatsAppProto *ppro;
	int readCounter = 0, writeCounter = 0;
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
	MBinBuffer sessionData, userData;

	MSignalSession(const CMStringA &_1, int _2);
	~MSignalSession();

	bool hasAddress(const char *name, size_t name_len) const;

	__forceinline session_cipher* getCipher(void) const { return cipher; }
	__forceinline int getDeviceId() const { return address.device_id; }
};

class MSignalStore
{
	void init();

	signal_context *m_pContext;
	signal_protocol_store_context *m_pStore;

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
	} preKey;

	MSignalStore(PROTO_INTERFACE *_1, const char *_2);
	~MSignalStore();

	__forceinline signal_context *CTX() const { return m_pContext; }

	MSignalSession *createSession(const CMStringA &szName, int deviceId);

	signal_buffer* decryptSignalProto(const CMStringA &from, const char *pszType, const MBinBuffer &encrypted);
	signal_buffer* decryptGroupSignalProto(const CMStringA &from, const CMStringA &author, const MBinBuffer &encrypted);

	signal_buffer* encryptSignalProto(const WAJid &to, const MBinBuffer &buf, int &type);

	MBinBuffer encodeSignedIdentity(bool);
	void generatePrekeys(int count);

	void logError(int code, const char *szMessage);

	void processSenderKeyMessage(const Wa__Message__SenderKeyDistributionMessage *msg);
};

class WhatsAppProto : public PROTO<WhatsAppProto>
{
	friend class WANoise;

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

	bool m_bTerminated, m_bRespawn, m_bUpdatedPrekeys;
	ptrW m_tszDefaultGroup;

	CMStringA m_szJid;
	CMStringW m_tszAvatarFolder;

	EVP_PKEY *m_pKeys; // private & public keys
	WANoise *m_noise;

	void UploadMorePrekeys();

	// App state management
	OBJLIST<WACollection> m_arCollections;

	void InitSync(void);
	void ApplyPatch(const JSONNode &index, const Wa__SyncActionValue *data);
	void ParsePatch(WACollection *pColl, const Wa__SyncdRecord *rec, bool bSet);
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

	OBJLIST<WADevice> m_arDevices;

	WAUser* FindUser(const char *szId);
	WAUser* AddUser(const char *szId, bool bTemporary, bool isChat = false);

	// Group chats /////////////////////////////////////////////////////////////////////////

	void InitChat(WAUser *pUser);

	// UI //////////////////////////////////////////////////////////////////////////////////

	void CloseQrDialog();
	bool ShowQrCode(const CMStringA &ref);

	/// Network ////////////////////////////////////////////////////////////////////////////

	time_t m_lastRecvTime;
	HNETLIBCONN m_hServerConn;

	mir_cs m_csPacketQueue;
	OBJLIST<WARequest> m_arPacketQueue;

	LIST<WAPersistentHandler> m_arPersistent;
	WA_PKT_HANDLER FindPersistentHandler(const WANode &node);

	int m_iPacketId;
	uint16_t m_wMsgPrefix[2];
	CMStringA GenerateMessageId();
	void ProcessMessage(WAMSG type, const Wa__WebMessageInfo &msg);
	bool CreateMsgParticipant(WANode *pParticipants, const WAJid &jid, const MBinBuffer &orig);

	void ProcessReceipt(MCONTACT hContact, const char *msgId, bool bRead);

	bool WSReadPacket(const WSHeader &hdr, MBinBuffer &buf);
	int  WSSend(const ProtobufCMessage *msg);
	int  WSSendNode(WANode &node, WA_PKT_HANDLER = nullptr);

	MBinBuffer DownloadEncryptedFile(const char *url, const ProtobufCBinaryData &mediaKeys, const char *pszType);
	CMStringW  GetTmpFileName(const char *pszClass, const char *addition);

	void OnLoggedIn(void);
	void OnLoggedOut(void);
	void ServerThreadWorker(void);
	void ShutdownSession(void);

	void SendAck(const WANode &node);
	void SendReceipt(const char *pszTo, const char *pszParticipant, const char *pszId, const char *pszType);
	void SendKeepAlive();
	int  SendTextMessage(const char *jid, const char *pszMsg);
	void SetServerStatus(int iStatus);

	/// Popups /////////////////////////////////////////////////////////////////////////////

	HANDLE m_hPopupClass;
	CMOption<bool> m_bUsePopups;

	void InitPopups(void);
	void Popup(MCONTACT hContact, const wchar_t *szMsg, const wchar_t *szTitle);

	/// Request handlers ///////////////////////////////////////////////////////////////////

	void OnGetAvatarInfo(const JSONNode &node, void*);
	void OnGetChatInfo(const JSONNode &node, void*);

	void OnProcessHandshake(const uint8_t *pData, int cbLen);
	
	void InitPersistentHandlers();
	void OnAccountSync(const WANode &node);
	void OnIqBlockList(const WANode &node);
	void OnIqCountPrekeys(const WANode &node);
	void OnIqDoNothing(const WANode &node);
	void OnIqGetUsync(const WANode &node);
	void OnIqPairDevice(const WANode &node);
	void OnIqPairSuccess(const WANode &node);
	void OnIqResult(const WANode &node);
	void OnIqServerSync(const WANode &node);
	void OnNotifyAny(const WANode &node);
	void OnNotifyDevices(const WANode &node);
	void OnNotifyEncrypt(const WANode &node);
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

	// unzip operations
	MBinBuffer unzip(const MBinBuffer &src);

	/// Avatars ////////////////////////////////////////////////////////////////////////////
	CMStringW GetAvatarFileName(MCONTACT hContact);

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

public:
	WhatsAppProto(const char *proto_name, const wchar_t *username);
	~WhatsAppProto();

	__forceinline bool isOnline() const
	{	return m_hServerConn != 0;
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
	int      SendMsg(MCONTACT hContact, int flags, const char* msg) override;
	int      SetStatus(int iNewStatus) override;
	int      UserIsTyping(MCONTACT hContact, int type) override;

	void     OnModulesLoaded() override;

	// Services ////////////////////////////////////////////////////////////////////////////

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);

	// Events //////////////////////////////////////////////////////////////////////////////

	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnBuildStatusMenu(WPARAM, LPARAM);

	// Options /////////////////////////////////////////////////////////////////////////////

	CMOption<wchar_t*> m_wszNick;				// your nick name in presence
	CMOption<wchar_t*> m_wszDefaultGroup;  // clist group to store contacts
	CMOption<bool>     m_bHideGroupchats;  // do not open chat windows on creation

	// Processing Threads //////////////////////////////////////////////////////////////////

	void __cdecl SearchAckThread(void*);
	void __cdecl ServerThread(void*);
};

struct CMPlugin : public ACCPROTOPLUGIN<WhatsAppProto>
{
	HNETLIBUSER hAvatarUser = nullptr;
	HNETLIBCONN hAvatarConn = nullptr;
	bool SaveFile(const char *pszUrl, PROTO_AVATAR_INFORMATION &ai);

	bool bHasMessageState = false;

	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif
