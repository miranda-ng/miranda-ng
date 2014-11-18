#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <map>
#include <xfunctional> 
#include <time.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_message.h>
#include <m_hotkeys.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <win2k.h>

#include <m_toptoolbar.h>

#include "resource.h"
#include "Version.h"

#define V_RECENTCONTACTS_TOGGLE_IGNORE "RecentContacts/ToggleIgnore"

using namespace std;

void wSetData(char **Data, const char *Value);
void wfree(char **Data);

static char msLastUC_ShowListName[] =           LPGEN("Recent Contacts");

static char dbLastUC_ModuleName[] =             "RecentContacts";
static char dbLastUC_LastUsedTimeLo[] =         "LastUsedTimeLo";
static char dbLastUC_LastUsedTimeHi[] =         "LastUsedTimeHi";
static char dbLastUC_WindowPosPrefix[] =        "Window";
static char dbLastUC_DateTimeFormat[] =         "DateTimeFormat";
static char dbLastUC_DateTimeFormatDefault[] =  "(%Y-%m-%d %H:%M)  ";
static char dbLastUC_MaxShownContacts[] =       "MaxShownContacts";
static char dbLastUC_IgnoreContact[] =          "Ignore";
static char dbLastUC_HideOfflineContacts[] =    "HideOfflineContacts";
static char dbLastUC_WindowAutosize[] =		    "WindowAutoSize";

static char msLastUC_ShowList[] =               "RecentContacts/ShowList";
static char msLastUC_IgnoreOff[] =              "RecentContacts/SetIgnoreOff";
static char msLastUC_IgnoreOn[] =               "RecentContacts/SetIgnoreOn";

/////////////////////////////////////////////////////////////////////////////////////////

typedef struct _LastUCOptions
{
	int    MaxShownContacts;
	int    HideOffline;
	int    WindowAutoSize;
	string DateTimeFormat;
}
	LastUCOptions;

extern LastUCOptions LastUCOpt;
//#include "m_tabsrmm.h"
// custom tabSRMM events
#define tabMSG_WINDOW_EVT_CUSTOM_BEFORESEND 1
struct TABSRMM_SessionInfo {
	unsigned int cbSize;
	unsigned int evtCode;
	HWND hwnd;              // handle of the message dialog (tab)
	HWND hwndContainer;     // handle of the parent container
	HWND hwndInput;         // handle of the input area (rich edit)
	/*struct MessageWindowData*/ void *dat;      // the session info
	/*struct ContainerWindowData*/ void *pContainer;
};


typedef std::multimap <__time64_t, MCONTACT, std::greater<__time64_t> > cmultimap;
typedef std::pair <__time64_t, MCONTACT> cpair;

typedef struct LASTUC_DLG_DATA
{
	cmultimap *Contacts;
	SIZE WindowMinSize;
	RECT ListUCRect;
	MCONTACT hContact;
} LASTUC_DLG_DATA;