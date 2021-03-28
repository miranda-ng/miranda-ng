#pragma once

#define _CRT_NONSTDC_NO_DEPRECATE

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <map>
#include <time.h>

#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_chat_int.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_message.h>
#include <m_hotkeys.h>
#include <m_protosvc.h>
#include <m_options.h>

#include <m_toptoolbar.h>

#include "resource.h"
#include "version.h"

#define V_RECENTCONTACTS_TOGGLE_IGNORE "RecentContacts/ToggleIgnore"

using namespace std;

void wSetData(char **Data, const char *Value);
void wfree(char **Data);

#define MODULENAME                     "RecentContacts"
#define dbLastUC_LastUsedTimeLo        "LastUsedTimeLo"
#define dbLastUC_LastUsedTimeHi        "LastUsedTimeHi"
#define dbLastUC_WindowPosPrefix       "Window"
#define dbLastUC_DateTimeFormat        "DateTimeFormat"
#define dbLastUC_DateTimeFormatDefault "(%Y-%m-%d %H:%M)  "
#define dbLastUC_MaxShownContacts      "MaxShownContacts"
#define dbLastUC_IgnoreContact         "Ignore"
#define dbLastUC_HideOfflineContacts   "HideOfflineContacts"
#define dbLastUC_WindowAutosize        "WindowAutoSize"

#define msLastUC_ShowList  "RecentContacts/ShowList"
#define msLastUC_IgnoreOff "RecentContacts/SetIgnoreOff"
#define msLastUC_IgnoreOn  "RecentContacts/SetIgnoreOn"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

/////////////////////////////////////////////////////////////////////////////////////////

struct LastUCOptions
{
	int    MaxShownContacts;
	int    HideOffline;
	int    WindowAutoSize;
	string DateTimeFormat;
};

extern LastUCOptions LastUCOpt;

/////////////////////////////////////////////////////////////////////////////////////////

typedef std::multimap <__time64_t, MCONTACT, std::greater<__time64_t> > cmultimap;
typedef std::pair <__time64_t, MCONTACT> cpair;

struct LASTUC_DLG_DATA
{
	cmultimap *Contacts;
	SIZE WindowMinSize;
	RECT ListUCRect;
	MCONTACT hContact;
};
