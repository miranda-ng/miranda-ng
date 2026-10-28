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

#define DB_KEY_ID        "UID"          // OK user id of a private contact
#define DB_KEY_CONV      "ConvId"       // conversation id (base64) of a group chat
#define DB_KEY_LASTTIME  "LastMsgTime"  // date_ms of the newest message already in the database
#define DB_KEY_TOKEN     "AuthToken"    // auth_token of api/auth/login, used for loginByToken

// the reply of one signed API call
struct OkReply
{
	int code = 0;            // 0 on success, an OK error_code, or -1 for a transport failure
	CMStringA error;         // error_msg
	JSONNode root;           // the parsed body (a top-level array is wrapped as "items")

	__forceinline operator bool() const { return code == 0; }
};

// what a conversation from messagesV2/getList tells us
struct OkConversation
{
	CMStringA id;            // base64 conversation id
	bool bChat = false;
	CMStringA peerUid;       // the other participant of a one-to-one conversation
	CMStringW topic;         // the title of a group chat
	int64_t lastTime = 0;    // last_msg_time_ms
	int newCount = 0;        // new_msgs_count
	std::vector<CMStringA> participants;
};

// a received photo until the core turns it into an offline file
struct OkPhoto
{
	CMStringA szUrl;
};

class COdnoklassnikiProto : public PROTO<COdnoklassnikiProto>
{
	friend class COkAccountDlg;

	// session
	mir_cs m_csSession;
	CMStringA m_szSessionKey, m_szSessionSecret, m_szApiServer, m_szMyUid;
	bool m_bOnline = false, m_bTerminated = false;
	HANDLE m_hPollEvent;

	// api.cpp
	OkReply Call(const char *szMethod, const std::map<CMStringA, CMStringA> &params, bool bLogin = false);
	OkReply Batch(const CMStringA &szMethodsJson);
	bool UploadPhoto(const wchar_t *pwszFile, CMStringA &szToken);

	// poll.cpp
	void __cdecl ServerThread(void *);
	bool Login();
	void LoadFriends();
	void PollConversations();
	void ProcessConversation(const OkConversation &cnv);
	void FetchMessages(MCONTACT hContact, const OkConversation &cnv, int64_t since, int iMaxUnread);
	void ResolveUsers(const std::vector<CMStringA> &uids);
	void ShowVerification(const CMStringA &szUrl);

	// contacts
	MCONTACT FindUser(const char *uid);
	MCONTACT AddUser(const char *uid, bool bTemporary);
	MCONTACT FindChat(const char *cnvId);
	CMStringA GetConversationId(MCONTACT hContact);
	CMStringW GetUserName(const char *uid);
	std::map<CMStringA, CMStringW> m_names;   // user names already resolved, by uid
	mir_cs m_csNames;

	// chats.cpp
	SESSION_INFO* StartChat(const OkConversation &cnv);
	std::set<CMStringA> m_onlineChats;   // chats already set online in this session (poll thread only)
	int __cdecl GcEventHook(WPARAM, LPARAM);

	// messages
	struct SendParam
	{
		MCONTACT hContact;
		int seq;
		CMStringA text;
	};
	void __cdecl SendMsgThread(void *);
	CMStringA SendMessage(const CMStringA &cnvId, const char *pszText, const CMStringA &szPhotoToken, CMStringA &szError);

	// files.cpp
	void __cdecl SendFileThread(void *);
	void __cdecl OfflineFileThread(void *);
	INT_PTR __cdecl SvcOfflineFile(WPARAM, LPARAM);
	volatile long m_msgSeq = 0;

	// photos sent from here: their echo from the server is already in the database as a file transfer
	std::set<CMStringA> m_sentPhotos;
	mir_cs m_csSentPhotos;

public:
	COdnoklassnikiProto(const char *, const wchar_t *);
	~COdnoklassnikiProto();

	CMOption<wchar_t*> m_wszLogin;
	CMOption<int> m_iPollInterval;

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	INT_PTR  GetCaps(int type, MCONTACT hContact = 0) override;
	int      SendMsg(MCONTACT hContact, MEVENT hReplyEvent, const char *msg) override;
	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	int      SetStatus(int iNewStatus) override;

	MWindow  OnCreateAccMgrUI(MWindow) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;
	void     OnReceiveOfflineFile(DB::EventInfo &dbei, DB::FILE_BLOB &blob) override;

	int __cdecl OnOptionsInit(WPARAM, LPARAM);
};

struct CMPlugin : public ACCPROTOPLUGIN<COdnoklassnikiProto>
{
	CMPlugin();

	int Load() override;
};
