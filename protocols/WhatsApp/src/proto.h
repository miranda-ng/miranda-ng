#if !defined(PROTO_H)
#define PROTO_H

class WASocketConnection;

#include "WhatsAPI++/WAConnection.h"

struct WAChatInfo
{
	WAChatInfo(wchar_t *_jid, wchar_t *_nick) :
		tszJid(_jid), tszNick(_nick)
	{
		bActive = false;
	}

	map<std::string, std::wstring> m_unsentMsgs;
	ptrW tszJid, tszNick, tszOwner;
	bool bActive;

	MCONTACT hContact;
};

class WhatsAppProto : public PROTO<WhatsAppProto>, public WAListener, public WAGroupListener
{
public:
	WhatsAppProto(const char *proto_name, const wchar_t *username);
	~WhatsAppProto();

	inline bool isOnline() const
	{	return (m_pConnection != NULL);
	}

	inline bool isOffline() const
	{	return (m_iStatus == ID_STATUS_OFFLINE);
	}

	inline bool isInvisible() const
	{	return (m_iStatus == ID_STATUS_INVISIBLE);
	}

	// PROTO_INTERFACE ///////////////////////////////////////////////////////////////////

	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);

	virtual	HANDLE    __cdecl SearchBasic(const wchar_t* id);

	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const wchar_t*, wchar_t **ppszFiles);

	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);

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

	MCONTACT AddToContactList(const std::string &jid, const char *new_name = NULL);

	MCONTACT ContactIDToHContact(const std::string&);
	void     SetAllContactStatuses(int status, bool reset_client = false);
	void     UpdateStatusMsg(MCONTACT hContact);
	wchar_t*   GetContactDisplayName(const string &jid);
	void     RequestFriendship(MCONTACT hContact);

	// Group chats ///////////////////////////////////////////////////////////////////////

	std::vector<string> m_szInviteJids;
	map<std::string, WAChatInfo*> m_chats;
	mir_cs   m_csChats;
	ptrW     m_tszDefaultGroup;

	void     ChatLogMenuHook(WAChatInfo *pInfo, GCHOOK *gch);
	void     NickListMenuHook(WAChatInfo *pInfo, GCHOOK *gch);

	void     AddChatUser(WAChatInfo *pInfo, const wchar_t *ptszJid);
	void     EditChatSubject(WAChatInfo *pInfo);
	void     InviteChatUser(WAChatInfo *pInfo);
	void     KickChatUser(WAChatInfo *pInfo, const wchar_t *ptszJid);
	wchar_t*   GetChatUserNick(const std::string &jid);
	void     SetChatAvatar(WAChatInfo *pInfo);

	void     onGroupMessageReceived(const FMessage &fmsg);

	WAChatInfo* InitChat(const std::string &jidjid, const std::string &nick);
	WAChatInfo* SafeGetChat(const std::string &jid);

	int      __cdecl onGroupChatEvent(WPARAM, LPARAM);
	int      __cdecl OnDeleteChat(WPARAM, LPARAM);
	int      __cdecl OnChatMenu(WPARAM, LPARAM);
	INT_PTR  __cdecl OnCreateGroup(WPARAM, LPARAM);

	// Registration //////////////////////////////////////////////////////////////////////

	bool Register(int state, const string &cc, const string &number, const string &code, string &password);

private:
	// Helpers 	//////////////////////////////////////////////////////////////////////////

	LONG m_iSerial;
	__forceinline LONG GetSerial()
	{	return ::_InterlockedIncrement(&m_iSerial);
	}

	void ToggleStatusMenuItems(bool bEnable);

	/// Avatars //////////////////////////////////////////////////////////////////////////

	std::wstring GetAvatarFileName(MCONTACT);
	std::wstring m_tszAvatarFolder;

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	int InternalSetAvatar(MCONTACT hContact, const char *szJid, const wchar_t *ptszFileName);

	// Private data //////////////////////////////////////////////////////////////////////
	
	void InitMenu();
	HGENMENU m_hMenuCreateGroup;

	HANDLE  update_loop_lock_;

	WASocketConnection *m_pSocket;
	WAConnection *m_pConnection;
	Mutex connMutex;
	time_t m_tLastWriteTime;

	std::vector<unsigned char> m_Challenge;
	std::string m_szPhoneNumber;
	std::string m_szJid, m_szNick;
	std::map<string, MCONTACT> m_hContactByJid;
	map<MCONTACT, map<MCONTACT, bool>> isMemberByGroupContact;

protected:
	// WAListener methods ////////////////////////////////////////////////////////////////
	virtual void onMessageForMe(const FMessage &paramFMessage);
	virtual void onMessageStatusUpdate(const FMessage &paramFMessage);
	virtual void onMessageError(const FMessage&, int) { }
	virtual void onPing(const std::string &id) throw (WAException);
	virtual void onPingResponseReceived() { }
	virtual void onAvailable(const std::string &paramString, bool paramBoolean, DWORD lastSeenTime);
	virtual void onClientConfigReceived(const std::string&) { }
	virtual void onIsTyping(const std::string &paramString, bool paramBoolean);
	virtual void onAccountChange(int, time_t) { }
	virtual void onPrivacyBlockListAdd(const std::string&) { }
	virtual void onPrivacyBlockListClear() { }
	virtual void onDirty(const std::map<string, string>&) { }
	virtual void onDirtyResponse(int) { }
	virtual void onRelayRequest(const std::string&, int, const std::string&) { }
	virtual void onSendGetPicture(const std::string &jid, const std::vector<unsigned char>& data, const std::string &id);
	virtual void onPictureChanged(const std::string &jid, const std::string &id, bool set);
	virtual void onContactChanged(const std::string &jid, bool added);
	virtual void onDeleteAccount(bool) {}

	// WAGroupListener methods ///////////////////////////////////////////////////////////
	virtual void onGroupAddUser(const std::string &gjid, const std::string &ujid, int ts);
	virtual void onGroupRemoveUser(const std::string &gjid, const std::string &ujid, int ts);
	virtual void onGroupNewSubject(const std::string &from, const std::string &author, const std::string &newSubject, int ts);
	virtual void onGroupMessage(const FMessage &paramFMessage);
	virtual void onServerProperties(std::map<std::string, std::string>*) { }
	virtual void onGroupCreated(const std::string &gjid, const std::string &nick);
	virtual void onGroupInfo(const std::string &jid, const std::string &owner, const std::string &subject, const std::string &subject_owner, int time_subject, int time_created);
	virtual void onSetSubject(const std::string&) { }
	virtual void onAddGroupParticipants(const std::string&, const std::vector<string>&, int) { }
	virtual void onRemoveGroupParticipants(const std::string&, const std::vector<string>&, int) { }
	virtual void onGetParticipants(const std::string &gjid, const std::vector<string> &participants);
	virtual void onLeaveGroup(const std::string &paramString);

	// Information providing /////////////////////////////////////////////////////////////

	void NotifyEvent(const wchar_t *title, const wchar_t *info, MCONTACT contact, DWORD flags, wchar_t *url = NULL);
	void NotifyEvent(const std::string &title, const std::string &info, MCONTACT contact, DWORD flags, wchar_t *url = NULL);
};

#endif
