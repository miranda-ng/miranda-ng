/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2007  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _TLEN_H_
#define _TLEN_H_

#define __try
#define __except(x) if (0)
#define __finally

#define _try __try
#define _except __except
#define _finally __finally

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#define ENABLE_LOGGING

/*******************************************************************
 * Global compilation flags
 *******************************************************************/

/*******************************************************************
 * Global header files
 *******************************************************************/

#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>
#include <win2k.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_netlib.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_contacts.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_clui.h>
#include <m_options.h>
#include <m_userinfo.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_message.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_avatars.h>
#include <m_nudge.h>

#include "tlen_xml.h"
#include "crypto/polarssl/aes.h"
#include "crypto/polarssl/bignum.h"

/*******************************************************************
 * Global constants
 *******************************************************************/
#define TLEN_DEFAULT_PORT 443
#define TLEN_IQID "mim_"
#define TLEN_REGISTER   "http://reg.tlen.pl/"
#define TLEN_MAX_SEARCH_RESULTS_PER_PAGE 20

// User-defined message
#define WM_TLEN_REFRESH						(WM_USER + 100)
// Error code
#define TLEN_ERROR_REDIRECT				302
#define TLEN_ERROR_BAD_REQUEST			400
#define TLEN_ERROR_UNAUTHORIZED			401
#define TLEN_ERROR_PAYMENT_REQUIRED		402
#define TLEN_ERROR_FORBIDDEN				403
#define TLEN_ERROR_NOT_FOUND				404
#define TLEN_ERROR_NOT_ALLOWED			405
#define TLEN_ERROR_NOT_ACCEPTABLE			406
#define TLEN_ERROR_REGISTRATION_REQUIRED	407
#define TLEN_ERROR_REQUEST_TIMEOUT		408
#define TLEN_ERROR_CONFLICT				409
#define TLEN_ERROR_INTERNAL_SERVER_ERROR	500
#define TLEN_ERROR_NOT_IMPLEMENTED		501
#define TLEN_ERROR_REMOTE_SERVER_ERROR	502
#define TLEN_ERROR_SERVICE_UNAVAILABLE	503
#define TLEN_ERROR_REMOTE_SERVER_TIMEOUT	504

#define TLEN_ALERTS_ACCEPT_ALL 0
#define TLEN_ALERTS_IGNORE_NIR 1
#define TLEN_ALERTS_IGNORE_ALL 2

#define TLEN_IMAGES_ACCEPT_ALL 0
#define TLEN_IMAGES_IGNORE_NIR 1
#define TLEN_IMAGES_IGNORE_ALL 2

#define TLEN_MUC_ASK		0
#define TLEN_MUC_ACCEPT_IR  1
#define TLEN_MUC_ACCEPT_ALL 2
#define TLEN_MUC_IGNORE_NIR 3
#define TLEN_MUC_IGNORE_ALL 4

#define IDC_STATIC (-1)

/*******************************************************************
 * Global data structures and data type definitions
 *******************************************************************/

typedef enum {
	LIST_ROSTER,	// Roster list
	LIST_CHATROOM,	// Groupchat room currently joined
	LIST_FILE,		// Current file transfer session
	LIST_INVITATIONS,// Invitations to be sent
	LIST_SEARCH,	 // Rooms names being searched
	LIST_VOICE,
	LIST_PICTURE
} TLEN_LIST;

typedef enum {
	IQ_PROC_NONE,
	IQ_PROC_GETSEARCH
} TLEN_IQ_PROCID;

typedef enum {
	SUB_NONE,
	SUB_TO,
	SUB_FROM,
	SUB_BOTH
} TLEN_SUBSCRIPTION;

typedef struct {
	char *szOnline;
	char *szAway;
	char *szNa;
	char *szDnd;
	char *szFreechat;
	char *szInvisible;
} TLEN_MODEMSGS;

typedef struct {
	char mailBase[256];
	char mailMsg[256];
	int  mailMsgMthd;
	char mailIndex[256];
	int  mailIndexMthd;
	char mailLogin[256];
	int  mailLoginMthd;
	char mailCompose[256];
	int  mailComposeMthd;
	char avatarGet[256];
	int  avatarGetMthd;
	char avatarUpload[256];
	int  avatarUploadMthd;
	char avatarRemove[256];
	int  avatarRemoveMthd;
} TlenConfiguration;

typedef struct {
	BOOL useEncryption;
	BOOL reconnect;
	BOOL rosterSync;
	BOOL offlineAsInvisible;
	BOOL leaveOfflineMessage;
	int offlineMessageOption;
	BOOL ignoreAdvertisements;
	int alertPolicy;
	int groupChatPolicy;
	int voiceChatPolicy;
	int imagePolicy;
	BOOL enableAvatars;
	BOOL enableVersion;
	BOOL useNudge;
	BOOL logAlerts;
	BOOL useNewP2P;
	BOOL sendKeepAlive;
	BOOL savePassword;
} TlenOptions;


struct TLEN_IQ_FUNC_STRUCT;
struct TLEN_LIST_ITEM_STRUCT;
struct TLEN_VOICE_CONTROL_STRUCT;

struct TlenProtocol : public PROTO<TlenProtocol>
{
	TlenProtocol( const char*, const TCHAR* );
	~TlenProtocol();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	MCONTACT __cdecl AddToList( int flags, PROTOSEARCHRESULT* psr );
	virtual	MCONTACT __cdecl AddToListByEvent( int flags, int iContact, MEVENT hDbEvent );

	virtual	int	     __cdecl Authorize(MEVENT hDbEvent);
	virtual	int      __cdecl AuthDeny(MEVENT hDbEvent, const PROTOCHAR* szReason);
	virtual	int      __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int      __cdecl AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage);

	virtual	HANDLE   __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath);
	virtual	int      __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int      __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason);
	virtual	int      __cdecl FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const PROTOCHAR* email);
	virtual	HANDLE    __cdecl SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvContacts(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTOFILEEVENT*);
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvUrl(MCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList);
	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);
	virtual	int       __cdecl SendUrl(MCONTACT hContact, int flags, const char* url);

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt);
	virtual	int       __cdecl SetAwayMsg(int iStatus, const PROTOCHAR* msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);

	//====================================================================================
	// Services

	INT_PTR __cdecl GetName(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetAvatarInfo(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl SendAlert(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetAvatarCaps(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl SetMyAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetMyAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl GetStatus(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl AccMgrUI(WPARAM wParam, LPARAM lParam);

	INT_PTR __cdecl MUCMenuHandleChats(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl MUCMenuHandleMUC(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl MenuHandleInbox(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl ContactMenuHandleSendPicture(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl MUCContactMenuHandleMUC(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl VoiceContactMenuHandleVoice(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl ContactMenuHandleRequestAuth(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl ContactMenuHandleGrantAuth(WPARAM wParam, LPARAM lParam);

	//====================================================================================
	// Events 

	int __cdecl OnModulesLoaded(WPARAM wParam, LPARAM lParam);
	int __cdecl OptionsInit(WPARAM wParam, LPARAM lParam);
	int __cdecl TlenDbSettingChanged(WPARAM wParam, LPARAM lParam);
	int __cdecl TlenContactDeleted(WPARAM wParam, LPARAM lParam);
	int __cdecl PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
	int __cdecl PreShutdown(WPARAM wParam, LPARAM lParam);

	int __cdecl UserInfoInit(WPARAM wParam, LPARAM lParam);

	int __cdecl MUCHandleEvent(WPARAM wParam, LPARAM lParam);

	//====================================================================================
	HANDLE hFileNetlibUser;

	TLEN_MODEMSGS modeMsgs;

	struct ThreadDataStruct *threadData;
	HANDLE hTlenNudge;
	HGENMENU hMenuMUC;
	HGENMENU hMenuChats;
	HGENMENU hMenuInbox;
	HGENMENU hMenuContactMUC;
	HGENMENU hMenuContactVoice;
	HGENMENU hMenuContactGrantAuth;
	HGENMENU hMenuContactRequestAuth;
	HGENMENU hMenuPicture;

	int listsCount;
	struct TLEN_LIST_ITEM_STRUCT *lists;
	CRITICAL_SECTION csLists;

	int iqCount;
	int iqAlloced;
	struct TLEN_IQ_FUNC_STRUCT *iqList;
	CRITICAL_SECTION csIqList;

	CRITICAL_SECTION csSerial;
	unsigned int serial;
	BOOL isOnline;
	BOOL isConnected;

	CRITICAL_SECTION modeMsgMutex;

	void initMenuItems();
	HGENMENU hMenuRoot;

	char *searchJID;
	int searchID;
	int searchIndex;
	char *searchQuery;
	int searchQueryLen;

	CRITICAL_SECTION csSend;

	HWND voiceDlgHWND;
	struct TLEN_VOICE_CONTROL_STRUCT *playbackControl;
	struct TLEN_VOICE_CONTROL_STRUCT *recordingControl;
	int framesAvailableForPlayback;
	int availOverrunValue;

	TlenOptions tlenOptions;

};



typedef struct ThreadDataStruct{
	HANDLE hThread;
	char *streamId;
	char username[128];
	char password[128];
	char server[128];
	char manualHost[128];
	char avatarToken[128];
	char avatarHash[64];
	int  avatarFormat;
	WORD port;
	BOOL useEncryption;

	HANDLE s;    //HANDLE from CallService(MS_NETLIB_OPENCONNECTION (tlen_ws.c:68)
	aes_context aes_in_context;
	aes_context aes_out_context;
	unsigned char aes_in_iv[16];
	unsigned char aes_out_iv[16];

	BOOL useAES;
	TlenConfiguration tlenConfig;
	TlenProtocol *proto;
} ThreadData;


typedef enum { FT_CONNECTING, FT_INITIALIZING, FT_RECEIVING, FT_DONE, FT_ERROR, FT_DENIED, FT_SWITCH } TLEN_FILE_STATE;
typedef enum { FT_RECV, FT_SEND} TLEN_FILE_MODE;
typedef struct TLEN_FILE_TRANSFER_STRUCT{
	MCONTACT hContact;
	HANDLE s;
	NETLIBNEWCONNECTIONPROC_V2 pfnNewConnectionV2;
	TLEN_FILE_STATE state;
	char *jid;
	int fileId;
	char *iqId;
	int	mode;

	// Used by file receiving only
	char *hostName;
	WORD wPort;
	char *localName;
	WORD wLocalPort;
	char *szSavePath;
	long fileReceivedBytes;
	long fileTotalSize;

	// Used by file sending only
	HANDLE hFileEvent;
	int fileCount;
	char **files;
	long *filesSize;
	//long fileTotalSize;		// Size of the current file (file being sent)
	long allFileTotalSize;
	long allFileReceivedBytes;
	char *szDescription;
	int currentFile;

	// New p2p
	BOOL newP2P;
	char *id2;
	SOCKET udps;
	aes_context aes_context;
	unsigned char aes_iv[16];
	TlenProtocol *proto;

} TLEN_FILE_TRANSFER;

typedef struct {
	PROTOSEARCHRESULT hdr;
	char jid[256];
} TLEN_SEARCH_RESULT;

typedef struct {
	char *iqId;
	PROTOSEARCHRESULT hdr;
	char jid[256];
} TLEN_CONFERENCE;


typedef struct {
	int id;
	TCHAR *name;
} TLEN_FIELD_MAP;


/*******************************************************************
 * Global variables
 *******************************************************************/
extern HINSTANCE hInst;
extern HANDLE hMainThread;

/*******************************************************************
 * Function declarations
 *******************************************************************/
HICON GetIcolibIcon(int iconId);
void ReleaseIcolibIcon(HICON hIcon);

void __cdecl TlenServerThread(ThreadData *info);
// tlen_ws.cpp
BOOL TlenWsInit(TlenProtocol *proto);
void TlenWsUninit(TlenProtocol *proto);
HANDLE TlenWsConnect(TlenProtocol *proto, char *host, WORD port);
int TlenWsSend(TlenProtocol *proto, HANDLE s, char *data, int datalen);
int TlenWsRecv(TlenProtocol *proto, HANDLE s, char *data, long datalen);
int TlenWsSendAES(TlenProtocol *proto, char *data, int datalen, aes_context *aes_ctx, unsigned char *aes_iv);
int TlenWsRecvAES(TlenProtocol *proto, char *data, long datalen, aes_context *aes_ctx, unsigned char *aes_iv);

// tlen_util.cpp
void TlenSerialInit(TlenProtocol *proto);
void TlenSerialUninit(TlenProtocol *proto);
unsigned int TlenSerialNext(TlenProtocol *proto);
int TlenSend(TlenProtocol *proto, const char *fmt, ...);
MCONTACT TlenHContactFromJID(TlenProtocol *proto, const char *jid);
char *TlenJIDFromHContact(TlenProtocol *proto, MCONTACT hContact);
char *TlenLoginFromJID(const char *jid);
char *TlenResourceFromJID(const char *jid);
char *TlenNickFromJID(const char *jid);
char *TlenLocalNickFromJID(const char *jid);
char *TlenGroupEncode(const char *str);
char *TlenGroupDecode(const char *str);
char *TlenSha1(char *str);
char *TlenSha1(char *str, int len);
char *TlenPasswordHash(const char *str);
void TlenUrlDecode(char *str);
char *TlenUrlEncode(const char *str);
char *TlenTextEncode(const char *str);
char *TlenTextDecode(const char *str);
void TlenLogMessage(TlenProtocol *proto, MCONTACT hContact, DWORD flags, const char *message);
BOOL IsAuthorized(TlenProtocol *proto, const char *jid);
//char *TlenGetVersionText();
time_t TlenIsoToUnixTime(char *stamp);
time_t TlenTimeToUTC(time_t time);
void TlenSendPresence(TlenProtocol *proto,int status);
void TlenStringAppend(char **str, int *sizeAlloced, const char *fmt, ...);
//char *TlenGetClientJID(char *jid);
// tlen_misc.cpp
void TlenDBAddEvent(TlenProtocol *proto, MCONTACT hContact, int eventType, DWORD flags, PBYTE pBlob, DWORD cbBlob);
void TlenDBAddAuthRequest(TlenProtocol *proto, char *jid, char *nick);
MCONTACT TlenDBCreateContact(TlenProtocol *proto, char *jid, char *nick, BOOL temporary);
// tlen_svc.cpp
int TlenRunSearch(TlenProtocol *proto);
// tlen_opt.cpp
void TlenLoadOptions(TlenProtocol *proto);
// tlen_voice.cpp
int TlenVoiceCancelAll(TlenProtocol *proto);
// tlen_advsearch.cpp
extern TLEN_FIELD_MAP tlenFieldGender[];
extern TLEN_FIELD_MAP tlenFieldLookfor[];
extern TLEN_FIELD_MAP tlenFieldStatus[];
extern TLEN_FIELD_MAP tlenFieldOccupation[];
extern TLEN_FIELD_MAP tlenFieldPlan[];
// tlen_advsearch.cpp
INT_PTR CALLBACK TlenAdvSearchDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
char *TlenAdvSearchCreateQuery(HWND hwndDlg, int iqId);


#endif
