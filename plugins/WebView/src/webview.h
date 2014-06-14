/*
 * A plugin for Miranda IM which displays web page text in a window Copyright 
 * (C) 2005 Vincent Joyce.
 * 
 * Miranda IM: the free icq client for MS Windows  Copyright (C) 2000-2
 * Richard Hughes, Roland Rabien & Tristan Van de Vreede
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 59 
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 */

#define CFM_BACKCOLOR   0x04000000

#define MODULENAME "WebView"

#define MENU_OFF    "DisableMenu"
#define REFRESH_KEY "Refresh interval"
#define ON_TOP_KEY  "Always on top"
#define OFFLINE_STATUS "OfflineOnBoot"
#define URL_KEY     "URL"
#define START_STRING_KEY        "Start_string"
#define END_STRING_KEY "End_String"
#define DBLE_WIN_KEY "Doub_click_win"
#define HIDE_STATUS_ICON_KEY    "Hide_Status_Icon"
#define HAS_CRASHED_KEY         "Has_Crashed"
#define U_ALLSITE_KEY  "UseAllTheWebsite"
#define MENU_IS_DISABLED_KEY    "MainMenuReallyDisabled"
#define UPDATE_ONALERT_KEY      "WND_UPDATE_OALERY_ONLY"
#define DISABLE_AUTOUPDATE_KEY  "Disable_Auto_Update"
#define RWSPACE_KEY "level_of_wspace_removal"
#define PRESERVE_NAME_KEY       "PreserveName"
#define CLEAR_DISPLAY_KEY       "Remove_tags_whitespace"
#define ALRT_S_STRING_KEY       "ALRT_S_STRING"
#define ALRT_E_STRING_KEY       "ALRT_E_STRING"
#define ALRT_INDEX_KEY "AlertIndex"
#define EVNT_INDEX_KEY "EventIndex"
#define START_DELAY_KEY         "StartUpDelay"
#define ALWAYS_LOG_KEY "AlwaysLogToFile"
#define SAVE_INDIVID_POS_KEY    "SaveIndividWinPos"
#define NO_PROTECT_KEY "NoDownloadProtection"
#define SAVE_AS_RAW_KEY         "SaveAsRaw"
#define FONT_SCRIPT_KEY         "FontScript"
#define STOP_KEY    "StopProcessing"
#define DATA_POPUP_KEY "DisplayDataPopup"
#define COUNTDOWN_KEY  "Countdown"

#define MINUTE       60000
#define SECOND       1000

#define MS_UPDATE_ALL  "Webview/UpdateAll"
#define MS_ADD_SITE "Webview/AddSite"
#define MS_AUTO_UPDATE "Webview/AutoUpdate"

#define CACHE_FILE_KEY "Filename"

#define Xpos_WIN_KEY     "win_Xpos"
#define Ypos_WIN_KEY     "win_Ypos"
#define BG_COLOR_KEY     "BgColor"
#define TXT_COLOR_KEY    "TxtColor"
#define WIN_HEIGHT_KEY   "Height"
#define SUPPRESS_ERR_KEY "Suppress error messages"
#define WIN_WIDTH_KEY    "Width"
#define FILE_KEY         "Filename"
#define APPEND_KEY       "Append"
#define UPDATE_ONSTART_KEY  "update_onboot"
#define UPDATE_ON_OPEN_KEY  "update_on_Window_open"
#define FONT_FACE_KEY    "FontFace"
#define FONT_BOLD_KEY    "FontBold"
#define FONT_ITALIC_KEY  "FontItalic"
#define FONT_UNDERLINE_KEY  "FontUnderline"
#define FONT_SIZE_KEY    "FontSize"
#define ERROR_POPUP_KEY  "UsePopupPlugin"
#define ENABLE_ALERTS_KEY "EnableAlerts"
#define ALERT_STRING_KEY "ALERTSTRING"
#define ALERT_TYPE_KEY   "Alert_Type"
#define APND_DATE_NAME_KEY  "AppendDateContact"
#define POP_DELAY_KEY    "PopUpDelay"
#define POP_BG_CLR_KEY   "PopBGClr"
#define POP_TXT_CLR_KEY  "PopTxtClr"
#define POP_USEWINCLRS_KEY  "PopUseWinClrs"
#define CONTACT_PREFIX_KEY  "PrefixDateContact"
#define USE_24_HOUR_KEY  "Use24hourformat"
#define POP_USESAMECLRS_KEY "PopUseSameClrs"
#define POP_USECUSTCLRS_KEY "PopUseCustomClrs"
#define LCLK_WINDOW_KEY  "LeftClkWindow"
#define LCLK_WEB_PGE_KEY "LeftClkWebPage"
#define LCLK_DISMISS_KEY "LeftClkDismiss"
#define RCLK_WINDOW_KEY  "RightClkWindow"
#define RCLK_WEB_PGE_KEY "RightClkWebPage"
#define RCLK_DISMISS_KEY "RightClkDismiss"
//#define INBUILTPOP_KEY   "UseInbuiltPopupAlert"

#define TIME  60
#define Def_color_bg     0x00ffffff
#define Def_color_txt    0x00000000
#define Def_font_face    _T("Courier")
#define Def_font_size    14
#define HK_SHOWHIDE      3001

#define MAXSIZE1         250000
#define MAXSIZE2         500000
#define MAXSIZE3         1000000

#define Def_win_height   152
#define Def_win_width    250

//lets get rid of some warnings
void CodetoSymbol(char *truncated);
void GetData(void *param);
void FillFontListThread(void *);

INT_PTR CALLBACK DlgProcDisplayData(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcAlertOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOpt(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int DataWndAlertCommand(WPARAM wParam, LPARAM lParam);
int PopupAlert(WPARAM wParam, LPARAM lParam);
int ErrorMsgs(WPARAM wParam, LPARAM lParam);
int OSDAlert(WPARAM wParam, LPARAM lParam);

void ReadFromFile(void *hContact);

/*
 * some globals for window settings
 */

extern DWORD Xposition, Yposition, BackgoundClr, TextClr, WindowHeight, WindowWidth;
extern UINT_PTR timerId, Countdown;
extern LOGFONT lf;
extern HFONT h_font;
extern HWND ContactHwnd;
extern HINSTANCE hInst;
extern HMENU hMenu;
extern int bpStatus;
extern HANDLE hNetlibUser, hWindowList;
extern HANDLE hMenuItem1, hMenuItemCountdown;
extern char optionsname[80];

//lets get rid of some warnings
void CALLBACK timerfunc(HWND, UINT, UINT_PTR, DWORD);
void CALLBACK Countdownfunc(HWND, UINT, UINT_PTR, DWORD);
void SavewinSettings(void);
void ValidatePosition(HWND hwndDlg);
int  ModulesLoaded(WPARAM wParam, LPARAM lParam);
void ChangeMenuItem3();
TCHAR* FixButtonText(TCHAR *url, size_t len);
int  ContactMenuItemUpdateData (WPARAM wParam, LPARAM lParam);

int Doubleclick(WPARAM wParam, LPARAM lParam);
int DBSettingChanged(WPARAM wParam, LPARAM lParam);

int SendToRichEdit(HWND hWindow, char *truncated, COLORREF rgbText, COLORREF rgbBack);

//Services
INT_PTR GetCaps(WPARAM wParam, LPARAM lParam);
INT_PTR GetName(WPARAM wParam, LPARAM lParam);
INT_PTR BPLoadIcon(WPARAM wParam, LPARAM lParam); // BPLoadIcon

INT_PTR SetStatus(WPARAM wParam, LPARAM lParam);
INT_PTR GetStatus(WPARAM wParam, LPARAM lParam);

INT_PTR BasicSearch(WPARAM wParam, LPARAM lParam);
INT_PTR AddToList(WPARAM wParam, LPARAM lParam);
INT_PTR GetInfo(WPARAM wParam, LPARAM lParam);

INT_PTR OpenCacheDir(WPARAM wParam, LPARAM lParam);

INT_PTR UpdateAllMenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR CountdownMenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR MarkAllReadMenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR WebsiteMenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR AddContactMenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR ContactOptionsMenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR CntOptionsMenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR CntAlertMenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR DataWndMenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR ShowHideContactCommand(WPARAM wParam, LPARAM lParam);
INT_PTR PingWebsiteMenuCommand(WPARAM wParam, LPARAM lParam);
INT_PTR StpPrcssMenuCommand(WPARAM wParam, LPARAM lParam);

int  UpdateMenuCommand(WPARAM wParam, LPARAM lParam, MCONTACT singlecontact);
int  OnTopMenuCommand(WPARAM wParam, LPARAM lParam, MCONTACT singlecontact);

//
void ChangeContactStatus(int con_stat);
void InitialiseGlobals(void);
void FontSettings(void);

///

void Removewhitespace(char *truncated);
void RemoveInvis(char *truncated, int AmountWspcRem);
void RemoveTabs(char *truncated);
void FastTagFilter(char *truncated);
void EraseBlock(char *truncated);
void EraseSymbols(char *truncated);
int  ProcessAlerts(MCONTACT hContact, char *truncated, char *tstr, char *contactname, int notpresent);

INT_PTR CALLBACK DlgPopUpOpts(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcFind(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int  DataDialogResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL * urc);

void Filter(char *truncated);
void TxtclrLoop();
void BGclrLoop();
void ContactLoop(void *dummy);
void NumSymbols(char *truncated);

INT_PTR AutoUpdateMCmd(WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcContactOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void AckFunc(void *dummy);
int  SiteDeleted(WPARAM wParam, LPARAM lParam);

int  WErrorPopup(MCONTACT hContact, TCHAR *textdisplay);
int  WAlertPopup(MCONTACT hContact, TCHAR *displaytext);

//////////////////////
// wrappers

extern HANDLE hHookDisplayDataAlert;
#define ME_DISPLAYDATA_ALERT	"Miranda/ALERT/DISPLAYDATA"

extern HANDLE hHookAlertPopup;
#define ME_POPUP_ALERT	"Miranda/ALERT/POPUP"

extern HANDLE hHookErrorPopup;
#define ME_POPUP_ERROR	"Miranda/ERROR/POPUP"

extern HANDLE hHookAlertOSD;
#define ME_OSD_ALERT	"Miranda/ALERT/OSD"
