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

class WhatsAppProto : public PROTO<WhatsAppProto>
{
	ptrW m_tszDefaultGroup;

	CMStringA m_szJid;
	CMStringW m_tszAvatarFolder;

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

	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	INT_PTR  __cdecl GetCaps(int type, MCONTACT hContact = NULL) override;
	HANDLE   __cdecl SearchBasic(const wchar_t* id) override;
	int      __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg) override;
	int      __cdecl SetStatus(int iNewStatus) override;
	int      __cdecl UserIsTyping(MCONTACT hContact, int type) override;

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
