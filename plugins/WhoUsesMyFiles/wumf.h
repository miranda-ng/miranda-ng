#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>
#include <lm.h>
#include <locale.h> 

#include "newpluginapi.h"
#include "m_system.h"
#include "m_options.h"
#include "m_langpack.h"
#include "m_clui.h"
#include "m_clist.h"
#include "m_database.h"
#include "m_utils.h"
#include "m_skin.h"
#include "m_popup.h"

#include "m_toptoolbar.h"

#include "resource.h"

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

typedef struct
{
	BOOL 		PopupsEnabled;

	BOOL		UseWinColor;
	BOOL		UseDefColor;
	BOOL		SelectColor;
	COLORREF 	ColorText;
	COLORREF 	ColorBack;
	BOOL        DelayInf;
	BOOL        DelayDef;
	BOOL        DelaySet;
	int         DelaySec;

	BOOL 		LogToFile;
	BOOL 		LogFolders;
	BOOL		AlertFolders;
	BOOL		LogUNC;
	BOOL		AlertUNC;
	BOOL		LogComp;
	BOOL		AlertComp;

	char		LogFile[255];
} WUMF_OPTIONS;

typedef struct _WUMF{
	DWORD dwID;
	LPSTR szID;
	LPSTR szUser;
	LPSTR szPath;
	LPSTR szComp;
	LPSTR szUNC;
	LPSTR szPerm;
	DWORD dwSess; 
	DWORD dwLocks; 
	DWORD dwAttr;
	DWORD dwPerm;
	BOOL  mark;
	struct _WUMF* next;
} Wumf, *PWumf;

PWumf new_wumf(
	DWORD dwID, 
	LPSTR szUser, 
	LPSTR szPath, 
	LPSTR szComp, 
	LPSTR szUNC, 
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



void FreeAll();
VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
int CALLBACK ConnDlgProc(HWND, UINT, WPARAM, LPARAM);
int ResizeDialog(WPARAM wParam,LPARAM lParam);

void ShowThePopUp(PWumf w, LPSTR, LPSTR);
void ShowWumfPopUp(PWumf w);

void process_session(SESSION_INFO_1 s_info);
void process_file(SESSION_INFO_1 s_info, FILE_INFO_3 f_info);
void printError(DWORD res);

#define msg(X) MessageBox(NULL, X, "WUMF", MB_OK|MB_ICONSTOP)
#define MS_WUMF_SWITCHPOPUP 	"WUMF/SwitchPopup"
#define MS_WUMF_CONNECTIONSSHOW "WUMF/ShowConnections"

#define malloc(size) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size)
#define free(something) HeapFree(GetProcessHeap(), 0, something)

#ifndef RD_ANCHORX_CUSTOM
#define RD_ANCHORX_CUSTOM   0	//function did everything required to the x axis, do no more processing
#define RD_ANCHORX_LEFT     0	//move the control to keep it constant distance from the left edge of the dialog
#define RD_ANCHORX_RIGHT    1	//move the control to keep it constant distance from the right edge of the dialog
#define RD_ANCHORX_WIDTH    2	//size the control to keep it constant distance from both edges of the dialog
#define RD_ANCHORX_CENTRE   4	//move the control to keep it constant distance from the centre of the dialog
#define RD_ANCHORY_CUSTOM   0
#define RD_ANCHORY_TOP      0
#define RD_ANCHORY_BOTTOM   8
#define RD_ANCHORY_HEIGHT   16
#define RD_ANCHORY_CENTRE   32

typedef struct {
	int cbSize;
	UINT wId;				//control ID
	RECT rcItem;			//original control rectangle, relative to dialog
                            //modify in-place to specify the new position
	SIZE dlgOriginalSize;	//size of dialog client area in template
	SIZE dlgNewSize;		//current size of dialog client area
} UTILRESIZECONTROL;

typedef int (*DIALOGRESIZERPROC)(HWND hwndDlg,LPARAM lParam,UTILRESIZECONTROL *urc);

typedef struct {
	int cbSize;
	HWND hwndDlg;
	HINSTANCE hInstance;	//module containing the dialog template
	LPCTSTR lpTemplate;		//dialog template
	LPARAM lParam;			//caller-defined
	DIALOGRESIZERPROC pfnResizer;
} UTILRESIZEDIALOG;

#endif
