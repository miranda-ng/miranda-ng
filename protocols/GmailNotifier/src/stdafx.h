#pragma once

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include "resource.h"

#include "newpluginapi.h"
#include "m_clistint.h"
#include "m_skin.h"
#include "m_langpack.h"
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
#define MODULENAME		"GmailMNotifier"
#define _MAX_DOWN_BUFFER 65536
#define LINK "https://accounts.google.com/ServiceLogin?continue=https%3A%2F%2Fmail.google.com%2Fmail&service=mail&passive=true&Email="
#define FORMDATA1 "<body onload=document.gmail.submit();><form name=gmail action=https://www.google.com/a/"
#define FORMDATA2 "/LoginAction method=POST><input type=hidden name=continue value=https://mail.google.com/a/"
#define FORMDATA3 "><INPUT type=hidden value=mail name=service>"
// #define STR1 "javascript:document.write('<form name=gmail action=https://www.google.com/a/"
// #define STR2 "/LoginAction method=post><input type=hidden name=continue value=https://mail.google.com/hosted/"
// #define STR3 "><input type=hidden value=mail name=service><input type=hidden name=userName value="
// #define STR4 "><input type=hidden name=password value="
// #define STR5 ">');document.gmail.submit();"
//#define LINK2 "https://www.google.com/a/altmanoptik.com/LoginAction?continue=https%3A%2F%2Fmail.google.com%2Fhosted%2Faltmanoptik.com&service=mail&userName=test&password=123456"

struct resultLink
{
	char content[64];
	struct resultLink *next;
};

struct Account : public MZeroedObject
{
	char name[256];
	char pass[256];
	char hosted[64];
	MCONTACT hContact;
	int oldResults_num;
	int results_num;
	resultLink results;
	HWND popUpHwnd;
	bool IsChecking;
};

extern HNETLIBUSER hNetlibUser;
extern UINT hTimer;
extern short ID_STATUS_NONEW;
extern BOOL optionWindowIsOpen;
extern OBJLIST<Account> g_accs;

INT_PTR Notifying(WPARAM, LPARAM);
INT_PTR PluginMenuCommand(WPARAM, LPARAM);
void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
BOOL GetBrowser(char *);

void NotifyUser(Account *);
int OptInit(WPARAM, LPARAM);
void Check_ThreadFunc(void *);
int OpenBrowser(WPARAM, LPARAM);
void DeleteResults(resultLink *);
void BuildList(void);

Account* GetAccountByContact(MCONTACT hContact);

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMOption<bool> bNotifierOnTray, bNotifierOnPop;
	CMOption<bool> bShowCustomIcon, bUseOnline, bLogThreads;
	CMOption<uint32_t> AutoLogin, OpenUsePrg, circleTime, popupDuration, popupBgColor, popupTxtColor;

	CMPlugin();

	int Load() override;
	int Unload() override;
};
