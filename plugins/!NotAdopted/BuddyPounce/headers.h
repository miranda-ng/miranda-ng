#ifndef _COMMONHEADERS_H
#define _COMMONHEADERS_H
#pragma warning( disable : 4786 ) // limitation in MSVC's debugger.
//=====================================================
//	Includes 
//=====================================================


#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <string.h>
#include "../miranda_src/SDK/Headers_c/newpluginapi.h"
#include "../miranda_src/SDK/Headers_c/m_clist.h"
#include "../miranda_src/SDK/Headers_c/m_clui.h"
#include "../miranda_src/SDK/Headers_c/m_skin.h"
#include "../miranda_src/SDK/Headers_c/m_langpack.h"
#include "../miranda_src/SDK/Headers_c/m_protomod.h"
#include "../miranda_src/SDK/Headers_c/m_database.h"
#include "../miranda_src/SDK/Headers_c/m_system.h"
#include "../miranda_src/SDK/Headers_c/m_protocols.h"
#include "../miranda_src/SDK/Headers_c/m_userinfo.h"
#include "../miranda_src/SDK/Headers_c/m_options.h"
#include "../miranda_src/SDK/Headers_c/m_protosvc.h"
#include "../miranda_src/SDK/Headers_c/m_utils.h"
#include "../miranda_src/SDK/Headers_c/m_ignore.h"
#include "../miranda_src/SDK/Headers_c/m_clc.h"
#include "../miranda_src/SDK/Headers_c/m_history.h"
//#include "../miranda_src/SDK/Headers_c/win2k.h"
#include "../miranda_src/SDK/Headers_c/m_popup.h"
#include "../miranda_src/SDK/Headers_c/m_file.h"
#include "../miranda_src/SDK/Headers_c/m_uninstaller.h"
#include "resource.h"

#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
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
PLUGINLINK *pluginLink;
HINSTANCE hInst;

struct windowInfo {
	HANDLE hContact;
	HWND SendIfMy;
	HWND SendWhenThey;
};

struct SendPounceDlgProcStruct {
	HANDLE hContact;
	int timer;
	char *message;
};
//=======================================================
//  Functions
//=======================================================

// main.c
void SendPounce(char* text, HANDLE hContact);

//dialog.c 
BOOL CALLBACK BuddyPounceDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK BuddyPounceSimpleDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK BuddyPounceOptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SendPounceDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateMessageAcknowlegedWindow(HANDLE hContact, int SentSuccess);

#ifndef NDEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#pragma comment(lib,"comctl32.lib")

#endif //_COMMONHEADERS_H