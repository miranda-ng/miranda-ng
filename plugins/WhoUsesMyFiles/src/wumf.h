#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <lm.h>
#include <locale.h>
#include <shlobj.h>

#include <newpluginapi.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_utils.h>
#include <m_popup.h>
#include <win2k.h>

#include <m_toptoolbar.h>

#include "resource.h"
#include "Version.h"

#define MODULENAME		"WUMF Plugin"

#define LIFETIME_MAX 60
#define LIFETIME_MIN 1
#define MAX_PATHNAME 512
#define MAX_USERNAME 512
#define TIME 500

#define POPUPS_ENABLED              "1033"
#define DELAY_SEC	                "1026"
#define DELAY_SET                   "1049"
#define DELAY_INF                   "1050"
#define DELAY_DEF                   "1051"
#define COLOR_SET                   "1000"
#define COLOR_WIN                   "1001"
#define COLOR_DEF                   "1002"
#define COLOR_BACK                  "1003"
#define COLOR_TEXT                  "1004"
#define OPT_FILE                    "1006"
#define LOG_INTO_FILE               "1054"
#define LOG_FOLDER                  "1055"
#define ALERT_FOLDER                "1056"
#define LOG_UNC                     "1057"
#define ALERT_UNC                   "1058"
#define LOG_COMP                    "1059"
#define ALERT_COMP                  "1060"

#define IDM_SETUP 0x0402
#define IDM_ABOUT 0x0403
#define IDM_SHOW  0x0405
#define IDM_EXIT  0x0404

struct WUMF_OPTIONS
{
	BOOL     PopupsEnabled;
	BOOL     UseWinColor;
	BOOL     UseDefColor;
	BOOL     SelectColor;
	BOOL     DelayInf;
	BOOL     DelayDef;
	BOOL     DelaySet;
	int      DelaySec;

	BOOL     LogToFile;
	BOOL     LogFolders;
	BOOL     AlertFolders;
	BOOL     LogUNC;
	BOOL     AlertUNC;
	BOOL     LogComp;
	BOOL     AlertComp;

	COLORREF ColorText;
	COLORREF ColorBack;

	TCHAR    LogFile[255];
};

struct Wumf
{
	DWORD  dwID;
	TCHAR  szID[10], szPerm[10];
	LPTSTR szUser;
	LPTSTR szPath;
	LPTSTR szComp;
	LPTSTR szUNC;
	DWORD  dwSess; 
	DWORD  dwLocks; 
	DWORD  dwAttr;
	DWORD  dwPerm;
	BOOL   mark;
	Wumf  *next;
};

typedef Wumf *PWumf;

PWumf new_wumf(
	DWORD dwID, 
	LPTSTR szUser, 
	LPTSTR szPath, 
	LPTSTR szComp, 
	LPTSTR szUNC, 
	DWORD szSess, 
	DWORD dwPerm, 
	DWORD dwAttr);

BOOL  add_cell  (PWumf* l, PWumf w);
BOOL  del_cell  (PWumf* l, PWumf w);
BOOL  cpy_cell  (PWumf* l, PWumf w);
PWumf fnd_cell  (PWumf* l, DWORD dwID);
PWumf cpy_list  (PWumf* l);
BOOL  del_all   (PWumf* l);
void  mark_all  (PWumf* l, BOOL mark);
BOOL  del_marked(PWumf* l);

extern WUMF_OPTIONS WumfOptions;
extern HINSTANCE hInst;
extern HANDLE hLogger;
static HANDLE hWumfBut;
extern PWumf list;

void FreeAll();
VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
INT_PTR CALLBACK ConnDlgProc(HWND, UINT, WPARAM, LPARAM);

void ShowThePopup(PWumf w, LPTSTR, LPTSTR);
void ShowWumfPopup(PWumf w);

void process_session(SESSION_INFO_1 s_info);
void process_file(SESSION_INFO_1 s_info, FILE_INFO_3 f_info);
void printError(DWORD res);

#define msg(X) MessageBox(NULL, X, _T("WUMF"), MB_OK|MB_ICONSTOP)
#define MS_WUMF_SWITCHPOPUP 	"WUMF/SwitchPopup"
#define MS_WUMF_CONNECTIONSSHOW "WUMF/ShowConnections"
