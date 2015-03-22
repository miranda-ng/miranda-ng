////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2009 Adam Strzelecki <ono+miranda@java.pl>
// Copyright (c) 2009-2012 Bartosz Bia³ek
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#ifndef GGPROTO_H
#define GGPROTO_H

struct GGPROTO : public PROTO<GGPROTO>
{
				GGPROTO( const char*, const TCHAR* );
				~GGPROTO();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	MCONTACT __cdecl AddToList( int flags, PROTOSEARCHRESULT* psr );
							 
	virtual	HANDLE   __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath );
	virtual	int      __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer );
	virtual	int      __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason );

	virtual	DWORD_PTR __cdecl GetCaps( int type, MCONTACT hContact = NULL );
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType );

	virtual	HANDLE    __cdecl SearchBasic( const TCHAR* id );
	virtual	HANDLE    __cdecl SearchByName( const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName );
	virtual	HWND      __cdecl SearchAdvanced( HWND owner );
	virtual	HWND      __cdecl CreateExtendedSearchUI( HWND owner );

	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTORECVFILET* );

	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const TCHAR* szDescription, TCHAR** ppszFiles );
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg );

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode );
	virtual	int       __cdecl SetStatus( int iNewStatus );

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact );
	virtual	int       __cdecl SetAwayMsg( int m_iStatus, const TCHAR* msg );

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type );

	virtual	int       __cdecl OnEvent( PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam );

	//////////////////////////////////////////////////////////////////////////////////////
	//  Services

	INT_PTR  __cdecl blockuser(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl getmyawaymsg(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl get_acc_mgr_gui(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl leavechat(WPARAM wParam, LPARAM lParam);

	void     __cdecl sendackthread(void *);
	void     __cdecl searchthread(void *);
	void     __cdecl cmdgetinfothread(void *hContact);
	void     __cdecl getawaymsgthread(void *hContact);
	void     __cdecl dccmainthread(void *);
	void     __cdecl ftfailthread(void *param);
	void     __cdecl remindpasswordthread(void *param);

	//////////////////////////////////////////////////////////////////////////////////////

	/* Helper functions */
	int status_m2gg(int status, int descr);
	int status_gg2m(int status);
	void checknewuser(uin_t uin, const char* passwd);

	/* Thread functions */
	void threadwait(GGTHREAD *thread);

#ifdef DEBUGMODE
	volatile int extendedLogging;
#endif
	void gg_EnterCriticalSection(CRITICAL_SECTION* mutex, char* callingFunction, int sectionNumber, char* mutexName, int logging);
	void gg_LeaveCriticalSection(CRITICAL_SECTION* mutex, char* callingFunction, int sectionNumber, int returnNumber, char* mutexName, int logging);
	void gg_sleep(DWORD miliseconds, BOOL alterable, char* callingFunction, int sleepNumber, int logging);

	/* Global GG functions */
	void notifyuser(MCONTACT hContact, int refresh);
	void setalloffline();
	void disconnect();
	MCONTACT getcontact(uin_t uin, int create, int inlist, TCHAR *nick);
	void __cdecl mainthread(void *empty);
	int isonline();
	int refreshstatus(int status);

	void broadcastnewstatus(int newStatus);
	void cleanuplastplugin(DWORD version);
	int contactdeleted(WPARAM wParam, LPARAM lParam);
	int dbsettingchanged(WPARAM wParam, LPARAM lParam);
	void notifyall();
	void changecontactstatus(uin_t uin, int status, const TCHAR *idescr, int time, uint32_t remote_ip, uint16_t remote_port, uint32_t version);
	TCHAR *getstatusmsg(int status);
	void dccstart();
	void dccconnect(uin_t uin);
	int gettoken(GGTOKEN *token);
	void parsecontacts(char *contacts);
	void remindpassword(uin_t uin, const char *email);
	void menus_init();

	/* Avatar functions */
	void getAvatarFilename(MCONTACT hContact, TCHAR *pszDest, int cbLen);
	void requestAvatarTransfer(MCONTACT hContact, char *szAvatarURL);
	void requestAvatarInfo(MCONTACT hContact, int iWaitFor);
	void getOwnAvatar();
	void setAvatar(const TCHAR *szFilename);
	bool getAvatarFileInfo(uin_t uin, char **avatarurl, char **avatarts);

	INT_PTR  __cdecl getavatarcaps(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl getavatarinfo(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl getmyavatar(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl setmyavatar(WPARAM wParam, LPARAM lParam);

	void initavatarrequestthread();

	void     __cdecl avatarrequestthread(void*);
	void     __cdecl getOwnAvatarThread(void*);
	void     __cdecl setavatarthread(void*);

	/* File transfer functions */
	HANDLE fileallow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath);
	int filecancel(MCONTACT hContact, HANDLE hTransfer);
	int filedeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason);
	int recvfile(MCONTACT hContact, PROTOFILEEVENT* pre);
	HANDLE sendfile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles);

	HANDLE dccfileallow(HANDLE hTransfer, const PROTOCHAR* szPath);
	HANDLE dcc7fileallow(HANDLE hTransfer, const PROTOCHAR* szPath);

	int dccfiledeny(HANDLE hTransfer);
	int dcc7filedeny(HANDLE hTransfer);

	int dccfilecancel(HANDLE hTransfer);
	int dcc7filecancel(HANDLE hTransfer);

	/* Import module */
	void import_init(HGENMENU hRoot);

	INT_PTR  __cdecl import_server(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl import_text(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl remove_server(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl export_server(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl export_text(WPARAM wParam, LPARAM lParam);

	/* Keep-alive module */
	void keepalive_init();
	void keepalive_destroy();

	/* Image reception functions */
	int img_init();
	int img_destroy();
	int img_shutdown();
	int img_sendonrequest(gg_event* e);
	BOOL img_opened(uin_t uin);
	void *img_loadpicture(gg_event* e, TCHAR *szFileName);
	int img_display(MCONTACT hContact, void *img);
	int img_displayasmsg(MCONTACT hContact, void *img);

	void __cdecl img_dlgcallthread(void *param);

	INT_PTR __cdecl img_recvimage(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl img_sendimg(WPARAM wParam, LPARAM lParam);

	void links_instance_init();

	/* OAuth functions */
	int oauth_checktoken(int force);
	int oauth_receivetoken();

	/* UI page initializers */
	int __cdecl options_init(WPARAM wParam, LPARAM lParam);
	int __cdecl details_init(WPARAM wParam, LPARAM lParam);

	/* Groupchat functions */
	int gc_init();
	void gc_menus_init(HGENMENU hRoot);
	int gc_destroy();
	TCHAR * gc_getchat(uin_t sender, uin_t *recipients, int recipients_count);
	GGGC *gc_lookup(const TCHAR *id);
	int gc_changenick(MCONTACT hContact, TCHAR *ptszNick);

	int __cdecl gc_event(WPARAM wParam, LPARAM lParam);

	INT_PTR __cdecl gc_openconf(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl gc_clearignored(WPARAM wParam, LPARAM lParam);

	/* Popups functions */
	void initpopups();
	void showpopup(const TCHAR* nickname, const TCHAR* msg, int flags);

	/* Sessions functions */
	INT_PTR __cdecl sessions_view(WPARAM wParam, LPARAM lParam);
	void sessions_updatedlg();
	BOOL sessions_closedlg();
	void sessions_menus_init(HGENMENU hRoot);

	void block_init();
	void block_uninit();

	//////////////////////////////////////////////////////////////////////////////////////

	CRITICAL_SECTION ft_mutex, sess_mutex, img_mutex, modemsg_mutex, avatar_mutex, sessions_mutex;
	list_t watches, transfers, requests, chats, imagedlgs, sessions;
	LIST<GGREQUESTAVATARDATA> avatar_requests;
	LIST<GGGETAVATARDATA> avatar_transfers;
	int gc_enabled, gc_id, is_list_remove, check_first_conn;
	uin_t next_uin;
	unsigned long last_crc;
	GGTHREAD pth_dcc;
	GGTHREAD pth_sess;
	GGTHREAD pth_avatar;
	struct gg_session *sess;
	struct gg_dcc *dcc;
	HANDLE hEvent;
	HANDLE hConnStopEvent;
	SOCKET sock;
	UINT_PTR timer;
	struct
	{
		TCHAR *online;
		TCHAR *away;
		TCHAR *dnd;
		TCHAR *freechat;
		TCHAR *invisible;
		TCHAR *offline;
	} modemsg;
	
	HGENMENU hMenuRoot;
	HGENMENU hMainMenu[7];
	HGENMENU hBlockMenuItem, hImageMenuItem, hInstanceMenuItem;
	HANDLE   hPrebuildMenuHook;
	HANDLE   hImagesFolder;
	HWND     hwndSessionsDlg;
	HANDLE   hPopupNotify, hPopupError;
};

typedef struct
{
	int mode;
	uin_t uin;
	char *pass;
	char *email;
	GGPROTO *gg;
} GGUSERUTILDLGDATA;


inline void GGPROTO::gg_EnterCriticalSection(CRITICAL_SECTION* mutex, char* callingFunction, int sectionNumber, char* mutexName, int logging)
{
#ifdef DEBUGMODE
	int logAfter = 0;
	extendedLogging = 1;
	if(logging == 1 && mutex->LockCount != -1) {
		logAfter = 1;
		debugLogA("%s(): %i before EnterCriticalSection %s LockCount=%ld RecursionCount=%ld OwningThread=%ld", callingFunction, sectionNumber, mutexName, mutex->LockCount, mutex->RecursionCount, mutex->OwningThread);
	}
#endif
	EnterCriticalSection(mutex);
#ifdef DEBUGMODE
	if(logging == 1 && logAfter == 1) debugLogA("%s(): %i after EnterCriticalSection %s LockCount=%ld RecursionCount=%ld OwningThread=%ld", callingFunction, sectionNumber, mutexName, mutex->LockCount, mutex->RecursionCount, mutex->OwningThread);
	extendedLogging = 0;
#endif

}

inline void GGPROTO::gg_LeaveCriticalSection(CRITICAL_SECTION* mutex, char* callingFunction, int sectionNumber, int returnNumber, char* mutexName, int logging) /*0-never, 1-debug, 2-all*/
{
#ifdef DEBUGMODE
	if(logging == 1 && extendedLogging == 1) debugLogA("%s(): %i.%i LeaveCriticalSection %s", callingFunction, sectionNumber, returnNumber, mutexName);
#endif
	LeaveCriticalSection(mutex);
}

inline void GGPROTO::gg_sleep(DWORD miliseconds, BOOL alterable, char* callingFunction, int sleepNumber, int logging){
	SleepEx(miliseconds, alterable);
#ifdef DEBUGMODE
	if(logging == 1 && extendedLogging == 1) debugLogA("%s(): %i after SleepEx(%ld,%u)", callingFunction, sleepNumber, miliseconds, alterable);
#endif
}


#endif
