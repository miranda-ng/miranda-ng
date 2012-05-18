/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* This file contains the includes, weather constants/declarations,
   the structs, and the primitives for some of the functions.
*/

//============  THE INCLUDES  ===========
#include <m_stdhdr.h>

#include <stdio.h>
#include <io.h>
#include <share.h>
#include <direct.h>
#include <process.h>
#include <time.h>

#include <windows.h>
#include <commctrl.h>
#include <richedit.h>

#include <win2k.h>

#define MIRANDA_VER 0x0A00

#include <newpluginapi.h>
#include <m_system.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_icolib.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_database.h>
#include <m_history.h>
#include <m_utils.h>
#include <m_userinfo.h>
#include <m_netlib.h>
#include <m_ignore.h>
#include <m_findadd.h>
#include <m_button.h>
#include <m_avatars.h>
#include <m_clui.h>
#include <m_clc.h>
#include <m_fontservice.h>
#include "m_cluiframes.h"

#include <m_popup.h>

#include "m_weather.h"
#include "resource.h"
#include "version.h"

//============  CONSTANTS  ============

// status
#define NOSTATUSDATA	1

// limits
#define MAX_TEXT_SIZE	4096
#define MAX_DATA_LEN	1024

// db info mangement mode
#define WDBM_REMOVE			1
#define WDBM_DETAILDISPLAY	2

// more info list column width
#define LIST_COLUMN		150

// others
#define NODATA			Translate("N/A")
#define UM_SETCONTACT	40000

// weather update error codes
#define INVALID_ID_FORMAT	10
#define INVALID_SVC			11
#define	INVALID_ID			12
#define SVC_NOT_FOUND		20
#define	NETLIB_ERROR		30
#define	DATA_EMPTY			40
#define	DOC_NOT_FOUND		42
#define	DOC_TOO_SHORT		43
#define	UNKNOWN_ERROR		99

// weather update error text
#define E10		Translate("Invalid ID format, missing \"/\" (10)")
#define E11		Translate("Invalid service (11)")
#define E12		Translate("Invalid station (12)")
#define E20		Translate("Weather service ini for this station is not found (20)")
#define E30		Translate("Netlib error - check your internet connection (30)")
#define E40		Translate("Empty data is retrieved (40)")
#define E42		Translate("Document not found (42)")
#define E43		Translate("Document too short to contain any weather data (43)")
#define E99		Translate("Unknown error (99)")

// HTTP error... not all translated
// 100 Continue 
// 101 Switching Protocols 
// 200 OK 
// 201 Created 
// 202 Accepted 
// 203 Non-Authoritative Information 
#define E204	Translate("HTTP Error: No content (204)")
// 205 Reset Content 
// 206 Partial Content 
// 300 Multiple Choices 
#define E301	Translate("HTTP Error: Data moved (301)")
// 302 Found 
// 303 See Other 
// 304 Not Modified 
#define E305	Translate("HTTP Error: Use proxy (305)")
// 306 (Unused) 
#define E307	Translate("HTTP Error: Temporary redirect (307)")
#define E400	Translate("HTTP Error: Bad request (400)")
#define E401	Translate("HTTP Error: Unauthorized (401)")
#define E402	Translate("HTTP Error: Payment required (402)")
#define E403	Translate("HTTP Error: Forbidden (403)")
#define E404	Translate("HTTP Error: Not found (404)")
#define E405	Translate("HTTP Error: Method not allowed (405)")
// 406 Not Acceptable 
#define E407	Translate("HTTP Error: Proxy authentication required (407)")
// 408 Request Timeout 
// 409 Conflict 
#define E410	Translate("HTTP Error: Gone (410)")
// 411 Length Required 
// 412 Precondition Failed 
// 413 Request Entity Too Large 
// 414 Request-URI Too Long 
// 415 Unsupported Media Type 
// 416 Requested Range Not Satisfiable 
// 417 Expectation Failed 
#define E500	Translate("HTTP Error: Internal server error (500)")
// 501 Not Implemented 
#define E502	Translate("HTTP Error: Bad gateway (502)")
#define E503	Translate("HTTP Error: Service unavailable (503)")
#define E504	Translate("HTTP Error: Gateway timeout (504)")
// 505 HTTP Version Not Supported 

// defaults constants
#define C_DEFAULT "%n"
#define N_DEFAULT ""
#define B_DEFAULT "Max/Min: %[Max]/%[Min]\r\nCondition: %[Condition]\r\nTemperature: %[Temperature]\r\nPressure: %[Pressure] (%[Pressure Change])\r\nWind: %[Wind Direction], %[Wind Speed]\r\nHumidity: %[Humidity]\r\nVisibility: %[Visibility]\r\nDew Point: %[Dewpoint]\r\nUV Index: %[UV Index]\r\nMoon: %[Moon]\r\nCoordinates: Latitude: %[Latitude] Longitude: %[Longitude]\r\n\r\nSunrise: %[Sunrise]\r\nSunset: %[Sunset]\r\nDay Length: %[Day Length]\r\n\r\n5 Days Forecast:\r\n%[Forecast Day 1]\r\n%[Forecast Day 2]\r\n%[Forecast Day 3]\r\n%[Forecast Day 4]\r\n%[Forecast Day 5]"
#define b_DEFAULT "Weather Condition for %n as of %u"
#define X_DEFAULT N_DEFAULT
#define H_DEFAULT ""
#define E_DEFAULT ""
#define P_DEFAULT "Weather Condition for %n as of %u"
#define p_DEFAULT "Condition: %[Condition]\r\nTemperature: %[Temperature]\r\nPressure: %[Pressure] (%[Pressure Change])\r\nWind: %[Wind Direction], %[Wind Speed]\r\nHumidity: %[Humidity]\r\nVisibility: %[Visibility]\r\nUV Index: %[UV Index]"
#define s_DEFAULT "Temperature: %[Temperature]"


//============  OPTION STRUCT  ============

// option struct
typedef struct {
// main options
	BOOL AutoUpdate;
	BOOL CAutoUpdate;
	BOOL StartupUpdate;
	WORD UpdateTime;
	WORD AvatarSize;
	BOOL NewBrowserWin;
	BOOL NoProtoCondition;
	BOOL UpdateOnlyConditionChanged;
	BOOL RemoveOldData;
	BOOL MakeItalic;
// units
	WORD tUnit;
	WORD wUnit;
	WORD vUnit;
	WORD pUnit;
	WORD dUnit;
	WORD eUnit;
	char DegreeSign[4];
	BOOL DoNotAppendUnit;
    BOOL NoFrac;
// texts
	char *cText;
	char *bTitle;
	char *bText;
	char *nText;
	char *eText;
	char *hText;
	char *xText;
	char *sText;
// advanced
	BOOL DisCondIcon;
// popup options
	BOOL UsePopup;
	BOOL UpdatePopup;
	BOOL AlertPopup;
	BOOL PopupOnChange;
	BOOL ShowWarnings;
// popup colors
	BOOL UseWinColors;
	COLORREF BGColour;
	COLORREF TextColour;
// popup actions
	DWORD LeftClickAction;
	DWORD RightClickAction;
// popup delay
	DWORD pDelay;
// popup texts
	char *pTitle;
	char *pText;
// other misc stuff
	char Default[64];
	HANDLE DefStn;
} MYOPTIONS;

void DestroyOptions(void);

//============  STRUCT USED TO MAKE AN UPDATE LIST  ============

struct WCONTACTLIST {
	HANDLE hContact;
	struct WCONTACTLIST *next;
};

typedef struct WCONTACTLIST UPDATELIST;

extern UPDATELIST *UpdateListHead;
extern UPDATELIST *UpdateListTail;

void DestroyUpdateList(void);

//============  DATA FORMAT STRUCT  ============

#define WID_NORMAL	0
#define WID_SET		1
#define WID_BREAK	2

typedef struct {
	char *Name;
	char *Start;
	char *End;
	char *Unit;
	char *Url;
	char *Break;
	int Type;
} WIDATAITEM;

struct WITEMLIST {
	WIDATAITEM Item;
	struct WITEMLIST *Next;
};

typedef struct WITEMLIST WIDATAITEMLIST;

typedef struct {
	BOOL Available;
	char *SearchURL;
	char *NotFoundStr;
	WIDATAITEM Name;
} WIIDSEARCH;

typedef struct {
	BOOL Available;
	char *First;
	WIDATAITEM Name;
	WIDATAITEM ID;
} WINAMESEARCHTYPE;

typedef struct {
	char *SearchURL;
	char *NotFoundStr;
	char *SingleStr;
	WINAMESEARCHTYPE Single;
	WINAMESEARCHTYPE Multiple;
} WINAMESEARCH;

struct STRLIST {
	char *Item;
	struct STRLIST *Next;
};

typedef struct STRLIST WICONDITEM;

typedef struct {
	WICONDITEM *Head;
	WICONDITEM *Tail;
} WICONDLIST;

typedef struct {
	char *FileName;
	char *ShortFileName;
	BOOL Enabled;
// header
	char *DisplayName;
	char *InternalName;
	char *Description;
	char *Author;
	char *Version;
	int InternalVer;
	size_t MemUsed;
// default
	char *DefaultURL;
	char *DefaultMap;
	char *UpdateURL;
	char *UpdateURL2;
	char *UpdateURL3;
	char *UpdateURL4;
	char *Cookie;
// items
	int UpdateDataCount;
	WIDATAITEMLIST *UpdateData;
	WIDATAITEMLIST *UpdateDataTail;
	WIIDSEARCH IDSearch;
	WINAMESEARCH NameSearch;
	WICONDLIST CondList[10];
} WIDATA;

//============  DATA LIST (LINKED LIST)  ============

struct DATALIST {
	WIDATA Data;
	struct DATALIST *next;
};

typedef struct DATALIST WIDATALIST;

//============  GLOBAL VARIABLES  ============

extern WIDATALIST *WIHead;
extern WIDATALIST *WITail;

extern HINSTANCE hInst;
extern HWND hPopupWindow;
extern HWND hWndSetup;

extern MYOPTIONS opt;

extern unsigned status;
extern unsigned old_status;

extern HANDLE hDataWindowList;
extern HANDLE hNetlibUser, hNetlibHttp;
extern HANDLE hHookWeatherUpdated;
extern HANDLE hHookWeatherError;
extern HANDLE hWindowList;
extern HANDLE hMwinMenu;

extern UINT_PTR timerId;

// check if weather is currently updating
extern BOOL ThreadRunning;

//============  FUNCTION PRIMITIVES  ============

// functions in weather.c
void UpgradeContact(DWORD lastver, HANDLE hContact);

// functions in weather_addstn.c
INT_PTR WeatherAddToList(WPARAM wParam,LPARAM lParam);
BOOL CheckSearch();

int IDSearch(char *id, const int searchId);
int NameSearch(char *name, const int searchId);

INT_PTR WeatherBasicSearch(WPARAM wParam,LPARAM lParam);
INT_PTR WeatherCreateAdvancedSearchUI(WPARAM wParam, LPARAM lParam);
INT_PTR WeatherAdvancedSearch(WPARAM wParam, LPARAM lParam);

int WeatherAdd(WPARAM wParam, LPARAM lParam);

// functions used in weather_contacts.c
INT_PTR ViewLog(WPARAM wParam,LPARAM lParam);
INT_PTR LoadForecast(WPARAM wParam,LPARAM lParam);
INT_PTR WeatherMap(WPARAM wParam,LPARAM lParam);

INT_PTR EditSettings(WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK DlgProcChange(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int ContactDeleted(WPARAM wParam,LPARAM lParam);

BOOL IsMyContact(HANDLE hContact);

// functions in weather_conv.c
BOOL is_number(char *s);

extern unsigned lpcp;
void InitUniConv(void);
LPWSTR ConvToUnicode(LPCSTR str2);
UINT GetDlgItemTextWth(HWND hDlg, int nIDDlgItem, LPSTR lpString, int nMaxCount);
BOOL SetDlgItemTextWth(HWND hDlg, int nIDDlgItem, LPCSTR lpString);
BOOL SetWindowTextWth(HWND hWnd, LPCSTR lpString);
void ListView_SetItemTextWth(HWND hwndLV, int i, int iSubItem_, LPSTR pszText_);
int ListView_InsertItemWth(HWND hwnd, LV_ITEM *pitem);
int ListView_InsertColumnWth(HWND hwnd, int iCol, LV_COLUMN *pitem);

void GetTemp(char *tempchar, char *unit, char *str);
void GetSpeed(char *tempchar, char *unit, char *str);
void GetPressure(char *tempchar, char *unit, char *str);
void GetDist(char *tempchar, char *unit, char *str);
void GetElev(char *tempchar, char *unit, char *str);

WORD GetIcon(const char* cond, WIDATA *Data);
void CaseConv(char *str);
void TrimString(char *str);
void ConvertBackslashes(char *str);
char *GetSearchStr(char *dis);

char *GetDisplay(WEATHERINFO *w, const char *dis, char* str);
INT_PTR GetDisplaySvcFunc(WPARAM wParam, LPARAM lParam);

void GetSvc(char *pszID);
void GetID(char *pszID);

char *GetError(int code);

// functions in weather_data.c
void GetStationID(HANDLE hContact, char* id, size_t idlen);
WEATHERINFO LoadWeatherInfo(HANDLE Change);
int DBGetData(HANDLE hContact, char *setting, DBVARIANT *dbv);
int DBGetStaticString(HANDLE hContact, const char *szModule, const char *valueName, char *dest, size_t dest_len);

void EraseAllInfo(DWORD lastver);

void LoadStationData(char *pszFile, char *pszShortFile, WIDATA *Data);
void GetDataValue(WIDATAITEM *UpdateData, char *Data, char** szInfo);
void ConvertDataValue(WIDATAITEM *UpdateData, char *Data);
void wSetData(char **Data, const char *Value);
void wfree(char **Data);

void DBDataManage(HANDLE hContact, WORD Mode, WPARAM wParam, LPARAM lParam);
int GetWeatherDataFromDB(const char *szSetting, LPARAM lparam);

// functions in weather_http.c
int InternetDownloadFile (char *szUrl, char *cookie, char** szData);
void NetlibInit();
void NetlibHttpDisconnect(void); 

// functions in weather_ini.c
void WIListAdd(WIDATA Data);
WIDATA* GetWIData(char *pszServ);

BOOL IsContainedInCondList(const char *pszStr, WICONDLIST *List);

void DestroyWIList();
BOOL LoadWIData(BOOL dial);
void FreeWIData(WIDATA *Data);

INT_PTR CALLBACK DlgProcSetup(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// functions in weather_info.c
void GetINIInfo(char *pszSvc);

void MoreVarList();

// functions in weather_opt.c
void SetTextDefault(const char* in);
void LoadOptions();
void SaveOptions();

int OptInit(WPARAM wParam,LPARAM lParam);

INT_PTR CALLBACK OptionsProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam);
void SetIconDefault();
void RemoveIconSettings();

BOOL CALLBACK TextOptionsProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam);
BOOL CALLBACK AdvOptionsProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam);
INT_PTR CALLBACK DlgProcText(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgPopUpOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// functions in weather_popup.c
int WeatherPopup(WPARAM wParam, LPARAM lParam);
int WeatherError(WPARAM wParam, LPARAM lParam);
int WPShowMessage(char* lpzText, WORD kind);

LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void SelectMenuItem(HMENU hMenu, int Check);

// functions in weather_svcs.c
void InitServices(void);
void DestroyServices(void);

INT_PTR WeatherSetStatus(WPARAM new_status, LPARAM lParam);
INT_PTR WeatherGetCaps(WPARAM wParam, LPARAM lParam);
INT_PTR WeatherGetName(WPARAM wParam, LPARAM lParam);
INT_PTR WeatherGetStatus(WPARAM wParam, LPARAM lParam);
INT_PTR WeatherLoadIcon(WPARAM wParam, LPARAM lParam);

void UpdateMenu(BOOL State);
void UpdatePopupMenu(BOOL State);
void AddMenuItems();
void AvatarDownloaded(HANDLE hContact);

// functions in weather_update.c
int UpdateWeather(HANDLE hContact);

int RetrieveWeather(HANDLE hContact, WEATHERINFO *winfo);

void UpdateAll(BOOL AutoUpdate, BOOL RemoveOld);
void UpdateThreadProc(LPVOID hWnd);
INT_PTR UpdateSingleStation(WPARAM wParam,LPARAM lParam);
INT_PTR UpdateAllInfo(WPARAM wParam,LPARAM lParam);
INT_PTR UpdateSingleRemove(WPARAM wParam,LPARAM lParam);
INT_PTR UpdateAllRemove(WPARAM wParam,LPARAM lParam);

int GetWeatherData(HANDLE hContact);

void CALLBACK timerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void CALLBACK timerProc2(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

// function from multiwin module
void InitMwin(void);
void DestroyMwin(void);
INT_PTR Mwin_MenuClicked(WPARAM wParam, LPARAM lParam); 
int BuildContactMenu(WPARAM wparam, LPARAM lparam); 
void UpdateMwinData(HANDLE hContact); 
void removeWindow(HANDLE hContact);

// functions in weather_userinfo.c
int UserInfoInit(WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcUIPage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcMoreData(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define WM_UPDATEDATA WM_USER + 2687

int BriefInfo(WPARAM wParam, LPARAM lParam);
INT_PTR BriefInfoSvc(WPARAM wParam, LPARAM lParam);
void LoadBriefInfoText(HWND hwndDlg, HANDLE hContact);
INT_PTR CALLBACK DlgProcBrief(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void InitIcons(void);
HICON  LoadIconEx(const char* name, BOOL big);
HANDLE GetIconHandle(const char* name);
void   ReleaseIconEx(HICON hIcon);
