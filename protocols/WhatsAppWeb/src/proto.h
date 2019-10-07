/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019 George Hazan

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

struct WAConnection : public MZeroedObject
{
	EVP_PKEY *m_pKeys; // private & public keys
};

class WhatsAppProto : public PROTO<WhatsAppProto>
{
	ptrW m_tszDefaultGroup;

	CMStringA m_szJid, m_szClientId;
	CMStringW m_tszAvatarFolder;

	WAConnection *m_pConn;

	bool ShowQrCode(void);

	/// Avatars //////////////////////////////////////////////////////////////////////////
	CMStringW GetAvatarFileName(MCONTACT hContact);

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

public:
	WhatsAppProto(const char *proto_name, const wchar_t *username);
	~WhatsAppProto();

	inline bool isOnline() const
	{	return false;
	}

	inline bool isOffline() const
	{	return (m_iStatus == ID_STATUS_OFFLINE);
	}

	inline bool isInvisible() const
	{	return (m_iStatus == ID_STATUS_INVISIBLE);
	}

	// PROTO_INTERFACE ///////////////////////////////////////////////////////////////////

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	HANDLE   SearchBasic(const wchar_t* id) override;
	int      SendMsg(MCONTACT hContact, int flags, const char* msg) override;
	int      SetStatus(int iNewStatus) override;
	int      UserIsTyping(MCONTACT hContact, int type) override;

	// Services //////////////////////////////////////////////////////////////////////////

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);

	// Events ////////////////////////////////////////////////////////////////////////////

	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnUserInfo(WPARAM, LPARAM);
	int __cdecl OnBuildStatusMenu(WPARAM, LPARAM);

	// Worker Threads ////////////////////////////////////////////////////////////////////

	void __cdecl stayConnectedLoop(void*);
	void __cdecl sentinelLoop(void*);

	// Processing Threads ////////////////////////////////////////////////////////////////

	void __cdecl ProcessBuddyList(void*);
	void __cdecl SearchAckThread(void*);
	void __cdecl ServerThread(void*);

	// Contacts handling /////////////////////////////////////////////////////////////////

	void     SetAllContactStatuses(int status, bool reset_client = false);
	void     UpdateStatusMsg(MCONTACT hContact);
	void     RequestFriendship(MCONTACT hContact);
};

struct CMPlugin : public ACCPROTOPLUGIN<WhatsAppProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif
