/*
Weather Protocol plugin for Miranda NG
Copyright (C) 2012-22 Miranda NG team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

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

#pragma once

//============  THE INCLUDES  ===========

#include <share.h>
#include <time.h>
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <m_acc.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_clc.h>
#include <m_cluiframes.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_findadd.h>
#include <m_fontservice.h>
#include <m_history.h>
#include <m_icolib.h>
#include <m_ignore.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_skin_eng.h>
#include <m_userinfo.h>
#include <m_xstatus.h>

#include <m_tipper.h>
#include <m_weather.h>
#include <m_toptoolbar.h>

#include "resource.h"
#include "version.h"

//============  CONSTANTS  ============

// name
#define MODULENAME         "Weather"
#define WEATHERPROTOTEXT   "Weather"
#define DEFCURRENTWEATHER  "WeatherCondition"
#define WEATHERCONDITION   "Current"

// weather conditions
enum EWeatherCondition
{
	SUNNY,
	NA,
	PCLOUDY,
	CLOUDY,
	RAIN,
	RSHOWER,
	FOG,
	SNOW,
	SSHOWER,
	LIGHT,
	MAX_COND
};

// status
#define NOSTATUSDATA	1

// limits
#define MAX_TEXT_SIZE   4096
#define MAX_DATA_LEN    1024

// db info mangement mode
#define WDBM_REMOVE			1
#define WDBM_DETAILDISPLAY	2

// more info list column width
#define LIST_COLUMN		 150

// others
#define NODATA			TranslateT("N/A")
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
#define E10		TranslateT("Invalid ID format, missing \"/\" (10)")
#define E11		TranslateT("Invalid service (11)")
#define E12		TranslateT("Invalid station (12)")
#define E20		TranslateT("Weather service ini for this station is not found (20)")
#define E30		TranslateT("Netlib error - check your internet connection (30)")
#define E40		TranslateT("Empty data is retrieved (40)")
#define E42		TranslateT("Document not found (42)")
#define E43		TranslateT("Document too short to contain any weather data (43)")
#define E99		TranslateT("Unknown error (99)")

// HTTP error... not all translated
// 100 Continue
// 101 Switching Protocols
// 200 OK
// 201 Created
// 202 Accepted
// 203 Non-Authoritative Information
#define E204	TranslateT("HTTP Error: No content (204)")
// 205 Reset Content
// 206 Partial Content
// 300 Multiple Choices
#define E301	TranslateT("HTTP Error: Data moved (301)")
// 302 Found
// 303 See Other
// 304 Not Modified
#define E305	TranslateT("HTTP Error: Use proxy (305)")
// 306 (Unused)
#define E307	TranslateT("HTTP Error: Temporary redirect (307)")
#define E400	TranslateT("HTTP Error: Bad request (400)")
#define E401	TranslateT("HTTP Error: Unauthorized (401)")
#define E402	TranslateT("HTTP Error: Payment required (402)")
#define E403	TranslateT("HTTP Error: Forbidden (403)")
#define E404	TranslateT("HTTP Error: Not found (404)")
#define E405	TranslateT("HTTP Error: Method not allowed (405)")
// 406 Not Acceptable
#define E407	TranslateT("HTTP Error: Proxy authentication required (407)")
// 408 Request Timeout
// 409 Conflict
#define E410	TranslateT("HTTP Error: Gone (410)")
// 411 Length Required
// 412 Precondition Failed
// 413 Request Entity Too Large
// 414 Request-URI Too Long
// 415 Unsupported Media Type
// 416 Requested Range Not Satisfiable
// 417 Expectation Failed
#define E500	TranslateT("HTTP Error: Internal server error (500)")
// 501 Not Implemented
#define E502	TranslateT("HTTP Error: Bad gateway (502)")
#define E503	TranslateT("HTTP Error: Service unavailable (503)")
#define E504	TranslateT("HTTP Error: Gateway timeout (504)")
// 505 HTTP Version Not Supported

// defaults constants
#define VAR_LIST_OPT TranslateT("%c\tcurrent condition\n%d\tcurrent date\n%e\tdewpoint\n%f\tfeel-like temp\n%h\ttoday's high\n%i\twind direction\n%l\ttoday's low\n%m\thumidity\n%n\tstation name\n%p\tpressure\n%r\tsunrise time\n%s\tstation ID\n%t\ttemperature\n%u\tupdate time\n%v\tvisibility\n%w\twind speed\n%y\tsun set\n----------\n\\n\tnew line")

//============  OPTION STRUCT  ============

// option struct
struct MYOPTIONS
{
	// main options
	uint8_t AutoUpdate;
	uint8_t CAutoUpdate;
	uint8_t StartupUpdate;
	uint8_t NoProtoCondition;
	uint8_t UpdateOnlyConditionChanged;
	uint8_t RemoveOldData;
	uint8_t MakeItalic;

	uint16_t UpdateTime;
	uint16_t AvatarSize;

	// units
	uint16_t tUnit;
	uint16_t wUnit;
	uint16_t vUnit;
	uint16_t pUnit;
	uint16_t dUnit;
	uint16_t eUnit;
	wchar_t DegreeSign[4];
	uint8_t DoNotAppendUnit;
	uint8_t NoFrac;

	// advanced
	uint8_t DisCondIcon;

	// popup options
	uint8_t UpdatePopup;
	uint8_t AlertPopup;
	uint8_t PopupOnChange;
	uint8_t ShowWarnings;

	// popup colors
	uint8_t UseWinColors;
	COLORREF BGColour;
	COLORREF TextColour;

	// popup actions
	uint32_t LeftClickAction;
	uint32_t RightClickAction;

	// popup delay
	uint32_t pDelay;

	// other misc stuff
	wchar_t Default[64];
	MCONTACT DefStn;
};

//============  STRUCT USED TO MAKE AN UPDATE LIST  ============
struct WCONTACTLIST {
	MCONTACT hContact;
	struct WCONTACTLIST *next;
};

typedef struct WCONTACTLIST UPDATELIST;

extern UPDATELIST *UpdateListHead, *UpdateListTail;

void DestroyUpdateList(void);

//============  DATA FORMAT STRUCT  ============

#define WID_NORMAL	0
#define WID_SET		1
#define WID_BREAK	2

struct WIDATAITEM
{
	wchar_t *Name;
	wchar_t *Start;
	wchar_t *End;
	wchar_t *Unit;
	char    *Url;
	wchar_t *Break;
	int      Type;
};

struct WITEMLIST
{
	WIDATAITEM Item;
	struct WITEMLIST *Next;
};

typedef struct WITEMLIST WIDATAITEMLIST;

struct WIIDSEARCH
{
	BOOL Available;
	char *SearchURL;
	wchar_t *NotFoundStr;
	WIDATAITEM Name;
};

struct WINAMESEARCHTYPE
{
	BOOL Available;
	wchar_t *First;
	WIDATAITEM Name;
	WIDATAITEM ID;
};

struct WINAMESEARCH
{
	char *SearchURL;
	wchar_t *NotFoundStr;
	wchar_t *SingleStr;
	WINAMESEARCHTYPE Single;
	WINAMESEARCHTYPE Multiple;
};

struct STRLIST
{
	wchar_t *Item;
	struct STRLIST *Next;
};

typedef struct STRLIST WICONDITEM;

struct WICONDLIST
{
	WICONDITEM *Head;
	WICONDITEM *Tail;
};

struct WIDATA
{
	wchar_t *FileName;
	wchar_t *ShortFileName;
	BOOL Enabled;

	// header
	wchar_t *DisplayName;
	wchar_t *InternalName;
	wchar_t *Description;
	wchar_t *Author;
	wchar_t *Version;
	int InternalVer;
	size_t MemUsed;

	// default
	char  *DefaultURL;
	wchar_t *DefaultMap;
	char  *UpdateURL;
	char  *UpdateURL2;
	char  *UpdateURL3;
	char  *UpdateURL4;
	char  *Cookie;
	char  *UserAgent;

	// items
	int UpdateDataCount;
	WIDATAITEMLIST *UpdateData;
	WIDATAITEMLIST *UpdateDataTail;
	WIIDSEARCH IDSearch;
	WINAMESEARCH NameSearch;
	WICONDLIST CondList[MAX_COND];
};

//============  DATA LIST (LINKED LIST)  ============

struct DATALIST
{
	WIDATA Data;
	struct DATALIST *next;
};

typedef struct DATALIST WIDATALIST;

//============  GLOBAL VARIABLES  ============

extern WIDATALIST *WIHead, *WITail;

extern HWND hPopupWindow, hWndSetup;

extern MYOPTIONS opt;

extern unsigned status, old_status;

extern MWindowList hDataWindowList, hWindowList;

extern HNETLIBUSER hNetlibUser;
extern HANDLE hHookWeatherUpdated, hHookWeatherError, hTBButton, hUpdateMutex;
extern UINT_PTR timerId;

extern HGENMENU hMwinMenu;

// check if weather is currently updating
extern BOOL ThreadRunning;
extern bool g_bIsUtf;

//============  FUNCTION PRIMITIVES  ============

// functions in weather_addstn.c
INT_PTR WeatherAddToList(WPARAM wParam,LPARAM lParam);
BOOL CheckSearch();

int IDSearch(wchar_t *id, const int searchId);
int NameSearch(wchar_t *name, const int searchId);

INT_PTR WeatherBasicSearch(WPARAM wParam,LPARAM lParam);
INT_PTR WeatherCreateAdvancedSearchUI(WPARAM wParam, LPARAM lParam);
INT_PTR WeatherAdvancedSearch(WPARAM wParam, LPARAM lParam);

int WeatherAdd(WPARAM wParam, LPARAM lParam);

// functions used in weather_contacts.c
INT_PTR ViewLog(WPARAM wParam,LPARAM lParam);
INT_PTR LoadForecast(WPARAM wParam,LPARAM lParam);
INT_PTR WeatherMap(WPARAM wParam,LPARAM lParam);
INT_PTR EditSettings(WPARAM wParam,LPARAM lParam);

int ContactDeleted(WPARAM wParam,LPARAM lParam);

BOOL IsMyContact(MCONTACT hContact);

// functions in weather_conv.c
void GetTemp(wchar_t *tempchar, wchar_t *unit, wchar_t *str);
void GetSpeed(wchar_t *tempchar, wchar_t *unit, wchar_t *str);
void GetPressure(wchar_t *tempchar, wchar_t *unit, wchar_t *str);
void GetDist(wchar_t *tempchar, wchar_t *unit, wchar_t *str);
void GetElev(wchar_t *tempchar, wchar_t *unit, wchar_t *str);

void ClearStatusIcons();
int MapCondToStatus(MCONTACT hContact);
HICON GetStatusIcon(MCONTACT hContact);
HICON GetStatusIconBig(MCONTACT hContact);

uint16_t GetIcon(const wchar_t* cond, WIDATA *Data);
void CaseConv(wchar_t *str);
void TrimString(char *str);
void TrimString(wchar_t *str);
void ConvertBackslashes(char *str);
char *GetSearchStr(char *dis);

wchar_t *GetDisplay(WEATHERINFO *w, const wchar_t *dis, wchar_t* str);
INT_PTR GetDisplaySvcFunc(WPARAM wParam, LPARAM lParam);

void GetSvc(wchar_t *pszID);
void GetID(wchar_t *pszID);

wchar_t *GetError(int code);

// functions in weather_data.c
void GetStationID(MCONTACT hContact, wchar_t* id, int idlen);
WEATHERINFO LoadWeatherInfo(MCONTACT Change);
int DBGetData(MCONTACT hContact, char *setting, DBVARIANT *dbv);

void EraseAllInfo(void);

void GetDataValue(WIDATAITEM *UpdateData, wchar_t *Data, wchar_t** szInfo);
void ConvertDataValue(WIDATAITEM *UpdateData, wchar_t *Data);
void wSetData(char *&Data, const char *Value);
void wSetData(wchar_t *&Data, const char *Value);
void wSetData(wchar_t *&Data, const wchar_t *Value);
void wfree(char *&Data);
void wfree(wchar_t *&Data);

void DBDataManage(MCONTACT hContact, uint16_t Mode, WPARAM wParam, LPARAM lParam);

// functions in weather_http.c
int InternetDownloadFile (char *szUrl, char *cookie, char *userAgent, wchar_t** szData);
void NetlibInit();

// functions in weather_ini.c
WIDATA* GetWIData(wchar_t *pszServ);

bool IsContainedInCondList(const wchar_t *pszStr, WICONDLIST *List);

void DestroyWIList();
bool LoadWIData(bool dial);

INT_PTR CALLBACK DlgPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// functions in weather_info.c
void GetINIInfo(wchar_t *pszSvc);
wchar_t* GetINIVersionNum(int iVersion);

void MoreVarList();

// functions in weather_opt.c
void LoadOptions();
void SaveOptions();

int OptInit(WPARAM wParam,LPARAM lParam);

CMStringW GetTextValue(int c);
const wchar_t* GetDefaultText(int c);

// functions in weather_popup.c
int WeatherPopup(WPARAM wParam, LPARAM lParam);
int WeatherError(WPARAM wParam, LPARAM lParam);
int WPShowMessage(const wchar_t* lpzText, uint16_t kind);

LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// functions in weather_svcs.c
void InitServices(void);

INT_PTR WeatherSetStatus(WPARAM new_status, LPARAM lParam);
INT_PTR WeatherGetCaps(WPARAM wParam, LPARAM lParam);
INT_PTR WeatherGetName(WPARAM wParam, LPARAM lParam);
INT_PTR WeatherGetStatus(WPARAM wParam, LPARAM lParam);
INT_PTR WeatherLoadIcon(WPARAM wParam, LPARAM lParam);

void UpdateMenu(BOOL State);
void UpdatePopupMenu(BOOL State);
void AddMenuItems();
void AvatarDownloaded(MCONTACT hContact);

// functions in weather_update.c
int UpdateWeather(MCONTACT hContact);

void UpdateAll(BOOL AutoUpdate, BOOL RemoveOld);
INT_PTR UpdateSingleStation(WPARAM wParam,LPARAM lParam);
INT_PTR UpdateAllInfo(WPARAM wParam,LPARAM lParam);
INT_PTR UpdateSingleRemove(WPARAM wParam,LPARAM lParam);
INT_PTR UpdateAllRemove(WPARAM wParam,LPARAM lParam);

int GetWeatherData(MCONTACT hContact);

void CALLBACK timerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void CALLBACK timerProc2(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

// function from multiwin module
void InitMwin(void);
void DestroyMwin(void);
INT_PTR Mwin_MenuClicked(WPARAM wParam, LPARAM lParam);
int BuildContactMenu(WPARAM wparam, LPARAM lparam);
void UpdateMwinData(MCONTACT hContact);
void removeWindow(MCONTACT hContact);

// functions in weather_userinfo.c
int UserInfoInit(WPARAM wParam, LPARAM lParam);

#define WM_UPDATEDATA WM_USER + 2687

int BriefInfo(WPARAM wParam, LPARAM lParam);
INT_PTR BriefInfoSvc(WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////////////////////////////////////////
// UI Classes

class WeatherMyDetailsDlg : public CUserInfoPageDlg
{
	CCtrlButton btnReload;

public:
	WeatherMyDetailsDlg();

	bool OnInitDialog() override;

	void onClick_Reload(CCtrlButton *);
};

//============  Plugin Class ============

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	HINSTANCE hIconsDll = nullptr;
	CMOption<bool> bPopups;

	int Load() override;
	int Unload() override;
};

