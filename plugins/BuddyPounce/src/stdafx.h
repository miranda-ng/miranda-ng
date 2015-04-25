#ifndef _COMMONHEADERS_H
#define _COMMONHEADERS_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <win2k.h>

#include "resource.h"
#include "Version.h"

//=======================================================
//	Definitions
//=======================================================
#define modname			"buddypounce"
#define modFullname		"Buddy Pounce"
#define msg(a,b)		MessageBox(0,a,b,MB_OK)

#define SECONDSINADAY	86400

/* flags for the sending and recieving.... */
#define ANY 1
#define ONLINE 2
#define AWAY 4
#define NA 8
#define OCCUPIED 16
#define DND 32
#define FFC 64
#define INVISIBLE 128
#define PHONE 256
#define LUNCH 512

//=======================================================
//  Variables
//=======================================================

struct windowInfo
{
	MCONTACT hContact;
	HWND SendIfMy;
	HWND SendWhenThey;
};

struct SendPounceDlgProcStruct
{
	MCONTACT hContact;
	int timer;
	TCHAR *message;
};

extern HINSTANCE hInst;

//=======================================================
//  Functions
//=======================================================

// main.c
void SendPounce(TCHAR* text, MCONTACT hContact);

//dialog.c
INT_PTR CALLBACK BuddyPounceDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK BuddyPounceSimpleDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK BuddyPounceOptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SendPounceDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateMessageAcknowlegedWindow(MCONTACT hContact, int SentSuccess);

#endif //_COMMONHEADERS_H