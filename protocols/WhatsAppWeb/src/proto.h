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

struct WAUser
{
	WAUser(MCONTACT _1, const char *_2) :
		hContact(_1),
		szId(mir_strdup(_2))
	{}

	~WAUser()
	{
		mir_free(szId);
	}

	MCONTACT hContact;
	char *szId;
};

class WhatsAppProto : public PROTO<WhatsAppProto>
{
	bool m_bTerminated, m_bOnline;
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

	// UI //////////////////////////////////////////////////////////////////////////////////

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

	bool decryptBinaryMessage(size_t cbSize, const void *buf, MBinBuffer &res);

	/// Request handlers ///////////////////////////////////////////////////////////////////

	void OnRestoreSession1(const JSONNode &node);
	void OnRestoreSession2(const JSONNode &node);
	void OnStartSession(const JSONNode &node);

	void ProcessBinaryPacket(const MBinBuffer &buf);

	void ProcessPacket(const JSONNode &node);
	void ProcessBlocked(const JSONNode &node);
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

	void     OnModulesLoaded() override;

	// Services ////////////////////////////////////////////////////////////////////////////

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);

	// Events //////////////////////////////////////////////////////////////////////////////

	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnBuildStatusMenu(WPARAM, LPARAM);

	// Options /////////////////////////////////////////////////////////////////////////////

	CMOption<wchar_t*> m_wszDefaultGroup;  // clist group to store contacts

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
