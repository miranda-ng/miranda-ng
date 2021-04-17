/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-21 George Hazan

*/

#if !defined(PROTO_H)
#define PROTO_H

struct WAChatInfo
{
	WAChatInfo(wchar_t *_jid, wchar_t *_nick) :
		tszJid(_jid), tszNick(_nick)
	{
		bActive = false;
	}

	ptrW tszJid, tszNick, tszOwner;
	bool bActive;

	MCONTACT hContact;
};

class WhatsAppProto;
typedef void (WhatsAppProto:: *WA_PKT_HANDLER)(const JSONNode &node);

struct WARequest
{
	int pktId;
	time_t issued;
	WA_PKT_HANDLER pHandler;
};

class WhatsAppProto : public PROTO<WhatsAppProto>
{
	bool m_bTerminated, m_bOnline;
	ptrW m_tszDefaultGroup;

	CMStringA m_szJid, m_szClientId, m_szClientToken;
	CMStringW m_tszAvatarFolder;

	MBinBuffer mac_key, enc_key;
	bool getBlob(const char *pSetting, MBinBuffer &buf);

	EVP_PKEY *m_pKeys; // private & public keys

	void CloseQrDialog();
	bool ShowQrCode(const CMStringA &ref);

	/// Network ////////////////////////////////////////////////////////////////////////////

	int m_iPktNumber;
	time_t m_iLoginTime;
	HNETLIBCONN m_hServerConn;

	mir_cs m_csPacketQueue;
	OBJLIST<WARequest> m_arPacketQueue;

	bool WSReadPacket(int nBytes, const WSHeader &hdr, MBinBuffer &buf);
	int  WSSend(const CMStringA &str, WA_PKT_HANDLER = nullptr);

	void OnLoggedIn(void);
	void OnLoggedOut(void);
	bool ServerThreadWorker(void);
	void ShutdownSession(void);

	bool ProcessChallenge(const CMStringA &szChallenge);
	bool ProcessSecret(const CMStringA &szSecret);

	/// Request handlers ///////////////////////////////////////////////////////////////////

	void OnRestoreSession1(const JSONNode &node);
	void OnRestoreSession2(const JSONNode &node);
	void OnStartSession(const JSONNode &node);

	void ProcessPacket(const JSONNode &node);
	void ProcessCmd(const JSONNode &node);
	void ProcessConn(const JSONNode &node);

	/// Avatars ////////////////////////////////////////////////////////////////////////////
	CMStringW GetAvatarFileName(MCONTACT hContact);

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

public:
	WhatsAppProto(const char *proto_name, const wchar_t *username);
	~WhatsAppProto();

	inline bool isOnline() const
	{	return m_bOnline;
	}

	inline bool isOffline() const
	{	return (m_iStatus == ID_STATUS_OFFLINE);
	}

	inline bool isInvisible() const
	{	return (m_iStatus == ID_STATUS_INVISIBLE);
	}

	class CWhatsAppQRDlg *m_pQRDlg;

	// PROTO_INTERFACE /////////////////////////////////////////////////////////////////////

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	HANDLE   SearchBasic(const wchar_t* id) override;
	int      SendMsg(MCONTACT hContact, int flags, const char* msg) override;
	int      SetStatus(int iNewStatus) override;
	int      UserIsTyping(MCONTACT hContact, int type) override;

	// Services ////////////////////////////////////////////////////////////////////////////

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);

	// Events //////////////////////////////////////////////////////////////////////////////

	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnUserInfo(WPARAM, LPARAM);
	int __cdecl OnBuildStatusMenu(WPARAM, LPARAM);

	// Processing Threads //////////////////////////////////////////////////////////////////

	void __cdecl SearchAckThread(void*);
	void __cdecl ServerThread(void*);

	// Contacts handling ///////////////////////////////////////////////////////////////////

	void RequestFriendship(MCONTACT hContact);
};

struct CMPlugin : public ACCPROTOPLUGIN<WhatsAppProto>
{
	signal_context *pCtx;

	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif
