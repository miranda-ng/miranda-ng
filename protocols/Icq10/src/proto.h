// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera, George Hazan
// Copyright © 2012-2019 Miranda NG team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Protocol Interface declarations
// -----------------------------------------------------------------------------

#ifndef _ICQ_PROTO_H_
#define _ICQ_PROTO_H_

#include "m_system.h"
#include "m_protoint.h"

#define ICQ_APP_ID "ic1nmMjqg7Yu-0hL"
#define ICQ_API_SERVER "https://api.icq.net"
#define ICQ_ROBUST_SERVER "https://rapi.icq.net"

struct IcqCacheItem
{
	IcqCacheItem(DWORD _uin, MCONTACT _contact) :
		m_uin(_uin),
		m_hContact(_contact)
	{}

	DWORD m_uin;
	MCONTACT m_hContact;
	bool m_bInList = false;
};

struct IcqOwnMessage
{
	IcqOwnMessage(MCONTACT _hContact, int _msgid, const char *guid)
		: m_hContact(_hContact), m_msgid(_msgid)
	{
		strncpy_s(m_guid, guid, _TRUNCATE);
	}

	MCONTACT m_hContact;
	int m_msgid;
	char m_guid[50];
};

class CIcqProto : public PROTO<CIcqProto>
{
	friend struct CIcqRegistrationDlg;

	bool     m_bOnline = false, m_bTerminated = false;
	void     CheckAvatarChange(MCONTACT hContact, const JSONNode&);
	void     CheckLastId(MCONTACT hContact, const JSONNode&);
	void     CheckNickChange(MCONTACT hContact, const JSONNode&);
	MCONTACT CheckOwnMessage(const CMStringA &reqId, const CMStringA &msgId, bool bRemove);
	void     CheckPassword(void);
	void     ConnectionFailed(int iReason);
	void     OnLoggedIn(void);
	void     OnLoggedOut(void);
	MCONTACT ParseBuddyInfo(const JSONNode &buddy);
	void     ParseMessage(MCONTACT hContact, __int64 &lastMsgId, const JSONNode &msg);
	void     RetrieveUserHistory(MCONTACT, __int64 startMsgId, __int64 endMsgId);
	void     RetrieveUserInfo(MCONTACT);
	void     SetServerStatus(int iNewStatus);
	void     ShutdownSession(void);
	void     StartSession(void);

	mir_cs   csMarkReadQueue;
	LIST<IcqCacheItem> arMarkReadQueue;
	static   void CALLBACK MarkReadTimerProc(HWND hwnd, UINT, UINT_PTR id, DWORD);

	__int64  getId(MCONTACT hContact, const char *szSetting);
	void     setId(MCONTACT hContact, const char *szSetting, __int64 iValue);
	  
	void     OnAddBuddy(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnAddClient(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnCheckPassword(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnCheckPhone(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnFetchEvents(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnGetUserHistory(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnGetUserInfo(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnLoginViaPhone(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnNormalizePhone(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnReceiveAvatar(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnSearchResults(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnSendMessage(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnStartSession(NETLIBHTTPREQUEST*, AsyncHttpRequest*);
	void     OnValidateSms(NETLIBHTTPREQUEST*, AsyncHttpRequest*);

	void     ProcessBuddyList(const JSONNode&);
	void     ProcessEvent(const JSONNode&);
	void     ProcessHistData(const JSONNode&);
	void     ProcessImState(const JSONNode&);
	void     ProcessMyInfo(const JSONNode&);
	void     ProcessPresence(const JSONNode&);
	void     ProcessTyping(const JSONNode&);

	HNETLIBCONN m_ConnPool[CONN_LAST];
	CMStringA m_szSessionKey;
	CMStringA m_szAToken;
	CMStringA m_szRToken;
	CMStringA m_fetchBaseURL;
	CMStringA m_aimsid;
	LONG      m_msgId = 1;
	int       m_iRClientId;

	OBJLIST<IcqOwnMessage> m_arOwnIds;

	////////////////////////////////////////////////////////////////////////////////////////
	// http queue

	mir_cs   m_csHttpQueue;
	HANDLE   m_evRequestsQueue;
	LIST<AsyncHttpRequest> m_arHttpQueue;

	void     CalcHash(AsyncHttpRequest*);
	void     ExecuteRequest(AsyncHttpRequest*);
	void     Push(MHttpRequest*);
	bool     RefreshRobustToken();

	////////////////////////////////////////////////////////////////////////////////////////
	// cache

	mir_cs   m_csCache;
	OBJLIST<IcqCacheItem> m_arCache;

	void     InitContactCache(void);
	IcqCacheItem* FindContactByUIN(DWORD);
	MCONTACT CreateContact(DWORD dwUin, bool bTemporary);

	void     GetAvatarFileName(MCONTACT hContact, wchar_t *pszDest, size_t cbLen);

	////////////////////////////////////////////////////////////////////////////////////////
	// threads

	HANDLE   m_hWorkerThread;
	void     __cdecl ServerThread(void*);

	HANDLE   m_hPollThread;
	void     __cdecl PollThread(void*);

	void     __cdecl SendAckThread(void*);

	////////////////////////////////////////////////////////////////////////////////////////
	// services

	INT_PTR  __cdecl GetAvatar(WPARAM, LPARAM);
	INT_PTR  __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR  __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR  __cdecl SetAvatar(WPARAM, LPARAM);
	
	INT_PTR  __cdecl CreateAccMgrUI(WPARAM, LPARAM);

	////////////////////////////////////////////////////////////////////////////////////////
	// events

	int __cdecl OnDbEventRead(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);

	////////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	MCONTACT AddToList( int flags, PROTOSEARCHRESULT *psr) override;
	MCONTACT AddToListByEvent( int flags, int iContact, MEVENT hDbEvent) override;

	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t *szReason) override;
	int      AuthRecv(MCONTACT hContact, PROTORECVEVENT*) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t *szMessage) override;

	HANDLE   FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szPath) override;
	int      FileCancel(MCONTACT hContact, HANDLE hTransfer) override;
	int      FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szReason) override;
	int      FileResume( HANDLE hTransfer, int *action, const wchar_t **szFilename) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;

	HANDLE   SearchBasic(const wchar_t *id) override;

	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	int      SendMsg(MCONTACT hContact, int flags, const char *msg) override;
	int      SendUrl(MCONTACT hContact, int flags, const char *url) override;

	int      SetApparentMode(MCONTACT hContact, int mode) override;
	int      SetStatus(int iNewStatus) override;

	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT *evt) override;
	int      SetAwayMsg(int m_iStatus, const wchar_t *msg) override;

	int      UserIsTyping(MCONTACT hContact, int type) override;

	void     OnContactDeleted(MCONTACT) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

public:
	CIcqProto(const char*, const wchar_t*);
	~CIcqProto();

	CMOption<DWORD> m_dwUin;
	CMOption<wchar_t*> m_szPassword;
};

struct CMPlugin : public ACCPROTOPLUGIN<CIcqProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif
