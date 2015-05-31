#if !defined(PROTO_H)
#define PROTO_H

class WASocketConnection;

#include "WhatsAPI++/WAConnection.h"

struct WAChatInfo
{
	WAChatInfo(TCHAR *_jid, TCHAR *_nick) :
		tszJid(_jid), tszNick(_nick)
	{
		bActive = false;
	}

	map<std::string, std::tstring> m_unsentMsgs;
	ptrT tszJid, tszNick, tszOwner;
	bool bActive;

	MCONTACT hContact;
};

class WhatsAppProto : public PROTO<WhatsAppProto>, public WAListener, public WAGroupListener
{
public:
	WhatsAppProto(const char *proto_name, const TCHAR *username);
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

	virtual	HANDLE    __cdecl SearchBasic(const TCHAR* id);

	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

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
	TCHAR*   GetContactDisplayName(const string &jid);
	void     RequestFriendship(MCONTACT hContact);

	// Group chats ///////////////////////////////////////////////////////////////////////

	std::vector<string> m_szInviteJids;
	map<std::string, WAChatInfo*> m_chats;
	mir_cs   m_csChats;
	ptrT     m_tszDefaultGroup;

	void     ChatLogMenuHook(WAChatInfo *pInfo, GCHOOK *gch);
	void     NickListMenuHook(WAChatInfo *pInfo, GCHOOK *gch);

	void     AddChatUser(WAChatInfo *pInfo, const TCHAR *ptszJid);
	void     EditChatSubject(WAChatInfo *pInfo);
	void     InviteChatUser(WAChatInfo *pInfo);
	void     KickChatUser(WAChatInfo *pInfo, const TCHAR *ptszJid);
	TCHAR*   GetChatUserNick(const std::string &jid);
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

	void ToggleStatusMenuItems(BOOL bEnable);

	/// Avatars //////////////////////////////////////////////////////////////////////////

	std::tstring GetAvatarFileName(MCONTACT);
	std::tstring m_tszAvatarFolder;

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	int InternalSetAvatar(MCONTACT hContact, const char *szJid, const TCHAR *ptszFileName);

	// Private data //////////////////////////////////////////////////////////////////////
	
	HGENMENU m_hMenuRoot;
	HANDLE  m_hMenuCreateGroup;

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
	virtual void onMessageError(const FMessage &message, int paramInt) { ; }
	virtual void onPing(const std::string &id) throw (WAException);
	virtual void onPingResponseReceived() {  }
	virtual void onAvailable(const std::string &paramString, bool paramBoolean);
	virtual void onClientConfigReceived(const std::string &paramString) {  }
	virtual void onLastSeen(const std::string &paramString1, int paramInt, const std::string &paramString2);
	virtual void onIsTyping(const std::string &paramString, bool paramBoolean);
	virtual void onAccountChange(int paramInt, time_t expire_date) {  }
	virtual void onPrivacyBlockListAdd(const std::string &paramString) {  }
	virtual void onPrivacyBlockListClear() {  }
	virtual void onDirty(const std::map<string, string>& paramHashtable) {  }
	virtual void onDirtyResponse(int paramHashtable) {  }
	virtual void onRelayRequest(const std::string &paramString1, int paramInt, const std::string &paramString2) {  }
	virtual void onSendGetPicture(const std::string &jid, const std::vector<unsigned char>& data, const std::string &id);
	virtual void onPictureChanged(const std::string &jid, const std::string &id, bool set);
	virtual void onContactChanged(const std::string &jid, bool added);
	virtual void onDeleteAccount(bool result) {}

	// WAGroupListener methods ///////////////////////////////////////////////////////////
	virtual void onGroupAddUser(const std::string &gjid, const std::string &ujid, int ts);
	virtual void onGroupRemoveUser(const std::string &gjid, const std::string &ujid, int ts);
	virtual void onGroupNewSubject(const std::string &from, const std::string &author, const std::string &newSubject, int ts);
	virtual void onGroupMessage(const FMessage &paramFMessage);
	virtual void onServerProperties(std::map<std::string, std::string>* nameValueMap) {}
	virtual void onGroupCreated(const std::string &gjid, const std::string &nick);
	virtual void onGroupInfo(const std::string &jid, const std::string &owner, const std::string &subject, const std::string &subject_owner, int time_subject, int time_created);
	virtual void onSetSubject(const std::string &paramString) {  }
	virtual void onAddGroupParticipants(const std::string &paramString, const std::vector<string> &paramVector, int paramHashtable) {  }
	virtual void onRemoveGroupParticipants(const std::string &paramString, const std::vector<string> &paramVector, int paramHashtable) {  }
	virtual void onGetParticipants(const std::string &gjid, const std::vector<string> &participants);
	virtual void onLeaveGroup(const std::string &paramString);

	// Information providing /////////////////////////////////////////////////////////////

	void NotifyEvent(const TCHAR *title, const TCHAR *info, MCONTACT contact, DWORD flags, TCHAR *url = NULL);
	void NotifyEvent(const std::string &title, const std::string &info, MCONTACT contact, DWORD flags, TCHAR *url = NULL);
};

#endif
