/*
Copyright (C) 2026 Miranda NG team

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

#pragma once

#define KICQ_DEFAULT_SERVER "195.66.114.37"
#define KICQ_DEFAULT_PORT 5190

// what the session knows about a server-side group
struct KicqGroup
{
	int id;
	CMStringW name;
	bool bNotInList; // the "not in list" pseudo-group (TLV 0x6A): temporary contacts
};

// per-contact state that lives only as long as the connection
struct KicqUser
{
	KicqUser(uint32_t _uin, MCONTACT _hContact) :
		uin(_uin), hContact(_hContact)
	{}

	uint32_t uin;
	MCONTACT hContact;
	int groupId = 0, ssiId = 0; // 0: not on the server-side list
	bool bAuthorized = true;
	bool bUtf8 = false, bServerRelay = false;
	bool bOnline = false;
};

struct KicqPendingMsg
{
	uint32_t uin;
	int seq;
};

struct KicqInfoRequest
{
	MCONTACT hContact;  // 0 for a search
	HANDLE hSearch;
	uint32_t uin;
	time_t started;
	CMStringW nick, firstName, lastName, email;
};

class CKicqProto : public PROTO<CKicqProto>
{
	friend class CKicqAccountDlg;

	// connection
	HNETLIBCONN m_hConn = nullptr;
	mir_cs m_csSend;
	int m_seq = 1;
	bool m_bOnline = false, m_bTerminated = false;
	bool m_bProbeAnswered = true;
	time_t m_lastProbe = 0;
	bytes_t m_cookie;
	CMStringA m_szUin, m_szBosHost;
	int m_iBosPort = 0;
	int m_iLoginError = 0;

	void __cdecl ServerThread(void *);
	bool RunConnection(const char *host, int port, bool bLogin);
	void Send(int channel, const bytes_t &payload);
	void Send(const OscarSnac &snac);
	void ShutdownSession();

	// inbound
	enum ConnResult { CONN_CONTINUE, CONN_REDIRECT, CONN_CLOSE };
	ConnResult HandleFlap(int channel, const bytes_t &data, bool bLogin);
	ConnResult HandleChannel4(const bytes_t &data);
	void HandleSnac(const bytes_t &data);
	void HandleRates(const bytes_t &data);
	void HandleRoster(const bytes_t &data, int flags);
	void FinishLogin();
	void HandleUserOnline(const bytes_t &data);
	void HandleUserOffline(const bytes_t &data);
	void HandleIncomingMessage(const bytes_t &data);
	void HandleOfflineMessage(const bytes_t &data);
	void HandleUserInfo(const bytes_t &data, uint32_t reqId);
	void HandleMessageAck(const bytes_t &data);
	void HandleMessageError(const bytes_t &data, uint32_t reqId);
	void HandleTyping(const bytes_t &data);
	void HandleAuthRequest(const bytes_t &data);
	void HandleAuthReply(const bytes_t &data);
	void HandleYouWereAdded(const bytes_t &data);
	void HandleSsiItem(const bytes_t &data, bool bDeleted);
	void HandleSsiResult(const bytes_t &data, uint32_t reqId);
	void DeliverMessage(uint32_t uin, const CMStringW &text, time_t when);
	void CheckTimeouts();

	// roster
	mir_cs m_csRoster;
	OBJLIST<KicqUser> m_arUsers;
	OBJLIST<KicqGroup> m_arGroups;
	std::set<CMStringW> m_offlineSeen;

	KicqUser* FindUser(uint32_t uin);
	KicqUser* AddUser(uint32_t uin, MCONTACT hContact);
	KicqGroup* FindGroup(int id);
	KicqGroup* FindGroup(const wchar_t *name);
	int NewSsiId();
	MCONTACT FindContact(uint32_t uin);
	MCONTACT CreateContact(uint32_t uin, bool bTemporary);
	void ApplyRosterItem(uint32_t uin, int groupId, int ssiId, const OscarTlvList &tlvs, bool bFromLogin);
	int ResolveGroupFor(MCONTACT hContact);

	// one SSI edit at a time, like Jasmine: the result names only the request type
	struct PendingSsi
	{
		uint32_t uin = 0;
		CMStringW name;
		int groupId = 0, ssiId = 0;
		MCONTACT hContact = 0;
	};
	PendingSsi m_pendAdd, m_pendDel, m_pendRename, m_pendGroup, m_pendGroupUpdate;
	CMStringW m_wszAddAfterGroup;   // a contact add queued until its group exists
	MCONTACT m_hAddAfterGroup = 0;

	void SsiAddContact(MCONTACT hContact, int groupId);

	// messages, info
	mir_cs m_csPending;
	std::map<uint32_t, KicqPendingMsg> m_pendingMsgs;
	std::map<uint32_t, KicqInfoRequest> m_infoRequests;
	uint32_t m_nextMsgReq = 0x1000, m_nextInfoReq = 0x2000;
	int m_msgSeq = 0;

	uint32_t RequestUserInfo(uint32_t uin, MCONTACT hContact, HANDLE hSearch);

	// status
	int MirandaToIcqStatus(int iStatus);
	int IcqToMirandaStatus(int iIcqStatus);
	void SendStatus();
	CMStringW GetOwnAwayText();
	void __cdecl GetAwayMsgThread(void *param);

	// events
	int __cdecl OnSettingChanged(WPARAM, LPARAM);
	int __cdecl OnGroupChange(WPARAM, LPARAM);

public:
	CKicqProto(const char *, const wchar_t *);
	~CKicqProto();

	CMOption<wchar_t*> m_wszServer;
	CMOption<int> m_iPort;
	CMOption<wchar_t*> m_wszDefaultGroup;

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	MCONTACT AddToListByEvent(int flags, int iContact, MEVENT hDbEvent) override;

	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t *szReason) override;
	int      AuthRecv(MCONTACT, DB::EventInfo &dbei) override;
	int      AuthRequest(MCONTACT hContact, const wchar_t *szMessage) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact = 0) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;

	HANDLE   SearchBasic(const wchar_t *id) override;

	int      SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char *msg) override;

	int      SetStatus(int iNewStatus) override;

	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      SetAwayMsg(int iStatus, const wchar_t *msg) override;

	int      UserIsTyping(MCONTACT hContact, int type) override;

	MWindow  OnCreateAccMgrUI(MWindow) override;
	bool     OnContactDeleted(MCONTACT, uint32_t flags) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

	int __cdecl OnOptionsInit(WPARAM, LPARAM);
};

struct CMPlugin : public ACCPROTOPLUGIN<CKicqProto>
{
	CMPlugin();

	int Load() override;
};
