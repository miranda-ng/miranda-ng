/*
 * $Id: proto.h 14181 2012-03-11 17:51:16Z george.hazan $
 *
 * myYahoo Miranda Plugin
 *
 * Authors: Gennady Feldman (aka Gena01)
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */

#ifndef _YAHOO_PROTO_H_
#define _YAHOO_PROTO_H_

#include <m_protoint.h>

struct CYahooProto : public PROTO<CYahooProto>
{
				CYahooProto(const char*, const TCHAR*);
				virtual ~CYahooProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	MCONTACT  __cdecl AddToList( int flags, PROTOSEARCHRESULT* psr );
	virtual	MCONTACT  __cdecl AddToListByEvent( int flags, int iContact, MEVENT hDbEvent );

	virtual	int       __cdecl Authorize( MEVENT hDbEvent );
	virtual	int       __cdecl AuthDeny( MEVENT hDbEvent, const TCHAR* szReason );
	virtual	int       __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT* );
	virtual	int       __cdecl AuthRequest(MCONTACT hContact, const TCHAR* szMessage );

	virtual	HANDLE    __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath );
	virtual	int       __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer );
	virtual	int       __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason );
	virtual	int       __cdecl FileResume( HANDLE hTransfer, int* action, const PROTOCHAR** szFilename );

	virtual	DWORD_PTR __cdecl GetCaps( int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType );

	virtual	HANDLE    __cdecl SearchBasic( const PROTOCHAR* id );
	virtual	HWND      __cdecl SearchAdvanced( HWND owner );
	virtual	HWND      __cdecl CreateExtendedSearchUI( HWND owner );

	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTORECVFILET*);
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);

	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const PROTOCHAR *szDescription, PROTOCHAR **ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg );

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl SetAwayMsg( int m_iStatus, const PROTOCHAR *msg);
	virtual INT_PTR    __cdecl GetMyAwayMsg(WPARAM wParam, LPARAM lParam);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	//====| Events |======================================================================
	int     __cdecl OnContactDeleted(WPARAM, LPARAM);
	int     __cdecl OnIdleEvent(WPARAM, LPARAM);
	int     __cdecl OnModulesLoadedEx(WPARAM, LPARAM);
	int     __cdecl OnOptionsInit(WPARAM, LPARAM);
	int     __cdecl OnSettingChanged(WPARAM, LPARAM);
	int     __cdecl OnPrebuildContactMenu(WPARAM wParam,LPARAM lParam);

	//====| Services |====================================================================
	INT_PTR __cdecl OnABCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnCalendarCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnEditMyProfile(WPARAM, LPARAM);
	INT_PTR __cdecl OnGotoMailboxCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnRefreshCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnShowMyProfileCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnShowProfileCommand(WPARAM, LPARAM);

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetUnreadEmailCount(WPARAM, LPARAM);
	INT_PTR __cdecl SendNudge(WPARAM, LPARAM);
	INT_PTR __cdecl SetMyAvatar(WPARAM, LPARAM);

	INT_PTR __cdecl CreateConference(WPARAM /*wParam*/, LPARAM /*lParam*/);


	void   BroadcastStatus(int s);
	void   LoadYahooServices( void );
	void   MenuMainInit( void );
	void   MenuContactInit( void );
	void   MenuUninit( void );

	//====| Data |========================================================================
	BOOL   m_bLoggedIn;
	YList *m_connections;
	unsigned int m_connection_tags;

	struct ChatRoom
	{
		char *name;
		YList *members;

		ChatRoom(const char* name, YList *members)
			: name(strdup(name)), members(members) {}

		~ChatRoom()
		{ for (YList *l = members; l; l = l->next) free(l->data);
		  free(name); y_list_free(members); }

		static int compare(const ChatRoom* c1, const ChatRoom* c2)
		{ return strcmp(c1->name, c2->name); }
	};

	OBJLIST <ChatRoom> m_chatrooms;

	char*  m_startMsg;

	// former ylad structure
	char   m_yahoo_id[255]; // user id (login)
	char   m_password[255]; // user password
	char   *m_pw_token; 	// yahoo token (login)
	int    m_id;            // libyahoo id allocated for that proto instance
	int    m_fd;            // socket descriptor
	int    m_status;
	char*  m_msg;
	int    m_rpkts;

	//====| avatar.cpp |==================================================================
	void __cdecl send_avt_thread(void *psf);
	void __cdecl recv_avatarthread(void *pavt);

	INT_PTR __cdecl GetAvatarInfo(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarCaps(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAvatar(WPARAM, LPARAM);

	void   ext_got_picture(const char *me, const char *who, const char *pic_url, int cksum, int type);
	void   ext_got_picture_checksum(const char *me, const char *who, int cksum);
	void   ext_got_picture_update(const char *me, const char *who, int buddy_icon);
	void   ext_got_picture_status(const char *me, const char *who, int buddy_icon);
	void   ext_got_picture_upload(const char *me, const char *url, unsigned int ts);
	void   ext_got_avatar_share(int buddy_icon);

	void   reset_avatar(MCONTACT hContact);
	void   request_avatar(const char* who);

	void   SendAvatar(const TCHAR *szFile);
	void   GetAvatarFileName(MCONTACT hContact, TCHAR* pszDest, int cbLen, int type);

	//====| chat.cpp |====================================================================
	void ChatRegister(void);
	void ChatStart(const char* room);
	void ChatEvent(const char* room, const char* who, int evt, const TCHAR* msg = NULL);
	void ChatLeave(const char* room);
	void ChatLeaveAll(void);

	int __cdecl OnGCEventHook(WPARAM, LPARAM lParam);
	int __cdecl OnGCMenuHook(WPARAM, LPARAM lParam);

	//====| filetransfer.cpp |============================================================
	void __cdecl recv_filethread(void *psf);
	void __cdecl send_filethread(void *psf);

	void   ext_got_file(const char *me, const char *who, const char *url, long expires, const char *msg, const char *fname, unsigned long fesize, const char *ft_token, int y7);
	void   ext_got_files(const char *me, const char *who, const char *ft_token, int y7, YList* files);
	void   ext_got_file7info(const char *me, const char *who, const char *url, const char *fname, const char *ft_token);
	void   ext_ft7_send_file(const char *me, const char *who, const char *filename, const char *token, const char *ft_token);

	//====| icolib.cpp |==================================================================
	void   IconsInit( void );
	HICON  LoadIconEx(const char* name, bool big = false);
	HANDLE GetIconHandle(int iconId);
	void   ReleaseIconEx(const char* name, bool big = false);

	//====| ignore.cpp |==================================================================
	const YList* GetIgnoreList(void);
	void  IgnoreBuddy(const char *buddy, int ignore);
	int   BuddyIgnored(const char *who);

	void 	ext_got_ignore(YList * igns);

	//====| im.cpp |======================================================================
	void   ext_got_im(const char *me, const char *who, int protocol, const char *msg, long tm, int stat, int utf8, int buddy_icon, const char *seqn=NULL, int sendn=0);

	void   send_msg(const char *id, int protocol, const char *msg, int utf8);

	void __cdecl im_sendacksuccess(void *hContact);
	void __cdecl im_sendackfail(void *hContact);
	void __cdecl im_sendackfail_longmsg(void *hContact);

	//====| proto.cpp |===================================================================
	void __cdecl get_status_thread(void *hContact);
	void __cdecl get_info_thread(void *hContact);

	//====| search.cpp |==================================================================
	void __cdecl search_simplethread(void *snsearch);
	void __cdecl searchadv_thread(void *pHWND);

	void   ext_got_search_result(int found, int start, int total, YList *contacts);

	//====| server.cpp |==================================================================
	void __cdecl server_main(void *empty);

	//====| services.cpp |================================================================
	void   logoff_buddies();

	void   OpenURL(const char *url, int autoLogin);

	INT_PTR __cdecl  SetCustomStatCommand(WPARAM, LPARAM);

	//====| user_info.cpp |===============================================================
	int     __cdecl  OnUserInfoInit( WPARAM wParam, LPARAM lParam );

	//====| util.cpp |====================================================================
	int    GetStringUtf(MCONTACT hContact, const char* name, DBVARIANT* );
	DWORD  SetStringUtf(MCONTACT hContact, const char* valueName, const char* parValue );

	DWORD  Set_Protocol(MCONTACT hContact, int protocol );

	int    ShowNotification(const TCHAR *title, const TCHAR *info, DWORD flags);
	void   ShowError(const TCHAR *title, const TCHAR *buff);
	int    ShowPopup( const TCHAR* nickname, const TCHAR* msg, const char *szURL );
	bool   IsMyContact(MCONTACT hContact);

	//====| yahoo.cpp |===================================================================
	MCONTACT add_buddy(const char *yahoo_id, const char *yahoo_name, int protocol, DWORD flags);
	const char *find_buddy( const char *yahoo_id);
	MCONTACT getbuddyH(const char *yahoo_id);
	void   remove_buddy(const char *who, int protocol);

	void   logout();

	void   accept(const char *myid, const char *who, int protocol);
	void   reject(const char *myid, const char *who, int protocol, const char *msg);
	void   sendtyping(const char *who, int protocol, int stat);
	void   set_status(int myyahooStatus, char *msg, int away);
	void   stealth(const char *buddy, int add);

	INT_PTR ext_connect(const char *h, int p, int type);
	int    ext_connect_async(const char *host, int port, int type, yahoo_connect_callback callback, void *data);

	void   ext_send_http_request(enum yahoo_connection_type type, const char *method, const char *url, const char *cookies, long content_length, yahoo_get_fd_callback callback, void *callback_data);
	char * ext_send_https_request(struct yahoo_data *yd, const char *host, const char *path);

	void   ext_status_changed(const char *who, int protocol, int stat, const char *msg, int away, int idle, int mobile, int utf8);
	void   ext_status_logon(const char *who, int protocol, int stat, const char *msg, int away, int idle, int mobile, int cksum, int buddy_icon, long client_version, int utf8);
	void   ext_got_audible(const char *me, const char *who, const char *aud, const char *msg, const char *aud_hash);
	void   ext_got_calendar(const char *url, int type, const char *msg, int svc);
	void   ext_got_stealth(char *stealthlist);
	void   ext_got_buddies(YList * buds);
	void   ext_rejected(const char *who, const char *msg);
	void   ext_buddy_added(char *myid, char *who, char *group, int status, int auth);
	void   ext_contact_added(const char *myid, const char *who, const char *fname, const char *lname, const char *msg, int protocol);
	void   ext_typing_notify(const char *me, const char *who, int protocol, int stat);
	void   ext_game_notify(const char *me, const char *who, int stat, const char *msg);
	void   ext_mail_notify(const char *from, const char *subj, int cnt);
	void   ext_system_message(const char *me, const char *who, const char *msg);
	void   ext_got_identities(const char *nick, const char *fname, const char *lname, YList * ids);
	void   ext_got_ping(const char *errormsg);
	void   ext_error(const char *err, int fatal, int num);
	void   ext_login_response(int succ, const char *url);
	void   ext_login(enum yahoo_status login_mode);

	void   AddBuddy(MCONTACT hContact, const char *group, const TCHAR *msg);

	void   YAHOO_utils_logversion();

	unsigned int 	ext_add_handler(int fd, yahoo_input_condition cond, void *data);
	void 			ext_remove_handler(unsigned int tag);

private:
	int    m_startStatus;
	int    m_unreadMessages;

	int    poll_loop;
	long   lLastSend;

	HANDLE hYahooNudge;

	HGENMENU mainMenuRoot;
	HGENMENU hShowProfileMenuItem;
	HGENMENU menuItemsAll[ 8 ];
};

extern LIST<CYahooProto> g_instances;

#endif
