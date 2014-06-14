#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <wininet.h>
#include <time.h>
#include <stdio.h>
#include "resource.h"

#include "newpluginapi.h"
#include "m_clist.h"
#include "m_clui.h"
#include "m_skin.h"
#include "m_langpack.h"
#include "m_protomod.h"
#include "m_database.h"
#include "m_system.h"
#include "m_protocols.h"
#include "m_userinfo.h"
#include "m_options.h"
#include "m_protosvc.h"
#include "m_utils.h"
#include "m_ignore.h"
#include "m_clc.h"
#include "m_popup.h"
#include "m_netlib.h"

#define WM_SHELLNOTIFY 	WM_USER+5
#define IDI_TRAY		WM_USER+6
#define pluginName		"GmailMNotifier"
#define _MAX_DOWN_BUFFER 65536
#define LINK _T("https://www.google.com/accounts/ServiceLoginAuth?continue=https%3A%2F%2Fmail.google.com%2Fmail&service=mail&Email=")
#define FORMDATA1 _T("<body onload=document.gmail.submit();><form name=gmail action=https://www.google.com/a/")
#define FORMDATA2 _T("/LoginAction method=POST><input type=hidden name=continue value=https://mail.google.com/a/")
#define FORMDATA3 _T("><INPUT type=hidden value=mail name=service>")
// #define STR1 "javascript:document.write('<form name=gmail action=https://www.google.com/a/"
// #define STR2 "/LoginAction method=post><input type=hidden name=continue value=https://mail.google.com/hosted/"
// #define STR3 "><input type=hidden value=mail name=service><input type=hidden name=userName value="
// #define STR4 "><input type=hidden name=password value="
// #define STR5 ">');document.gmail.submit();"
//#define LINK2 "https://www.google.com/a/altmanoptik.com/LoginAction?continue=https%3A%2F%2Fmail.google.com%2Fhosted%2Faltmanoptik.com&service=mail&userName=test&password=123456"

typedef struct s_resultLink{
	TCHAR content[64];
	struct s_resultLink *next;
}resultLink;

typedef struct s_Account{
	TCHAR name[64];
	TCHAR pass[64];
	TCHAR hosted[64];
	MCONTACT hContact;
	int oldResults_num;
	int	results_num;
	resultLink results;
	HWND popUpHwnd;
	BOOL IsChecking;
}Account;

typedef struct s_optionSettings{
	int circleTime;
	BOOL notifierOnTray;
	BOOL notifierOnPop;
	int popupDuration;
	COLORREF popupTxtColor;
	COLORREF popupBgColor;
	int OpenUsePrg;
	BOOL ShowCustomIcon;
	BOOL UseOnline;
	int AutoLogin;
	BOOL LogThreads;
}optionSettings;

extern int acc_num;
extern Account *acc;
extern optionSettings opt;
extern HINSTANCE hInst;
extern UINT hTimer;
extern short ID_STATUS_NONEW;
extern BOOL optionWindowIsOpen;

INT_PTR Notifying(WPARAM, LPARAM);
INT_PTR PluginMenuCommand(WPARAM, LPARAM);
void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
BOOL GetBrowser(TCHAR *);
void CheckMailInbox(Account *);
void NotifyUser(Account *);
int OptInit(WPARAM, LPARAM);
void Check_ThreadFunc(void *);
void Login_ThreadFunc(void *);
int OpenBrowser(WPARAM , LPARAM);
int ParsePage(TCHAR *, resultLink *);
void DeleteResults(resultLink *);
void BuildList(void);

Account* GetAccountByContact(MCONTACT hContact);