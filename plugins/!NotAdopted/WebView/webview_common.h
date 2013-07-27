/* __try/__finally taken from abiword patch found on the web */
#if 0
 #include <crtdbg.h>
#else
#define __try
#define __except(x) if (0) /* don't execute handler */
#define __finally

#define _try __try
#define _except __except
#define _finally __finally
#endif



#include <windows.h>
#include <process.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <time.h>
#include <locale.h>
#include <wchar.h>
#include <tchar.h>
#include <Richedit.h>
#include <Commctrl.h>
#include <dirent.h> 
#include "../include/newpluginapi.h"
#include "../include/m_system.h"
//#include "../include/m_netlib.h"
#include "../include/m_clist.h"
#include "../include/m_clui.h"
#include "../include/m_database.h"
#include "../include/m_protomod.h"
#include "../include/m_protocols.h"
#include "../include/m_protosvc.h"
#include "../include/m_button.h"
#include "../include/m_utils.h"
#include "../include/m_skin.h"
#include "../include/m_genmenu.h"
#include "../include/m_icolib.h"



#define MIRANDA_CUSTOM_LP





#include "../include/m_netlib.h"
#include "../include/m_langpack.h"
#include "../include/m_findadd.h"

// leave here as reference
/*
#define UNICODE_VERISON
#define UNICODE
#define _UNICODE
*/
#include "resource.h"


//extern unsigned lpcp;



#define CFM_BACKCOLOR   0x04000000

#define MENU_OFF                "DisableMenu"
#define REFRESH_KEY             "Refresh interval"
#define ON_TOP_KEY              "Always on top"
#define OFFLINE_STATUS          "OfflineOnBoot"
#define URL_KEY                 "URL"
#define START_STRING_KEY        "Start_string"
#define END_STRING_KEY          "End_String"
#define DBLE_WIN_KEY            "Doub_click_win"
#define HIDE_STATUS_ICON_KEY    "Hide_Status_Icon"
#define HAS_CRASHED_KEY         "Has_Crashed"
#define U_ALLSITE_KEY           "UseAllTheWebsite"
#define MENU_IS_DISABLED_KEY    "MainMenuReallyDisabled"
#define UPDATE_ONALERT_KEY      "WND_UPDATE_OALERY_ONLY"
#define DISABLE_AUTOUPDATE_KEY  "Disable_Auto_Update"
#define RWSPACE_KEY             "level_of_wspace_removal"
#define PRESERVE_NAME_KEY       "PreserveName"
#define CLEAR_DISPLAY_KEY       "Remove_tags_whitespace"
#define ALRT_S_STRING_KEY       "ALRT_S_STRING"
#define ALRT_E_STRING_KEY       "ALRT_E_STRING"
#define ALRT_INDEX_KEY          "AlertIndex"
#define EVNT_INDEX_KEY          "EventIndex"
#define START_DELAY_KEY         "StartUpDelay"
#define ALWAYS_LOG_KEY          "AlwaysLogToFile"
#define SAVE_INDIVID_POS_KEY    "SaveIndividWinPos"
#define NO_PROTECT_KEY          "NoDownloadProtection"
#define SAVE_AS_RAW_KEY         "SaveAsRaw"
#define FONT_SCRIPT_KEY         "FontScript"
#define STOP_KEY                "StopProcessing"
#define DATA_POPUP_KEY          "DisplayDataPopup"
#define COUNTDOWN_KEY           "Countdown"

#define MINUTE                   60000
#define SECOND                   1000
#define MONITOR_DEFAULTTONEAREST 2

#define MS_UPDATE_ALL           "Webview/UpdateAll"
#define MS_ADD_SITE             "Webview/AddSite"
#define MS_AUTO_UPDATE          "Webview/AutoUpdate"

#define CACHE_FILE_KEY          "Filename"


/*
 * some globals for window settings
 */
DWORD           Xposition;
DWORD           Yposition;
DWORD           BackgoundClr;
DWORD           TextClr;
DWORD           WindowHeight;
DWORD           WindowWidth;

UINT_PTR        timerId;
UINT_PTR        Countdown;
LOGFONT         lf;
HFONT           h_font;
HWND            ContactHwnd;
HINSTANCE       hInst;
HMENU           hMenu;
char           *DLLNAME;
int             bpStatus;
CLISTMENUITEM   mi;
HANDLE          hMenuItem1;
HANDLE          hMenuItemCountdown;
char            optionsname[80];

struct FORK_ARG
{
   HANDLE          hEvent;
   void            (__cdecl * threadcode) (void *);
   unsigned        (__stdcall * threadcodeex) (void *);
   void           *arg;
};

//lets get rid of some warnings
void            timerfunc(void);
void            Countdownfunc(void);
void            SavewinSettings(void);
void            ValidatePosition(HWND hwndDlg);
void            CheckDbKeys(void);
void __cdecl    forkthread_r(void *param);
unsigned long   forkthread(void (__cdecl * threadcode) (void *), unsigned long stacksize, void *arg);
int             WebsiteMenuCommand(WPARAM wParam, LPARAM lParam);
int             UpdateMenuCommand(WPARAM wParam, LPARAM lParam, HANDLE singlecontact);
int             OnTopMenuCommand(WPARAM wParam, LPARAM lParam, HANDLE singlecontact);
int             AddContactMenuCommand(WPARAM wParam, LPARAM lParam);
int             ContactOptionsMenuCommand(WPARAM wParam, LPARAM lParam);
int             CntOptionsMenuCommand(WPARAM wParam, LPARAM lParam);
int             CntAlertMenuCommand(WPARAM wParam, LPARAM lParam);
int             ModulesLoaded(WPARAM wParam, LPARAM lParam);
void            ChangeMenuItem3();
int             ContactMenuItemUpdateData (WPARAM wParam, LPARAM lParam);
int             MarkAllReadMenuCommand(WPARAM wParam, LPARAM lParam);


//
int             ShowHideContactCommand(WPARAM wParam, LPARAM lParam);

//
int             Doubleclick(WPARAM wParam, LPARAM lParam);
int             DataWndMenuCommand(WPARAM wParam, LPARAM lParam);
//int             SendToRichEdit(HWND hWindow, char *truncated, COLORREF rgbText, COLORREF rgbBack);
int             SendToRichEdit(HWND hWindow, TCHAR *truncated, COLORREF rgbText, COLORREF rgbBack);
int             UpdateAllMenuCommand(WPARAM wParam, LPARAM lParam);
int             CountdownMenuCommand(WPARAM wParam, LPARAM lParam);

//Services
int             GetCaps(WPARAM wParam, LPARAM lParam);
int             GetName(WPARAM wParam, LPARAM lParam);
int             BPLoadIcon(WPARAM wParam, LPARAM lParam); // BPLoadIcon

                                                          // because LoadIcon 
                                                          // wont work..

int             SetStatus(WPARAM wParam, LPARAM lParam);
int             GetStatus(WPARAM wParam, LPARAM lParam);

//
void            ChangeContactStatus(int con_stat);
void            InitialiseGlobals(void);
void            FontSettings(void);

///

void            Removewhitespace(char *truncated);
void            RemoveInvis(char *truncated, int AmountWspcRem);
void            RemoveTabs(char *truncated);
void            FastTagFilter(char *truncated);
void            EraseBlock(char *truncated);
void            EraseSymbols(char *truncated);
int             ProcessAlerts(HANDLE AContact, char *truncated, char *tstr, char *contactname, int notpresent);

BOOL CALLBACK   DlgPopUpOpts(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK   DlgProcFind(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int             DataDialogResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL * urc);

void            Filter(char *truncated);
void            TxtclrLoop();
void            BGclrLoop();
void            ContactLoop(void *dummy);
void            NumSymbols(char *truncated);

int             AutoUpdateMCmd(WPARAM wParam, LPARAM lParam);
BOOL CALLBACK   DlgProcContactOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void            AckFunc(void *dummy);
int             SiteDeleted(WPARAM wParam, LPARAM lParam);



int WErrorPopup(char *namecontact, char *textdisplay);
void DestroyServices(void);



//////////////////////
// wrappers
HANDLE          hHookDisplayDataAlert;

#define ME_DISPLAYDATA_ALERT	"Miranda/ALERT/DISPLAYDATA"

HANDLE          hHookAlertPopup;

#define ME_POPUP_ALERT	"Miranda/ALERT/POPUP"

HANDLE          hHookAlertWPopup;

#define ME_WPOPUP_ALERT	"Miranda/ALERT/WPOPUP"

HANDLE          hHookErrorPopup;

#define ME_POPUP_ERROR	"Miranda/ERROR/POPUP"

HANDLE          hHookAlertOSD;

#define ME_OSD_ALERT	"Miranda/ALERT/OSD"
////////////

//test
//int             utf8_encode(const char *from, char **to);
