/*
Copyright (C) 2013 Miranda NG Project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

struct CVkProto;
typedef void (CVkProto::*VK_REQUEST_HANDLER)(NETLIBHTTPREQUEST*, void*);

struct CVkProto : public PROTO<CVkProto>
{
				CVkProto(const char*, const TCHAR*);
				~CVkProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	HANDLE __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	HANDLE __cdecl AddToListByEvent(int flags, int iContact, HANDLE hDbEvent);

	virtual	int    __cdecl Authorize(HANDLE hDbEvent);
	virtual	int    __cdecl AuthDeny(HANDLE hDbEvent, const TCHAR *szReason);
	virtual	int    __cdecl AuthRecv(HANDLE hContact, PROTORECVEVENT*);
	virtual	int    __cdecl AuthRequest(HANDLE hContact, const TCHAR *szMessage);

	virtual	HANDLE __cdecl ChangeInfo(int iInfoType, void* pInfoData);

	virtual	HANDLE __cdecl FileAllow(HANDLE hContact, HANDLE hTransfer, const TCHAR *szPath);
	virtual	int    __cdecl FileCancel(HANDLE hContact, HANDLE hTransfer);
	virtual	int    __cdecl FileDeny(HANDLE hContact, HANDLE hTransfer, const TCHAR *szReason);
	virtual	int    __cdecl FileResume(HANDLE hTransfer, int* action, const TCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, HANDLE hContact = NULL);
	virtual	int    __cdecl GetInfo(HANDLE hContact, int infoType);

	virtual	HANDLE __cdecl SearchBasic(const TCHAR *id);
	virtual	HANDLE __cdecl SearchByEmail(const TCHAR *email);
	virtual	HANDLE __cdecl SearchByName(const TCHAR *nick, const TCHAR *firstName, const TCHAR *lastName);
	virtual	HWND   __cdecl SearchAdvanced(HWND owner);
	virtual	HWND   __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int    __cdecl RecvContacts(HANDLE hContact, PROTORECVEVENT*);
	virtual	int    __cdecl RecvFile(HANDLE hContact, PROTORECVFILET*);
	virtual	int    __cdecl RecvMsg(HANDLE hContact, PROTORECVEVENT*);
	virtual	int    __cdecl RecvUrl(HANDLE hContact, PROTORECVEVENT*);

	virtual	int    __cdecl SendContacts(HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList);
	virtual	HANDLE __cdecl SendFile(HANDLE hContact, const TCHAR *szDescription, TCHAR** ppszFiles);
	virtual	int    __cdecl SendMsg(HANDLE hContact, int flags, const char* msg);
	virtual	int    __cdecl SendUrl(HANDLE hContact, int flags, const char* url);

	virtual	int    __cdecl SetApparentMode(HANDLE hContact, int mode);
	virtual	int    __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE __cdecl GetAwayMsg(HANDLE hContact);
	virtual	int    __cdecl RecvAwayMsg(HANDLE hContact, int mode, PROTORECVEVENT* evt);
	virtual	int    __cdecl SetAwayMsg(int m_iStatus, const TCHAR *msg);

	virtual	int    __cdecl UserIsTyping(HANDLE hContact, int type);

	virtual	int    __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	//==== Events ========================================================================

	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);

	void OnOAuthAuthorize(NETLIBHTTPREQUEST*, void*);
	void OnReceiveMyInfo(NETLIBHTTPREQUEST*, void*);
	void OnReceiveAvatar(NETLIBHTTPREQUEST*, void*);

	//==== Services ======================================================================

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl SvcGetAvatarCaps(WPARAM, LPARAM);

	//==== Misc ==========================================================================

	TCHAR* GetUserStoredPassword(void);

	void RetrieveUserInfo(LPCSTR szUserId);
	void OnReceiveUserInfo(NETLIBHTTPREQUEST*, void*);

	void RetrieveFriends();
	void OnReceiveFriends(NETLIBHTTPREQUEST*, void*);

	void RetrieveUnreadMessages();
	void OnReceiveMessages(NETLIBHTTPREQUEST*, void*);

	void RetrievePollingInfo();
	void OnReceivePollingInfo(NETLIBHTTPREQUEST*, void*);

	void __cdecl PollingThread(void*);
	int  PollServer();
	void OnReceivePolling(NETLIBHTTPREQUEST*, void*);

	int  SetServerStatus(int);

	__forceinline bool IsOnline() const { return m_bOnline; }

private:
	struct AsyncHttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
	{
		AsyncHttpRequest();
		~AsyncHttpRequest();

		void AddHeader(LPCSTR, LPCSTR);
		void Redirect(NETLIBHTTPREQUEST*);

		VK_REQUEST_HANDLER m_pFunc;
		void *pUserInfo;
	};
	LIST<AsyncHttpRequest> m_arRequestsQueue;
	CRITICAL_SECTION m_csRequestsQueue;
	CMStringA m_prevUrl, m_savedCookie;
	HANDLE m_evRequestsQueue;
	HANDLE m_hWorkerThread;
	bool   m_bTerminated;

	void   InitQueue();
	void   UninitQueue();
	void   ExecuteRequest(AsyncHttpRequest*);
	bool   PushAsyncHttpRequest(int iRequestType, LPCSTR szUrl, bool bSecure, VK_REQUEST_HANDLER pFunc, int nParams = 0, HttpParam *pParams = 0, int iTimeout = 10000);
	bool   PushAsyncHttpRequest(AsyncHttpRequest*, int iTimeout = 10000);
	int    SetupConnection(void);
	void   __cdecl WorkerThread(void*);

	CMStringA RunCaptchaForm(LPCSTR szUrl);
	CMStringA AutoFillForm(char*, CMStringA&);

	void   ConnectionFailed(int iReason);
	bool   CheckJsonResult(JSONNODE*);
	void   OnLoggedIn();
	void   OnLoggedOut();
	void   ShutdownSession();

	void   GetAvatarFileName(HANDLE hContact, TCHAR* pszDest, size_t cbLen);

	HANDLE FindUser(LPCSTR userid, bool bCreate = false);

	void   SetAllContactStatuses(int status);

	void   __cdecl SendMsgAck(void *param);

	bool   m_bOnline;

	HANDLE m_hNetlibConn;
	ptrA   m_szAccessToken, m_myUserId;
	ptrT   m_defaultGroup;
	UINT_PTR m_timer;

	ptrA   m_pollingServer, m_pollingKey, m_pollingTs;
	HANDLE m_hPollingThread;
	ULONG  m_msgId;
};
