/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-21 George Hazan

*/

#if !defined(PROTO_H)
#define PROTO_H

class WhatsAppProto;
typedef void (WhatsAppProto:: *WA_PKT_HANDLER)(const JSONNode &node);

struct WARequest
{
	int pktId;
	time_t issued;
	WA_PKT_HANDLER pHandler;
};

struct WAUser
{
	WAUser(MCONTACT _1, const char *_2) :
		hContact(_1),
		szId(mir_strdup(_2))
	{
	}

	~WAUser()
	{
		mir_free(szId);
	}

	MCONTACT hContact;
	DWORD dwModifyTag;
	char *szId;
	SESSION_INFO *si = 0;
	DWORD m_time1 = 0, m_time2 = 0;
};

struct WAMessage
{
	bool bFromTo;
	BYTE iMsgType;
	ptrA szShit;
	ptrA szJid;
	ptrA szMsgId;
	ptrA szBody;
	__int64 timestamp;
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
	bool getBlob(const char *pSetting, MBinBuffer &buf);

	// Contacts management /////////////////////////////////////////////////////////////////

	mir_cs m_csUsers;
	OBJLIST<WAUser> m_arUsers;

	WAUser* FindUser(const char *szId);
	WAUser* AddUser(const char *szId, bool bTemporary);

	// Group chats /////////////////////////////////////////////////////////////////////////

	void InitChat(WAUser *pUser, const JSONNode &chat);

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
	int  WSSend(const CMStringA &str, WA_PKT_HANDLER = nullptr);

	void OnLoggedIn(void);
	void OnLoggedOut(void);
	bool ServerThreadWorker(void);
	void ShutdownSession(void);

	bool ProcessChallenge(const CMStringA &szChallenge);
	bool ProcessSecret(const CMStringA &szSecret);

	bool decryptBinaryMessage(size_t cbSize, const void *buf, MBinBuffer &res);

	void SendKeepAlive();

	/// Request handlers ///////////////////////////////////////////////////////////////////

	void OnRestoreSession1(const JSONNode &node);
	void OnRestoreSession2(const JSONNode &node);
	void OnStartSession(const JSONNode &node);

	// binary packets
	void ProcessBinaryPacket(const MBinBuffer &buf);
	void ProcessAdd(const CMStringA &type, const JSONNode &node);
	void ProcessChats(const JSONNode &node);
	void ProcessContacts(const JSONNode &node);

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
	signal_context *pCtx;

	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif
