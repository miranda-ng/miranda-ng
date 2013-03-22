//=====================================================
//	Includes (yea why not include lots of stuff :p )
//=====================================================
#ifndef COMMONHEADERS
#define COMMONHEADERS


#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <string.h>
#include <winsock.h>
#include <shlobj.h>
#include <prsht.h>
#include <pshpack1.h>
#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
typedef struct DLGTEMPLATEEX
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
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;

#include <poppack.h>


#include "resource.h"
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
#include "../miranda_src/SDK/headers_c/m_netlib.h"
//#include "../miranda_src/SDK/Headers_c/m_uninstaller.h"
#include <winbase.h>


//=======================================================
//	Definitions
//=======================================================
#define modname			"NIM_Contact"
#define modFullname		"Non-IM Contact"
#define MAXLINES	10000
#define MAX_STRING_LENGTH	10000
#define LINE_LENGTH 10000
#define msg(a,b)	MessageBox(0,a,b,MB_OK);
#define TIMER	(DBGetContactSettingWord(NULL, modname, "Timer", 1) * 1000)
/* ERROR VALUES */
#define ERROR_NO_LINE_AFTER_VAR_F -1
#define ERROR_LINE_NOT_READ -2
#define ERROR_NO_FILE -3

//=======================================================
//	Defines
//=======================================================
//General
HINSTANCE hInst;
extern int LCStatus;
static HANDLE hWindowList=NULL;

//Services.c
int GetLCCaps(WPARAM wParam,LPARAM lParam);
int GetLCName(WPARAM wParam,LPARAM lParam);
int LoadLCIcon(WPARAM wParam,LPARAM lParam); 
int SetLCStatus(WPARAM wParam,LPARAM lParam);
int GetLCStatus(WPARAM wParam,LPARAM lParam);
int DBGetContactSettingString(HANDLE hContact, char* szModule, char* szSetting, char* value);

// dialog.c
int addContact(WPARAM wParam,LPARAM lParam) ;
int editContact(WPARAM wParam,LPARAM lParam);
BOOL CALLBACK DlgProcNimcOpts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int testStringReplacer(WPARAM wParam, LPARAM lParam);
int LoadFilesDlg(WPARAM wParam, LPARAM lParam);


// files.c
int Openfile(char * outputFile, int saveOpen);
BOOL CALLBACK DlgProcFiles(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int savehtml(char* outFile);
char* getMimDir(char* file);
int exportContacts(WPARAM wParam,LPARAM lParam) ;

// contactinfo.c
BOOL CALLBACK DlgProcContactInfo(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcOtherStuff(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcCopy(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ExportContact(HANDLE hContact);
int ImportContacts(WPARAM wParam, LPARAM lParam);

// stringreplacer.c
int stringReplacer(const char* oldString, char* newString, HANDLE hContact);
void replaceAllStrings(HANDLE hContact);
void WriteSetting(HANDLE hContact, char* module1, char* setting1 , char* module12, char* setting2);

//timer.c
UINT_PTR timerId;
void CALLBACK timerProc();
int startTimer(int interval);
int killTimer();

// http.c
void NetlibInit();
int InternetDownloadFile (CHAR *szUrl);
char *szInfo;
char *szData;
HANDLE hNetlibUser;
// threads.c
unsigned long forkthread (   void (__cdecl *threadcode)(void*),unsigned long stacksize,void *arg) ;


#endif

#ifndef NDEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#pragma comment(lib,"comctl32.lib")
