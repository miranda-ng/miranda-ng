////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2009 Adam Strzelecki <ono+miranda@java.pl>
// Copyright (c) 2009-2012 Bartosz Białek
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

struct GaduProto : public PROTO<GaduProto>
{
	GaduProto(const char*, const wchar_t*);
	~GaduProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT* psr) override;
				 
	HANDLE   FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szPath) override;
	int      FileCancel(MCONTACT hContact, HANDLE hTransfer) override;
	int      FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szReason) override;
				 
	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
	int      GetInfo(MCONTACT hContact, int infoType) override;
				
	HANDLE   SearchBasic(const wchar_t* id) override;
	HANDLE   SearchByName(const wchar_t* nick, const wchar_t* firstName, const wchar_t* lastName) override;
	HWND     SearchAdvanced(HWND owner) override;
	HWND     CreateExtendedSearchUI(HWND owner) override;
				
	HANDLE   SendFile(MCONTACT hContact, const wchar_t* szDescription, wchar_t** ppszFiles) override;
	int      SendMsg(MCONTACT hContact, int flags, const char* msg) override;
				
	int      SetApparentMode(MCONTACT hContact, int mode) override;
	int      SetStatus(int iNewStatus) override;
				
	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      SetAwayMsg(int m_iStatus, const wchar_t* msg) override;
				
	int      UserIsTyping(MCONTACT hContact, int type) override;
				
	void     OnBuildProtoMenu(void) override;
	void     OnContactDeleted(MCONTACT) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

	//////////////////////////////////////////////////////////////////////////////////////
	// Services

	INT_PTR  __cdecl blockuser(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl getmyawaymsg(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl get_acc_mgr_gui(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl leavechat(WPARAM wParam, LPARAM lParam);

	//////////////////////////////////////////////////////////////////////////////////////
	// Threads

	void __cdecl mainthread(void *empty);
	void __cdecl getawaymsgthread(void *hContact);
	void __cdecl dccmainthread(void *);
	void __cdecl remindpasswordthread(void *param);

	//////////////////////////////////////////////////////////////////////////////////////
	// Events

	int __cdecl dbsettingchanged(WPARAM wParam, LPARAM lParam);

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
	void gg_sleep(uint32_t miliseconds, BOOL alterable, char* callingFunction, int sleepNumber, int logging);

	/* Global GG functions */
	void notifyuser(MCONTACT hContact, int refresh);
	void setalloffline();
	void disconnect();
	MCONTACT getcontact(uin_t uin, int create, int inlist, wchar_t *nick);
	int isonline();
	int refreshstatus(int status);

	void broadcastnewstatus(int newStatus);
	void cleanuplastplugin(uint32_t version);
	void notifyall();
	void changecontactstatus(uin_t uin, int status, const wchar_t *idescr, int time, uint32_t remote_ip, uint16_t remote_port, uint32_t version);
	wchar_t *getstatusmsg(int status);
	void dccstart();
	void dccconnect(uin_t uin);
	int gettoken(GGTOKEN *token);
	void parsecontacts(char *contacts);
	void remindpassword(uin_t uin, const char *email);

	/* Avatar functions */
	void getAvatarFilename(MCONTACT hContact, wchar_t *pszDest, int cbLen);
	void requestAvatarTransfer(MCONTACT hContact, char *szAvatarURL);
	void requestAvatarInfo(MCONTACT hContact, int iWaitFor);
	void getOwnAvatar();
	void setAvatar(const wchar_t *szFilename);
	bool getAvatarFileInfo(uin_t uin, char **avatarurl, char **avatarts);

	INT_PTR __cdecl getavatarcaps(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl getavatarinfo(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl getmyavatar(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl setmyavatar(WPARAM wParam, LPARAM lParam);

	void initavatarrequestthread();

	void __cdecl avatarrequestthread(void*);
	void __cdecl getOwnAvatarThread(void*);
	void __cdecl setavatarthread(void*);

	/* File transfer functions */
	HANDLE dccfileallow(HANDLE hTransfer, const wchar_t* szPath);
	HANDLE dcc7fileallow(HANDLE hTransfer, const wchar_t* szPath);

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
	void *img_loadpicture(gg_event* e, wchar_t *szFileName);
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
	wchar_t * gc_getchat(uin_t sender, uin_t *recipients, int recipients_count);
	GGGC *gc_lookup(const wchar_t *id);
	int gc_changenick(MCONTACT hContact, wchar_t *ptszNick);

	int __cdecl gc_event(WPARAM wParam, LPARAM lParam);

	INT_PTR __cdecl gc_openconf(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl gc_clearignored(WPARAM wParam, LPARAM lParam);

	/* Popups functions */
	void initpopups();
	void showpopup(const wchar_t* nickname, const wchar_t* msg, int flags);

	/* Sessions functions */
	INT_PTR __cdecl sessions_view(WPARAM wParam, LPARAM lParam);
	void sessions_updatedlg();
	BOOL sessions_closedlg();
	void sessions_menus_init(HGENMENU hRoot);

	void block_init();
	void block_uninit();

	//////////////////////////////////////////////////////////////////////////////////////
	GaduOptions m_gaduOptions;

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
	struct gg_session *m_sess;
	struct gg_dcc *m_dcc;
	HANDLE hEvent;
	HANDLE hConnStopEvent;
	SOCKET sock;
	UINT_PTR timer;
	struct Modemsg
	{
		wchar_t *online;
		wchar_t *away;
		wchar_t *dnd;
		wchar_t *freechat;
		wchar_t *invisible;
		wchar_t *offline;
	};
	Modemsg modemsg;

	HGENMENU hMainMenu[7];
	HGENMENU hBlockMenuItem, hImageMenuItem, hInstanceMenuItem;
	HANDLE   hPrebuildMenuHook;
	HANDLE   hImagesFolder;
	HWND     hwndSessionsDlg;
	HANDLE   hPopupNotify, hPopupError;
};

struct GGUSERUTILDLGDATA
{
	int mode;
	uin_t uin;
	char *pass;
	char *email;
	GaduProto *gg;
};


#ifdef DEBUGMODE
inline void GaduProto::gg_EnterCriticalSection(CRITICAL_SECTION* mutex, char* callingFunction, int sectionNumber, char* mutexName, int logging)
#else
inline void GaduProto::gg_EnterCriticalSection(CRITICAL_SECTION* mutex, char*, int, char *, int)
#endif
{
#ifdef DEBUGMODE
	int logAfter = 0;
	extendedLogging = 1;
	if (logging == 1 && mutex->LockCount != -1) {
		logAfter = 1;
		debugLogA("%s(): %i before EnterCriticalSection %s LockCount=%ld RecursionCount=%ld OwningThread=%ld", callingFunction, sectionNumber, mutexName, mutex->LockCount, mutex->RecursionCount, mutex->OwningThread);
	}
#endif
	EnterCriticalSection(mutex);
#ifdef DEBUGMODE
	if (logging == 1 && logAfter == 1)
		debugLogA("%s(): %i after EnterCriticalSection %s LockCount=%ld RecursionCount=%ld OwningThread=%ld", callingFunction, sectionNumber, mutexName, mutex->LockCount, mutex->RecursionCount, mutex->OwningThread);
	extendedLogging = 0;
#endif

}

#ifdef DEBUGMODE
inline void GaduProto::gg_LeaveCriticalSection(CRITICAL_SECTION* mutex, char* callingFunction, int sectionNumber, int returnNumber, char* mutexName, int logging) /*0-never, 1-debug, 2-all*/
#else
inline void GaduProto::gg_LeaveCriticalSection(CRITICAL_SECTION* mutex, char *, int, int, char *, int) /*0-never, 1-debug, 2-all*/
#endif
{
#ifdef DEBUGMODE
	if (logging == 1 && extendedLogging == 1)
		debugLogA("%s(): %i.%i LeaveCriticalSection %s", callingFunction, sectionNumber, returnNumber, mutexName);
#endif
	LeaveCriticalSection(mutex);
}

#ifdef DEBUGMODE
inline void GaduProto::gg_sleep(uint32_t miliseconds, BOOL alterable, char* callingFunction, int sleepNumber, int logging) {
#else
inline void GaduProto::gg_sleep(uint32_t miliseconds, BOOL alterable, char* callingFunction, int, int) {
#endif
	SleepEx(miliseconds, alterable);
#ifdef DEBUGMODE
	if (logging == 1 && extendedLogging == 1)
		debugLogA("%s(): %i after SleepEx(%ld,%u)", callingFunction, sleepNumber, miliseconds, alterable);
#endif
}

void crc_gentable(void);

struct CMPlugin : public ACCPROTOPLUGIN<GaduProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif
