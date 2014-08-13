#pragma once

#define _CRT_SECURE_NO_DEPRECATE 1
#define TEXT_LEN	1024
#define CP_ACP		0 

#define code_page CP_ACP;


// System includes
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <shlobj.h>
#include <time.h>

#include "resource.h"
#include "version.h"
#include "util.h"

#pragma warning (push)
#pragma warning (disable: 4100) // unreferenced formal parameter

// Miranda Includes
#include <newpluginapi.h>
#include <m_utils.h>
#include <m_protosvc.h>
#include <m_protomod.h>
#include <m_skin.h>
#include <m_message.h>
#include <m_database.h>
#include <m_clist.h>
#include <m_system.h>
#include <m_folders.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_userinfo.h>
#include <m_avatars.h>
#include <m_contacts.h>
#include <m_metacontacts.h>
#include <m_popup.h>
#include <m_core.h>

#define MIRANDA_CUSTOM_LP

#pragma warning (pop)

// MyDetails defines

// wParam=NULL
// lParam=(char *) new nickname - do not free
// return=0 for sucess
#define PS_SETMYNICKNAME "/SetNickname"

// Optional, default value is 1024
// wParam=NULL
// lParam=NULL
// return= <=0 for error, >0 the max length of the nick
#define PS_GETMYNICKNAMEMAXLENGTH "/GetMyNicknameMaxLength"

// wParam=(char *)Buffer to file name
// lParam=(int)Buffer size
// return=0 for sucess
#define PS_GETMYAVATAR "/GetMyAvatar"

// wParam=0
// lParam=(const char *)Avatar file name
// return=0 for sucess
#define PS_SETMYAVATAR "/SetMyAvatar"


// Program defines
#define SKYPE_NAME		"Username"
#define SKYPE_PROTO		"PROTOCOL 7"
#define SKYPE_PROTONAME	g_szProtoName	// Name of our protocol, taken from .DLL name
#define MAX_MSGS		128		// Maximum messages in queue
#define MAX_USERLEN     32      // Maximum length of a username in Skype
#define PING_INTERVAL	10000	// Ping every 10000 msec to see if Skype is still available
#define USEPOPUP		1		// Use the popup-plugin?
#define TIMEOUT_MSGSEND 60000	// Stolen from msgdialog.c
#define MAX_MSG_AGE		30		// Maximum age in seconds before a Message from queue gets trashed
#define SKYPEBUG_OFFLN	1		// Activate fix for the SkypeAPI Offline-Bug

// Program hooks
typedef struct {
	char	ChatNew[MAXMODULELABELLENGTH];
	char	SetAvatar[MAXMODULELABELLENGTH];
	char	SendFile[MAXMODULELABELLENGTH];
	char	HoldCall[MAXMODULELABELLENGTH];
	char	AnswerCall[MAXMODULELABELLENGTH];
	char	ImportHistory[MAXMODULELABELLENGTH];
	char	AddUser[MAXMODULELABELLENGTH];
	char	SkypeOutCallUser[MAXMODULELABELLENGTH];
	char	CallHangupUser[MAXMODULELABELLENGTH];
	char	CallUser[MAXMODULELABELLENGTH];
} SKYPE_SVCNAMES;
#define SKYPE_CALL g_svcNames.CallUser
#define SKYPE_CALLHANGUP g_svcNames.CallHangupUser
#define SKYPEOUT_CALL g_svcNames.SkypeOutCallUser
#define SKYPE_ADDUSER g_svcNames.AddUser
#define SKYPE_IMPORTHISTORY g_svcNames.ImportHistory
#define SKYPE_ANSWERCALL g_svcNames.AnswerCall
#define SKYPE_HOLDCALL g_svcNames.HoldCall
#define SKYPE_SENDFILE g_svcNames.SendFile
#define SKYPE_SETAVATAR g_svcNames.SetAvatar
#define SKYPE_CHATNEW g_svcNames.ChatNew
#define EVENTTYPE_CALL 2000

#define WM_COPYDATALOCAL	WM_USER+100		// WM_COPYDATA for local window communication, needed due to Win98 bug

#ifndef __SKYPESVC_C__
extern SKYPE_SVCNAMES g_svcNames;
#endif

// Skype API Communication services
#define PSS_SKYPEAPIMSG	"/SendSkypeAPIMsg"
#define SKYPE_REGPROXY	"/RegisterProxySvc"

#define MUUID_SKYPE_CALL { 0x245241eb, 0x178c, 0x4b3f, { 0x91, 0xa, 0x4c, 0x4d, 0xf0, 0xa0, 0xc3, 0xb6 } }


// Common used code-pieces
#define OUTPUT(a) ShowMessage(IDI_ERRORS, a, 1);
#define OUTPUTA(a) ShowMessageA(IDI_ERRORS, a, 1);

typedef void ( __cdecl* pThreadFunc )( void* );

// Prototypes

void __cdecl SkypeSystemInit(char *);
void __cdecl MsgPump (char *dummy);
void PingPong(void);
void CheckIfApiIsResponding(char *);
void TellError(DWORD err);
int ShowMessage(int, TCHAR*, int);
#ifdef _UNICODE
int ShowMessageA(int iconID, char *lpzText, int mustShow);
#else
#define ShowMessageA ShowMessage
#endif
void EndCallThread(char *);
void GetInfoThread(void *);
int OnDetailsInit( WPARAM, LPARAM );
INT_PTR SkypeGetAvatarInfo(WPARAM wParam,LPARAM lParam);
INT_PTR SkypeGetAvatarCaps(WPARAM wParam,LPARAM lParam);
INT_PTR SkypeGetAwayMessage(WPARAM wParam,LPARAM lParam);
int HookContactAdded(WPARAM wParam, LPARAM lParam);
int HookContactDeleted(WPARAM wParam, LPARAM lParam);
INT_PTR ImportHistory(WPARAM wParam, LPARAM lParam);
int CreateTopToolbarButton(WPARAM wParam, LPARAM lParam);
int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeSetStatus(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeGetStatus(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeGetInfo(WPARAM wParam,LPARAM lParam);
INT_PTR SkypeAddToList(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeBasicSearch(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeSendMessage(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeRecvMessage(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeUserIsTyping(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeSendAuthRequest(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeRecvAuth(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeAuthAllow(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeAuthDeny(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeAddToListByEvent(WPARAM wParam, LPARAM lParam);
INT_PTR SkypeRegisterProxy(WPARAM wParam, LPARAM lParam);
time_t SkypeTime(time_t *timer);
void MessageSendWatchThread(void*);
int OkToExit(WPARAM wParam, LPARAM lParam);
int MirandaExit(WPARAM wParam, LPARAM lParam);
int __stdcall EnterBitmapFileName( char* szDest );
void CleanupNicknames(char *dummy);
int InitVSApi();
int FreeVSApi();
MCONTACT GetMetaHandle(DWORD dwId);
void LaunchSkypeAndSetStatusThread(void *newStatus);

// Structs

typedef struct {
	char *SkypeSetting;
	char *MirandaSetting;
} settings_map;
