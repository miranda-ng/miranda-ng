/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#if !defined(PROTO_H)
#define PROTO_H

#define S_WHATSAPP_NET "@s.whatsapp.net"
#define APP_VERSION "2.2230.15"
#define KEY_BUNDLE_TYPE "\x05"

class WhatsAppProto;
typedef void (WhatsAppProto:: *WA_PKT_HANDLER)(const WANode &node);

struct WARequest
{
	WARequest(WA_PKT_HANDLER _1, void *_2 = nullptr) :
		pHandler(_1),
		pUserInfo(_2)
	{}

	WA_PKT_HANDLER pHandler;
	void *pUserInfo;
};

struct WAPersistentHandler
{
	WAPersistentHandler(const char *_1, const char *_2, const char *_3, WA_PKT_HANDLER _4) :
		pszType(_1), pszXmlns(_2), pszNode(_3), pHandler(_4)
	{}

	const char *pszType;
	const char *pszXmlns;
	const char *pszNode;
	WA_PKT_HANDLER pHandler;
};

struct WAHistoryMessage
{
	CMStringA jid, text;
	DWORD timestamp;
};

struct WAUser
{
	WAUser(MCONTACT _1, const char *_2) :
		hContact(_1),
		szId(mir_strdup(_2)),
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
	bool bInited = false;
	SESSION_INFO *si = 0;
	DWORD m_time1 = 0, m_time2 = 0;
	OBJLIST<WAHistoryMessage> arHistory;
};

struct WAOwnMessage
{
	WAOwnMessage(int _1, MCONTACT _2, const char *_3) :
		pktId(_1),
		hContact(_2),
		szPrefix(_3)
	{}

	int pktId;
	MCONTACT hContact;
	CMStringA szPrefix;
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
	} noiseKeys, signedIdentity, ephemeral;

	struct {
		MBinBuffer priv, pub, signature;
		uint32_t keyid;
	} preKey;

	void deriveKey(const void *pData, size_t cbLen, MBinBuffer &write, MBinBuffer &read);
	void mixIntoKey(const void *n, const void *p);
	void updateHash(const void *pData, size_t cbLen);

public:
	WANoise(WhatsAppProto *_ppro);

	void finish();
	void init();

	MBinBuffer decrypt(const void *pData, size_t cbLen);
	MBinBuffer encrypt(const void *pData, size_t cbLen);

	size_t     decodeFrame(const void *&pData, size_t &cbLen);
	MBinBuffer encodeFrame(const void *pData, size_t cbLen);
};

class WhatsAppProto : public PROTO<WhatsAppProto>
{
	friend class WANoise;

	class CWhatsAppProtoImpl
	{
		friend class WhatsAppProto;
		WhatsAppProto &m_proto;

		CTimer m_keepAlive;
		void OnKeepAlive(CTimer *) {
			m_proto.SendKeepAlive();
		}

		CWhatsAppProtoImpl(WhatsAppProto &pro) :
			m_proto(pro),
			m_keepAlive(Miranda_GetSystemWindow(), UINT_PTR(this))
		{
			m_keepAlive.OnEvent = Callback(this, &CWhatsAppProtoImpl::OnKeepAlive);
		}
	} m_impl;


	bool m_bTerminated, m_bRespawn;
	ptrW m_tszDefaultGroup;

	CMStringA m_szJid;
	CMStringW m_tszAvatarFolder;

	EVP_PKEY *m_pKeys; // private & public keys
	WANoise *m_noise;

	MBinBuffer getBlob(const char *pSetting);

	// Contacts management /////////////////////////////////////////////////////////////////

	mir_cs m_csUsers;
	OBJLIST<WAUser> m_arUsers;

	mir_cs m_csOwnMessages;
	OBJLIST<WAOwnMessage> m_arOwnMsgs;

	WAUser* FindUser(const char *szId);
	WAUser* AddUser(const char *szId, bool bTemporary);

	// Group chats /////////////////////////////////////////////////////////////////////////

	void InitChat(WAUser *pUser);

	// UI //////////////////////////////////////////////////////////////////////////////////

	void CloseQrDialog();
	bool ShowQrCode(const CMStringA &ref);

	/// Network ////////////////////////////////////////////////////////////////////////////

	time_t m_iLoginTime;
	HNETLIBCONN m_hServerConn;

	mir_cs m_csPacketQueue;
	OBJLIST<WARequest> m_arPacketQueue;

	LIST<WAPersistentHandler> m_arPersistent;
	WA_PKT_HANDLER FindPersistentHandler(const WANode &node);

	bool WSReadPacket(const WSHeader &hdr, MBinBuffer &buf);
	int  WSSend(const MessageLite &msg);
	int  WSSendNode(WANode &node, WA_PKT_HANDLER = nullptr);

	void OnLoggedIn(void);
	void OnLoggedOut(void);
	void ServerThreadWorker(void);
	void ShutdownSession(void);

	void SendKeepAlive();

	/// Request handlers ///////////////////////////////////////////////////////////////////

	void OnGetAvatarInfo(const JSONNode &node, void*);
	void OnGetChatInfo(const JSONNode &node, void*);
	void OnSendMessage(const JSONNode &node, void*);

	void OnProcessHandshake(const void *pData, int cbLen);
	
	void InitPersistentHandlers();
	void OnIqPairDevice(const WANode &node);
	void OnIqPairSuccess(const WANode &node);
	void OnStreamError(const WANode &node);

	// binary packets
	void ProcessBinaryPacket(const void *pData, size_t cbLen);
	void ProcessBinaryNode(const WANode &node);

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

	CMOption<wchar_t*> m_wszDefaultGroup;  // clist group to store contacts
	CMOption<bool>     m_bHideGroupchats;  // do not open chat windows on creation

	// Processing Threads //////////////////////////////////////////////////////////////////

	void __cdecl SearchAckThread(void*);
	void __cdecl ServerThread(void*);
};

struct CMPlugin : public ACCPROTOPLUGIN<WhatsAppProto>
{
	signal_context *pCtx = nullptr;
	
	HNETLIBUSER hAvatarUser = nullptr;
	HNETLIBCONN hAvatarConn = nullptr;
	bool SaveFile(const char *pszUrl, PROTO_AVATAR_INFORMATION &ai);

	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif
