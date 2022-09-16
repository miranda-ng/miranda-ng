/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#if !defined(PROTO_H)
#define PROTO_H

class WhatsAppProto;
typedef void (WhatsAppProto:: *WA_PKT_HANDLER)(const JSONNode &node, void*);

struct WARequest
{
	CMStringA szPrefix;
	WA_PKT_HANDLER pHandler;
	void *pUserInfo;
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
	int readCounter = 0, writeCounter = 0;
	bool bInitFinished = false, bSendIntro = false;
	MBinBuffer pubKey, privKey, salt, encKey, decKey;
	uint8_t hash[32];

	void decrypt(const void *pData, size_t cbLen, MBinBuffer &dest);
	void encrypt(const void *pData, size_t cbLen, MBinBuffer &dest);
	void deriveKey(const void *pData, size_t cbLen, MBinBuffer &write, MBinBuffer &read);
	void mixIntoKey(const void *pData, size_t cbLen);
	void updateHash(const void *pData, size_t cbLen);

public:
	WANoise();

	void finish() { bInitFinished = true; }

	const MBinBuffer& getPub() const { return pubKey; }

	bool decodeFrame(const void *pData, size_t cbLen);
	void encodeFrame(const void *pData, size_t cbLen, MBinBuffer &dest);
};

class WhatsAppProto : public PROTO<WhatsAppProto>
{
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


	bool m_bTerminated;
	ptrW m_tszDefaultGroup;

	CMStringA m_szJid, m_szClientId, m_szClientToken;
	CMStringW m_tszAvatarFolder;

	EVP_PKEY *m_pKeys; // private & public keys
	MBinBuffer mac_key, enc_key;
	
	WANoise *m_noise;

	bool getBlob(const char *pSetting, MBinBuffer &buf);

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

	int m_iPktNumber;
	time_t m_iLoginTime;
	HNETLIBCONN m_hServerConn;

	mir_cs m_csPacketQueue;
	OBJLIST<WARequest> m_arPacketQueue;

	bool WSReadPacket(const WSHeader &hdr, MBinBuffer &buf);
	int  WSSend(const MessageLite &msg, WA_PKT_HANDLER = nullptr, void *pUserIndo = nullptr);
	int  WSSendNode(const char *pszPrefix, int flags, WANode &node, WA_PKT_HANDLER = nullptr);

	void OnLoggedIn(void);
	void OnLoggedOut(void);
	bool ServerThreadWorker(void);
	void ShutdownSession(void);

	bool ProcessChallenge(const CMStringA &szChallenge);
	bool ProcessSecret(const CMStringA &szSecret);

	bool decryptBinaryMessage(size_t cbSize, const void *buf, MBinBuffer &res);

	void SendKeepAlive();

	/// Request handlers ///////////////////////////////////////////////////////////////////

	void OnGetAvatarInfo(const JSONNode &node, void*);
	void OnGetChatInfo(const JSONNode &node, void*);
	void OnRestoreSession1(const JSONNode &node, void*);
	void OnRestoreSession2(const JSONNode &node, void*);
	void OnSendMessage(const JSONNode &node, void*);
	void OnStartSession(const JSONNode &node, void*);

	// binary packets
	void ProcessBinaryPacket(const uint8_t *pData, size_t cbLen);

	// text packets
	void ProcessPacket(const JSONNode &node);
	void ProcessBlocked(const JSONNode &node);
	void ProcessCmd(const JSONNode &node);
	void ProcessConn(const JSONNode &node);
	void ProcessPresence(const JSONNode &node);

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
