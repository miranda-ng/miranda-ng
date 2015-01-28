#if !defined(PROTO_H)
#define PROTO_H

class WASocketConnection;

class WhatsAppProto : public PROTO<WhatsAppProto>, public WAListener, public WAGroupListener
{
public:
	WhatsAppProto(const char *proto_name, const TCHAR *username);
	~WhatsAppProto( );

	inline bool isOnline() const
	{	return (m_pConnection != NULL);
	}

	inline bool isOffline() const
	{	return (m_iStatus == ID_STATUS_OFFLINE);
	}

	inline bool isInvisible() const
	{	return (m_iStatus == ID_STATUS_INVISIBLE);
	}

	// PROTO_INTERFACE

	virtual	MCONTACT __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT __cdecl AddToListByEvent(int flags, int iContact, MEVENT hDbEvent) { return NULL; }

	virtual	int      __cdecl Authorize(MEVENT hDbEvent);
	virtual	int      __cdecl AuthDeny(MEVENT hDbEvent, const PROTOCHAR* szReason) { return 1; }
	virtual	int      __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*) { return 1; }
	virtual	int      __cdecl AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage);

	virtual	HANDLE   __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath) { return NULL; }
	virtual	int      __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer) { return 1; }
	virtual	int      __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason) { return 1; }
	virtual	int      __cdecl FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename) { return 1; }

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType) { return 1; }

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const PROTOCHAR* email) { return NULL; }
	virtual	HANDLE    __cdecl SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName) { return NULL; }
	virtual	HWND      __cdecl SearchAdvanced(HWND owner) { return NULL; }
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner) { return NULL; }

	virtual	int       __cdecl RecvContacts(MCONTACT hContact, PROTORECVEVENT*) { return 1; }
	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTOFILEEVENT*) { return 1; }
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvUrl(MCONTACT hContact, PROTORECVEVENT*) { return 1; }

	virtual	int       __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList) { return 1; }
	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles) { return NULL; }
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);
	virtual	int       __cdecl SendUrl(MCONTACT hContact, int flags, const char* url) { return 1; }

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode) { return 1; }
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact) { return NULL; }
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt) { return 1; }
	virtual	int       __cdecl SendAwayMsg(MCONTACT hContact, HANDLE hProcess, const char* msg) { return 1; }
	virtual	int       __cdecl SetAwayMsg(int iStatus, const PROTOCHAR* msg) { return 1; }

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam) { return 1; }

	//////////////////////////////////////////////////////////////////////////////////////
	// Services

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
	INT_PTR __cdecl OnJoinChat(WPARAM, LPARAM);
	INT_PTR __cdecl OnLeaveChat(WPARAM, LPARAM);
	INT_PTR __cdecl OnCreateGroup(WPARAM, LPARAM);

	//////////////////////////////////////////////////////////////////////////////////////
	// Events

	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnBuildStatusMenu(WPARAM, LPARAM);
	int __cdecl OnChatOutgoing(WPARAM, LPARAM);
	int __cdecl OnPrebuildContactMenu(WPARAM, LPARAM);

	INT_PTR __cdecl OnAddContactToGroup(WPARAM, LPARAM, LPARAM);
	INT_PTR __cdecl OnRemoveContactFromGroup(WPARAM, LPARAM, LPARAM);
	INT_PTR __cdecl OnChangeGroupSubject(WPARAM, LPARAM);
	INT_PTR __cdecl OnLeaveGroup(WPARAM, LPARAM);

	// Loops
	bool NegotiateConnection();
	void __cdecl stayConnectedLoop(void*);
	void __cdecl sentinelLoop(void*);

	//////////////////////////////////////////////////////////////////////////////////////
	// Processing Threads

	void __cdecl ProcessBuddyList(void*);
	void __cdecl SearchAckThread(void*);

	//////////////////////////////////////////////////////////////////////////////////////
	// Worker Threads

	void __cdecl SendMsgWorker(void*);
	void __cdecl SendTypingWorker(void*);
	void __cdecl SendGetGroupInfoWorker(void*);
	void __cdecl SendSetGroupNameWorker(void*);
	void __cdecl SendCreateGroupWorker(void*);

	//////////////////////////////////////////////////////////////////////////////////////
	// Contacts handling

	MCONTACT AddToContactList(const std::string &jid, BYTE type = 0, bool dont_check = false,
		const char *new_name = NULL, bool isChatRoom = false, bool isHidden = false);

	bool     IsMyContact(MCONTACT hContact, bool include_chat = false);
	MCONTACT ContactIDToHContact(const std::string&);
	void     SetAllContactStatuses(int status, bool reset_client = false);
	void     UpdateStatusMsg(MCONTACT hContact);
	TCHAR*   GetContactDisplayName(const string &jid);
	void     InitContactMenus();
	void     HandleReceiveGroups(const std::vector<string> &groups, bool isOwned);
	void     RequestFriendship(MCONTACT hContact);

	bool IsGroupChat(MCONTACT hC, bool checkIsAdmin = false)
	{
		return getByte(hC, "SimpleChatRoom", 0) > (checkIsAdmin ? 1 : 0);
	}

	LONG GetSerial();

	//////////////////////////////////////////////////////////////////////////////////////
	// Registration

	bool Register(int state, const string &cc, const string &number, const string &code, string &password);

private:

	//////////////////////////////////////////////////////////////////////////////////////
	// Helpers

	LONG m_iSerial;

	void ToggleStatusMenuItems(BOOL bEnable);

	//////////////////////////////////////////////////////////////////////////////////////
	// Avatars

	std::tstring GetAvatarFileName(MCONTACT);
	std::tstring m_tszAvatarFolder;

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);

	//////////////////////////////////////////////////////////////////////////////////////
	// Handles, Locks

	HGENMENU m_hMenuRoot;
	HANDLE  m_hMenuCreateGroup;

	HANDLE  signon_lock_;
	HANDLE  log_lock_;
	HANDLE  update_loop_lock_;

	WASocketConnection *conn;
	WAConnection *m_pConnection;
	Mutex connMutex;
	int lastPongTime;

	std::vector<unsigned char> m_Challenge;
	string phoneNumber;
	string jid, nick;
	std::map<string, MCONTACT> hContactByJid;
	map<MCONTACT, map<MCONTACT, bool>> isMemberByGroupContact;

	//////////////////////////////////////////////////////////////////////////////////////
	// WhatsApp Events

protected:
	virtual void onMessageForMe(FMessage *paramFMessage, bool paramBoolean);
	virtual void onMessageStatusUpdate(FMessage *paramFMessage);
	virtual void onMessageError(FMessage *message, int paramInt) { ; }
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
	virtual void onPictureChanged(const std::string &from, const std::string &author, bool set);
	virtual void onDeleteAccount(bool result) {  }

	virtual void onGroupAddUser(const std::string &paramString1, const std::string &paramString2);
	virtual void onGroupRemoveUser(const std::string &paramString1, const std::string &paramString2);
	virtual void onGroupNewSubject(const std::string &from, const std::string &author, const std::string &newSubject, int paramInt);
	virtual void onServerProperties(std::map<std::string, std::string>* nameValueMap) {  }
	virtual void onGroupCreated(const std::string &paramString1, const std::string &paramString2);
	virtual void onGroupInfo(const std::string &paramString1, const std::string &paramString2, const std::string &paramString3, const std::string &paramString4, int paramInt1, int paramInt2);
	virtual void onGroupInfoFromList(const std::string &paramString1, const std::string &paramString2, const std::string &paramString3, const std::string &paramString4, int paramInt1, int paramInt2);
	virtual void onOwningGroups(const std::vector<string>& paramVector);
	virtual void onSetSubject(const std::string &paramString) {  }
	virtual void onAddGroupParticipants(const std::string &paramString, const std::vector<string>& paramVector, int paramHashtable) {  }
	virtual void onRemoveGroupParticipants(const std::string &paramString, const std::vector<string>& paramVector, int paramHashtable) {  }
	virtual void onGetParticipants(const std::string &gjid, const std::vector<string>& participants);
	virtual void onParticipatingGroups(const std::vector<string>& paramVector);
	virtual void onLeaveGroup(const std::string &paramString);

	//////////////////////////////////////////////////////////////////////////////////////
	// Information providing

	void NotifyEvent(const TCHAR *title, const TCHAR *info, MCONTACT contact, DWORD flags, TCHAR *url = NULL);
	void NotifyEvent(const std::string &title, const std::string &info, MCONTACT contact, DWORD flags, TCHAR *url = NULL);
};

#endif