//=====================================================
//	Includes (yea why not include lots of stuff :p )
//=====================================================
#ifndef COMMONHEADERS
#define COMMONHEADERS

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>
#include <winsock.h>
#include <shlobj.h>

#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <string.h>

struct DLGTEMPLATEEX
{
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
};

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_clui.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_protomod.h>
#include <m_database.h>
#include <m_system.h>
#include <m_icolib.h>
#include <m_protocols.h>
#include <m_userinfo.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_ignore.h>
#include <m_netlib.h>
#include <m_string.h>
#include <win2k.h>

#include "resource.h"

//=======================================================
//	Definitions
//=======================================================
#define MODNAME			"NIM_Contact"
#define modFullname		"Non-IM Contact"
#define MAXLINES	10000
#define MAX_STRING_LENGTH	10000
#define LINE_LENGTH 10000
#define msg(a,b)	MessageBoxA(0,a,b,MB_OK);
#define TIMER	(db_get_w(NULL, MODNAME, "Timer", 1) * 1000)
/* ERROR VALUES */
#define ERROR_NO_LINE_AFTER_VAR_F -1
#define ERROR_LINE_NOT_READ -2
#define ERROR_NO_FILE -3

//=======================================================
//	Defines
//=======================================================
//General
extern HINSTANCE hInst;
extern int LCStatus;
extern IconItem icoList[];

//Services.c
INT_PTR GetLCCaps(WPARAM wParam,LPARAM lParam);
INT_PTR GetLCName(WPARAM wParam,LPARAM lParam);
INT_PTR LoadLCIcon(WPARAM wParam,LPARAM lParam); 
int     SetLCStatus(WPARAM wParam,LPARAM lParam);
INT_PTR GetLCStatus(WPARAM wParam,LPARAM lParam);

// dialog.c
INT_PTR addContact(WPARAM wParam,LPARAM lParam) ;
INT_PTR editContact(WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK DlgProcNimcOpts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR testStringReplacer(WPARAM wParam, LPARAM lParam);
INT_PTR LoadFilesDlg(WPARAM wParam, LPARAM lParam);


// files.c
int Openfile(char* outputFile, int saveOpen);
INT_PTR CALLBACK DlgProcFiles(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int savehtml(char* outFile);
char* getMimDir(char* file);
INT_PTR exportContacts(WPARAM wParam,LPARAM lParam) ;

// contactinfo.c
INT_PTR CALLBACK DlgProcContactInfo(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOtherStuff(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcCopy(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ExportContact(MCONTACT hContact);
INT_PTR ImportContacts(WPARAM wParam, LPARAM lParam);

// stringreplacer.c
int stringReplacer(const char *oldString, CMStringA &szNewString, MCONTACT hContact);
void replaceAllStrings(MCONTACT hContact);
void WriteSetting(MCONTACT hContact, char* module1, char* setting1 , char* module12, char* setting2);

//timer.c
void CALLBACK timerProc();
int startTimer(int interval);
int killTimer();

// http.c
void NetlibInit();
int InternetDownloadFile (CHAR *szUrl);
extern char *szInfo;
extern char *szData;
extern HANDLE hNetlibUser;

int db_get_static(MCONTACT hContact, const char *moduleName, const char *fn, char *buf, size_t cbSize);

#endif

#pragma comment(lib,"comctl32.lib")
