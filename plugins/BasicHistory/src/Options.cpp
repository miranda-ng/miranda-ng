/*
Basic History plugin
Copyright (C) 2011-2012 Krzysztof Kral

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "StdAfx.h"
#include "Options.h"
#include "resource.h"

extern HINSTANCE hInst;
extern bool g_SmileyAddAvail;
extern bool bPopupsEnabled;

#define MODULE "BasicHistory"

Options *Options::instance;

struct EventNamesType
{
	int id;
	TCHAR* name;
}
EventNames[] = 
{
	EVENTTYPE_MESSAGE, LPGENT("Message"),
	EVENTTYPE_FILE, LPGENT("File transfer"),
	EVENTTYPE_URL, LPGENT("Link"),
	EVENTTYPE_AUTHREQUEST, LPGENT("Authorization request"),
	EVENTTYPE_ADDED, LPGENT("You were added"),
	EVENTTYPE_CONTACTS, LPGENT("Contacts received"),
	ICQEVENTTYPE_SMS, LPGENT("SMS message")
};

struct TCpTable {
	UINT cpId;
	TCHAR *cpName;
}
cpTable[] = {
	{ CP_UTF8,	_T("UTF-8")	 },
	{ 1250,	_T("windows-1250")	 },
	{ 1251,	_T("windows-1251") },
	{ 1252,	_T("windows-1252") },
	{ 1253,	_T("windows-1253") },
	{ 1254,	_T("windows-1254") },
	{ 1255,	_T("windows-1255") },
	{ 1256,	_T("windows-1256") },
	{ 1257,	_T("windows-1257") },
	{ 1258,	_T("windows-1258") },
	{ 28591,	_T("iso-8859-1") },
	{ 28592,	_T("iso-8859-2") },
	{ 28593,	_T("iso-8859-3") },
	{ 28594,	_T("iso-8859-4") },
	{ 28595,	_T("iso-8859-5") },
	{ 28596,	_T("iso-8859-6") },
	{ 28597,	_T("iso-8859-7") },
	{ 28598,	_T("iso-8859-8") },
	{ 28599,	_T("iso-8859-9") },
	{ 28603,	_T("iso-8859-13") },
	{ 28605,	_T("iso-8859-15") },
};

Options::Options()
{
	showContacts = false;
	showContactGroups = true;
	noFindBorder = false;
	groupNewOnTop = true;
	groupShowEvents = true;
	groupShowTime = true;
	groupShowName = false;
	groupShowMessage = true;
	groupMessageLen = 43;
	groupTime = 5;
	groupMessagesNumber = 100;
	messagesNewOnTop = false;
	messagesShowDate = false;
	messagesShowSec = false;
	messagesShowName = true;
	messagesShowEvents = false;
	messagesUseSmileys = true;
	searchForInList = true;
	searchForInMess = true;
	searchMatchCase = false;
	searchMatchWhole = false;
	searchOnlyIn = false;
	searchOnlyOut = false;
	searchOnlyGroup = false;
	searchAllContacts = false;
	schedulerAlerts = true;
	schedulerHistoryAlerts = true;
	defFilter = 0;
	codepageTxt = CP_UTF8;
	codepageHtml1 = CP_UTF8;
	codepageHtml2 = CP_UTF8;
	encodingTxt = _T("UTF-8");
	encodingHtml1 = _T("UTF-8");
	encodingHtml2 = _T("UTF-8");
	exportHtml1ShowDate = true;
	exportHtml2ShowDate = false;
	exportHtml2UseSmileys = true;
}

Options::~Options()
{
}

int Options::InitOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.ptszTitle = LPGENT("History");

	odp.ptszTab = LPGENT("Group list");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GROUPLIST);
	odp.pfnDlgProc = Options::DlgProcOptsGroupList;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Messages");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MESSAGES);
	odp.pfnDlgProc = Options::DlgProcOptsMessages;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Searching");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SEARCHING);
	odp.pfnDlgProc = Options::DlgProcOptsSearching;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Export");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_EXPORT);
	odp.pfnDlgProc = Options::DlgProcOptsExport;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Scheduler");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SCHEDULER);
	odp.pfnDlgProc = Options::DlgProcOptsScheduler;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc = Options::DlgProcOptsMain;
	Options_AddPage(wParam, &odp);
	return 0;
}

struct FontOptionsList {
	TCHAR*   szDescr;
	COLORREF defColour;
	TCHAR*   szDefFace;
	BYTE     defStyle;
	char     defSize;
	TCHAR*   szBackgroundName;
	DWORD     flags;
};

struct ColorOptionsList {
	TCHAR*		tszName;
	COLORREF 	def;
};

struct HotkeyOptionsList {
	const char *pszName;
	const TCHAR *ptszDescription;
	const TCHAR *ptszSection;
	const char *pszService;
	WORD DefHotKey;
	LPARAM lParam;
};

static FontOptionsList g_FontOptionsList[] = {
	{LPGENT(">> Outgoing timestamp"), RGB(0, 0, 0), _T("MS Shell Dlg 2"), DBFONTF_BOLD, -11, LPGENT("Outgoing background"), FIDF_ALLOWEFFECTS},
	{LPGENT("<< Incoming timestamp"), RGB(0, 0, 0), _T("MS Shell Dlg 2"), DBFONTF_BOLD, -11, LPGENT("Incoming background"), FIDF_ALLOWEFFECTS},
	{LPGENT(">> Outgoing name"), RGB(100,100,100), _T("MS Shell Dlg 2"), DBFONTF_BOLD, -11, LPGENT("Outgoing background"), FIDF_ALLOWEFFECTS},
	{LPGENT("<< Incoming name"), RGB(90,160,90), _T("MS Shell Dlg 2"), DBFONTF_BOLD, -11, LPGENT("Incoming background"), FIDF_ALLOWEFFECTS},
	{LPGENT(">> Outgoing messages"), RGB(0, 0, 0), _T("MS Shell Dlg 2"), 0, -11, LPGENT("Outgoing background"), FIDF_ALLOWEFFECTS},
	{LPGENT("<< Incoming messages"), RGB(0, 0, 0), _T("MS Shell Dlg 2"), 0, -11, LPGENT("Incoming background"), FIDF_ALLOWEFFECTS},
	{LPGENT("Group list"), RGB(0, 0, 0), _T("MS Shell Dlg 2"), 0, -11, LPGENT("Group list background"), FIDF_DISABLESTYLES},
	{LPGENT("Find window"), RGB(0, 0, 0), _T("MS Shell Dlg 2"), 0, -11, LPGENT("Find window background"), FIDF_DISABLESTYLES},
};

static ColorOptionsList g_ColorOptionsList[] = {
	LPGENT("Outgoing background"), RGB(245,245,255),
	LPGENT("Incoming background"), RGB(245,255,245),
	LPGENT("Group list background"), GetSysColor(COLOR_3DFACE),
	LPGENT("Window background"), GetSysColor(COLOR_3DFACE),
	LPGENT("Contact list background"), GetSysColor(COLOR_3DFACE),
	LPGENT("Find window background"), GetSysColor(COLOR_WINDOW),
};

static HotkeyOptionsList g_HotkeyOptionsList[] = {
	{ "basichistory_hot_showall", LPGENT("Open global history"), LPGENT("History"), MS_HISTORY_SHOWCONTACTHISTORY, HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'H') | HKF_MIRANDA_LOCAL, 0 },
	{ "basichistory_hot_find", LPGENT("Find"), LPGENT("History"), 0, HOTKEYCODE(HOTKEYF_CONTROL, 'F'), HISTORY_HK_FIND },
	{ "basichistory_hot_findnext", LPGENT("Find Next"), LPGENT("History"), 0, VK_F3, HISTORY_HK_FINDNEXT },
	{ "basichistory_hot_findprev", LPGENT("Find Previous"), LPGENT("History"), 0, VK_F2, HISTORY_HK_FINDPREV },
	{ "basichistory_hot_matchcase", LPGENT("Switch Match Case"), LPGENT("History"), 0, 0, HISTORY_HK_MATCHCASE },
	{ "basichistory_hot_matchwhole", LPGENT("Switch Match Whole Word"), LPGENT("History"), 0, 0, HISTORY_HK_MATCHWHOLE },
	{ "basichistory_hot_showcontacts", LPGENT("Show/Hide Contacts"), LPGENT("History"), 0, 0, HISTORY_HK_SHOWCONTACTS },
	{ "basichistory_hot_onlyin", LPGENT("Switch Only Incoming Messages"), LPGENT("History"), 0, 0, HISTORY_HK_ONLYIN },
	{ "basichistory_hot_onlyout", LPGENT("Switch Only Outgoing Messages"), LPGENT("History"), 0, 0, HISTORY_HK_ONLYOUT },
	{ "basichistory_hot_onlygroup", LPGENT("Switch Only Selected Group"), LPGENT("History"), 0, 0, HISTORY_HK_ONLYGROUP },
	{ "basichistory_hot_allcontacts", LPGENT("Switch All Contacts"), LPGENT("History"), 0, 0, HISTORY_HK_ALLCONTACTS },
	{ "basichistory_hot_delete", LPGENT("Delete"), LPGENT("History"), 0, VK_DELETE, HISTORY_HK_DELETE },
	{ "basichistory_hot_exrhtml", LPGENT("Export To Rich Html"), LPGENT("History"), 0, 0, HISTORY_HK_EXRHTML },
	{ "basichistory_hot_exphtml", LPGENT("Export To Plain Html"), LPGENT("History"), 0, 0, HISTORY_HK_EXPHTML },
	{ "basichistory_hot_extxt", LPGENT("Export To Txt"), LPGENT("History"), 0, 0, HISTORY_HK_EXTXT },
	{ "basichistory_hot_exbin", LPGENT("Export To Binary"), LPGENT("History"), 0, 0, HISTORY_HK_EXBIN },
	{ "basichistory_hot_impbin", LPGENT("Import From Binary"), LPGENT("History"), 0, 0, HISTORY_HK_IMPBIN },
	{ "basichistory_hot_exdat", LPGENT("Export To Dat (mContacts)"), LPGENT("History"), 0, 0, HISTORY_HK_EXDAT },
	{ "basichistory_hot_impdat", LPGENT("Import From Dat (mContacts)"), LPGENT("History"), 0, 0, HISTORY_HK_IMPDAT },
};

const int g_fontsSize = SIZEOF(g_FontOptionsList);

const int g_colorsSize = SIZEOF(g_ColorOptionsList);

const int g_hotkeysSize = SIZEOF(g_HotkeyOptionsList);

void Options::Load(void)
{
	FontIDT fid = {0};
	ColourIDT cid = {0};
	HOTKEYDESC hid = {0};
	fid.cbSize = sizeof(FontIDT);
	cid.cbSize = sizeof(ColourIDT);
	hid.cbSize = sizeof(HOTKEYDESC);
	strncpy_s(fid.dbSettingsGroup, "BasicHistory_Fonts", _TRUNCATE);
	_tcsncpy_s(fid.backgroundGroup, _T("History"), _TRUNCATE);
	_tcsncpy_s(fid.group, LPGENT("History"), _TRUNCATE);
	for (int i = 0; i < g_fontsSize; ++i) {
		fid.order = i;
		_tcsncpy_s(fid.deffontsettings.szFace, g_FontOptionsList[i].szDefFace, _TRUNCATE);
		fid.deffontsettings.size = g_FontOptionsList[i].defSize; 
		fid.deffontsettings.colour = g_FontOptionsList[i].defColour;
		fid.deffontsettings.style = g_FontOptionsList[i].defStyle;
		fid.deffontsettings.charset = DEFAULT_CHARSET;
		mir_snprintf(fid.prefix, SIZEOF(fid.prefix), "Font%d", i);
		_tcsncpy_s(fid.name, g_FontOptionsList[i].szDescr, _TRUNCATE);
		_tcsncpy_s(fid.backgroundName, g_FontOptionsList[i].szBackgroundName, _TRUNCATE);
		fid.flags = FIDF_DEFAULTVALID | FIDF_CLASSGENERAL | g_FontOptionsList[i].flags;
		FontRegisterT(&fid);
	}
	
	strncpy_s(cid.dbSettingsGroup, "BasicHistory_Fonts", _TRUNCATE);
	_tcsncpy_s(cid.group, LPGENT("History"), _TRUNCATE);
	for (int i = 0; i < g_colorsSize; ++i) {
		_tcsncpy_s(cid.name, g_ColorOptionsList[i].tszName, _TRUNCATE);
		mir_snprintf(cid.setting, SIZEOF(cid.setting), "Color%d", i);
		cid.order = i;
		cid.defcolour = g_ColorOptionsList[i].def;
		ColourRegisterT(&cid);
	}

	hid.dwFlags = HKD_TCHAR;
	for (int i = 0; i < g_hotkeysSize; ++i) {
		hid.pszName = g_HotkeyOptionsList[i].pszName;
		hid.ptszDescription = g_HotkeyOptionsList[i].ptszDescription;
		hid.ptszSection = g_HotkeyOptionsList[i].ptszSection;
		hid.pszService = g_HotkeyOptionsList[i].pszService;
		hid.DefHotKey = g_HotkeyOptionsList[i].DefHotKey;
		hid.lParam = g_HotkeyOptionsList[i].lParam;
		Hotkey_Register(&hid);
	}
	
	showContacts = db_get_b(0, MODULE, "showContacts", 0) ? true : false;
	showContactGroups = db_get_b(0, MODULE, "showContactGroups", 1) ? true : false;
	noFindBorder = db_get_b(0, MODULE, "noFindBorder", 0) ? true : false;
	groupNewOnTop = db_get_b(0, MODULE, "groupNewOnTop", 1) ? true : false;
	groupShowEvents = db_get_b(0, MODULE, "groupShowEvents", 1) ? true : false;
	groupShowTime = db_get_b(0, MODULE, "groupShowTime", 1) ? true : false;
	groupShowName = db_get_b(0, MODULE, "groupShowName", 0) ? true : false;
	groupShowMessage = db_get_b(0, MODULE, "groupShowMessage", 1) ? true : false;
	groupMessageLen = db_get_dw(0, MODULE, "groupMessageLen", 43);
	if (groupMessageLen < 5) groupMessageLen = 5;
	groupTime = db_get_dw(0, MODULE, "groupTime", 5);
	if (groupTime < 1) groupTime = 1;
	groupMessagesNumber = db_get_dw(0, MODULE, "groupMessagesNumber", 100);
	if (groupMessagesNumber < 1) groupMessagesNumber = 1;
	messagesNewOnTop = db_get_b(0, MODULE, "messagesNewOnTop", 0) ? true : false;
	messagesShowDate = db_get_b(0, MODULE, "messagesShowDate", 0) ? true : false;
	messagesShowSec = db_get_b(0, MODULE, "messagesShowSec", 0) ? true : false;
	messagesShowName = db_get_b(0, MODULE, "messagesShowName", 1) ? true : false;
	messagesShowEvents = db_get_b(0, MODULE, "messagesShowEvents", 0) ? true : false;
	messagesUseSmileys = db_get_b(0, MODULE, "messagesUseSmileys", 1) ? true : false;
	searchForInList = db_get_b(0, MODULE, "searchForInList", 1) ? true : false;
	searchForInMess = db_get_b(0, MODULE, "searchForInMess", 1) ? true : false;
	searchMatchCase = db_get_b(0, MODULE, "searchMatchCase", 0) ? true : false;
	searchMatchWhole = db_get_b(0, MODULE, "searchMatchWhole", 0) ? true : false;
	searchOnlyIn = db_get_b(0, MODULE, "searchOnlyIn", 0) ? true : false;
	searchOnlyOut = db_get_b(0, MODULE, "searchOnlyOut", 0) ? true : false;
	searchOnlyGroup = db_get_b(0, MODULE, "searchOnlyGroup", 0) ? true : false;
	searchAllContacts = db_get_b(0, MODULE, "searchAllContacts", 0) ? true : false;
	schedulerAlerts = db_get_b(0, MODULE, "schedulerAlerts", 1) ? true : false;
	schedulerHistoryAlerts = db_get_b(0, MODULE, "schedulerHistoryAlerts", 1) ? true : false;

	defFilter = db_get_b(0, MODULE, "defFilter", defFilter);
	int filtersCount = db_get_dw(0, MODULE, "customFiltersCount", 0);
	for (int i = 0; i < filtersCount; ++i) {
		char buf[256];
		FilterOptions fo;
		mir_snprintf(buf, SIZEOF(buf), "filterName_%d", i);
		DBVARIANT nameV;
		if (!db_get_ws(0, MODULE, buf, &nameV)) {
			fo.name = nameV.pwszVal;
			db_free(&nameV);
		}
		else break;
		mir_snprintf(buf, SIZEOF(buf), "filterInOut_%d", i);
		int inOut = db_get_b(0, MODULE, buf, 0);
		if (inOut == 1)
			fo.onlyIncomming = true;
		else if (inOut == 2)
			fo.onlyOutgoing = true;
		mir_snprintf(buf, SIZEOF(buf), "filterEvents_%d", i);
		DBVARIANT eventsV;
		if (!db_get_s(0, MODULE, buf, &eventsV)) {
			int k = 0;
			char* id = eventsV.pszVal;
			while(eventsV.pszVal[k]) {
				if (eventsV.pszVal[k] == ';') {
					eventsV.pszVal[k] = 0;
					fo.events.push_back(strtol(id, NULL, 16));
					id = eventsV.pszVal + k + 1;
				}

				++k;
			}
			db_free(&eventsV);
		}
		else break;
		
		customFilters.insert(customFilters.end(), fo);
	}

	if (defFilter > 1) {
		defFilter = 0;
		
		DBVARIANT defFilterStrV;
		if (!db_get_ws(0, MODULE, "defFilterStr", &defFilterStrV)) {
			std::wstring filterName = defFilterStrV.pwszVal;
			for (int i = 0; i < (int)customFilters.size(); ++i) {
				if (filterName == customFilters[i].name) {
					defFilter = i + 2;
					break;
				}
			}
			db_free(&defFilterStrV);
		}
	}
	
	codepageTxt = db_get_dw(0, MODULE, "codepageTxt", CP_UTF8);
	codepageHtml1 = db_get_dw(0, MODULE, "codepageHtml1", CP_UTF8);
	codepageHtml2 = db_get_dw(0, MODULE, "codepageHtml2", CP_UTF8);
	DBVARIANT encodingV;
	if (!db_get_ws(0, MODULE, "encodingTxt", &encodingV)) {
		encodingTxt = encodingV.pwszVal;
		db_free(&encodingV);
	}
	else encodingTxt = _T("UTF-8");

	if (!db_get_ws(0, MODULE, "encodingHtml1", &encodingV)) {
		encodingHtml1 = encodingV.pwszVal;
		db_free(&encodingV);
	}
	else encodingHtml1 = _T("UTF-8");

	if (!db_get_ws(0, MODULE, "encodingHtml2", &encodingV)) {
		encodingHtml2 = encodingV.pwszVal;
		db_free(&encodingV);
	}
	else encodingHtml2 = _T("UTF-8");

	exportHtml1ShowDate = db_get_b(0, MODULE, "exportHtml1ShowDate", 1) ? true : false;
	exportHtml2ShowDate = db_get_b(0, MODULE, "exportHtml2ShowDate", 0) ? true : false;
	exportHtml2UseSmileys = db_get_b(0, MODULE, "exportHtml2UseSmileys", 1) ? true : false;
	if (!db_get_ws(0, MODULE, "extCssHtml2", &encodingV)) {
		extCssHtml2 = encodingV.pwszVal;
		db_free(&encodingV);
	}
	else extCssHtml2 = _T("");

	if (!db_get_ws(0, MODULE, "ftpLogPath", &encodingV)) {
		ftpLogPath = encodingV.pwszVal;
		db_free(&encodingV);
	}

	if (!db_get_ws(0, MODULE, "ftpExePath", &encodingV)) {
		ftpExePath = encodingV.pwszVal;
		db_free(&encodingV);
	}
	else ftpExePath = ftpExePathDef;
	
	LoadTasks();
}

COLORREF Options::GetFont(Fonts fontId, PLOGFONT font)
{
	FontIDT fid = {0};
	fid.cbSize = sizeof(FontIDT);
	_tcsncpy_s(fid.group, LPGENT("History"), _TRUNCATE);
	_tcsncpy_s(fid.name, g_FontOptionsList[fontId].szDescr, _TRUNCATE);
	return (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)font);
}

COLORREF Options::GetColor(Colors colorId)
{
	ColourIDT cid = {0};
	cid.cbSize = sizeof(ColourIDT);
	_tcsncpy_s(cid.group, LPGENT("History"), _TRUNCATE);
	_tcsncpy_s(cid.name, g_ColorOptionsList[colorId].tszName, _TRUNCATE);
	return (COLORREF)CallService(MS_COLOUR_GETT, (WPARAM)&cid, NULL);
}

void Options::Save()
{
	db_set_b(0, MODULE, "showContacts", showContacts ? 1 : 0);
	db_set_b(0, MODULE, "showContactGroups", showContactGroups ? 1 : 0);
	db_set_b(0, MODULE, "noFindBorder", noFindBorder ? 1 : 0);
	db_set_b(0, MODULE, "groupNewOnTop", groupNewOnTop ? 1 : 0);
	db_set_b(0, MODULE, "groupShowEvents", groupShowEvents ? 1 : 0);
	db_set_b(0, MODULE, "groupShowTime", groupShowTime ? 1 : 0);
	db_set_b(0, MODULE, "groupShowName", groupShowName ? 1 : 0);
	db_set_b(0, MODULE, "groupShowMessage", groupShowMessage ? 1 : 0);
	if (groupMessageLen < 5)
		groupMessageLen = 5;
	db_set_dw(0, MODULE, "groupMessageLen", groupMessageLen);
	if (groupTime < 1)
		groupTime = 1;
	db_set_dw(0, MODULE, "groupTime", groupTime);
	if (groupMessagesNumber < 1)
		groupMessagesNumber = 1;
	db_set_dw(0, MODULE, "groupMessagesNumber", groupMessagesNumber);
	db_set_b(0, MODULE, "messagesNewOnTop", messagesNewOnTop ? 1 : 0);
	db_set_b(0, MODULE, "messagesShowDate", messagesShowDate ? 1 : 0);
	db_set_b(0, MODULE, "messagesShowSec", messagesShowSec ? 1 : 0);
	db_set_b(0, MODULE, "messagesShowName", messagesShowName ? 1 : 0);
	db_set_b(0, MODULE, "messagesShowEvents", messagesShowEvents ? 1 : 0);
	db_set_b(0, MODULE, "messagesUseSmileys", messagesUseSmileys ? 1 : 0);
	db_set_b(0, MODULE, "searchForInList", searchForInList ? 1 : 0);
	db_set_b(0, MODULE, "searchForInMess", searchForInMess ? 1 : 0);
	db_set_b(0, MODULE, "searchMatchCase", searchMatchCase ? 1 : 0);
	db_set_b(0, MODULE, "searchMatchWhole", searchMatchWhole ? 1 : 0);
	db_set_b(0, MODULE, "searchOnlyIn", searchOnlyIn ? 1 : 0);
	db_set_b(0, MODULE, "searchOnlyOut", searchOnlyOut ? 1 : 0);
	db_set_b(0, MODULE, "searchOnlyGroup", searchOnlyGroup ? 1 : 0);
	db_set_b(0, MODULE, "searchAllContacts", searchAllContacts ? 1 : 0);
	db_set_b(0, MODULE, "schedulerAlerts", schedulerAlerts ? 1 : 0);
	db_set_b(0, MODULE, "schedulerHistoryAlerts", schedulerHistoryAlerts ? 1 : 0);
	if (defFilter < 0 || defFilter - 2 >= (int)customFilters.size())
		defFilter = 0;
	db_set_b(0, MODULE, "defFilter", defFilter < 2 ? defFilter : 2);
	if (defFilter >= 2)
		db_set_ws(0, MODULE, "defFilterStr", customFilters[defFilter - 2].name.c_str());
	db_set_dw(0, MODULE, "customFiltersCount", (DWORD)customFilters.size());
	for (int i = 0 ; i < (int)customFilters.size(); ++i) {
		char buf[256];
		mir_snprintf(buf, SIZEOF(buf), "filterName_%d", i);
		db_set_ws(0, MODULE, buf, customFilters[i].name.c_str());
		mir_snprintf(buf, SIZEOF(buf), "filterInOut_%d", i);
		db_set_b(0, MODULE, buf, customFilters[i].onlyIncomming ? 1 : (customFilters[i].onlyOutgoing ? 2 : 0));
		std::string events;
		for (std::vector<int>::iterator it = customFilters[i].events.begin(); it != customFilters[i].events.end(); ++it) {
			_itoa_s(*it, buf, 16);
			events += buf;
			events += ";";
		}

		mir_snprintf(buf, SIZEOF(buf), "filterEvents_%d", i);
		db_set_s(0, MODULE, buf, events.c_str());
	}

	db_set_dw(0, MODULE, "codepageTxt", codepageTxt);
	db_set_dw(0, MODULE, "codepageHtml1", codepageHtml1);
	db_set_dw(0, MODULE, "codepageHtml2", codepageHtml2);
	db_set_ws(0, MODULE, "encodingTxt", encodingTxt.c_str());
	db_set_ws(0, MODULE, "encodingHtml1", encodingHtml1.c_str());
	db_set_ws(0, MODULE, "encodingHtml2", encodingHtml2.c_str());
	db_set_b(0, MODULE, "exportHtml1ShowDate", exportHtml1ShowDate ? 1 : 0);
	db_set_b(0, MODULE, "exportHtml2ShowDate", exportHtml2ShowDate ? 1 : 0);
	db_set_b(0, MODULE, "exportHtml2UseSmileys", exportHtml2UseSmileys ? 1 : 0);
	db_set_ws(0, MODULE, "extCssHtml2", extCssHtml2.c_str());
	db_set_ws(0, MODULE, "ftpLogPath", ftpLogPath.c_str());
	if (ftpExePath != ftpExePathDef)
		db_set_ws(0, MODULE, "ftpExePath", ftpExePath.c_str());
	else
		db_unset(0, MODULE, "ftpExePath");
}

void Options::SaveTasks(std::list<TaskOptions>* tasks)
{
	mir_cslock lck(criticalSection);
	int oldTaskNr = (int)taskOptions.size();
	taskOptions.clear();
	int i = 0;
	char buf[256];
	for (std::list<TaskOptions>::iterator it = tasks->begin(); it != tasks->end(); ++it) {
		mir_snprintf(buf, SIZEOF(buf), "Task_compress_%d", i);
		db_set_b(0, MODULE, buf, it->compress);
		mir_snprintf(buf, SIZEOF(buf), "Task_useFtp_%d", i);
		db_set_b(0, MODULE, buf, it->useFtp);
		mir_snprintf(buf, SIZEOF(buf), "Task_isSystem_%d", i);
		db_set_b(0, MODULE, buf, it->isSystem);
		mir_snprintf(buf, SIZEOF(buf), "Task_active_%d", i);
		db_set_b(0, MODULE, buf, it->active);
		mir_snprintf(buf, SIZEOF(buf), "Task_exportImported_%d", i);
		db_set_b(0, MODULE, buf, it->exportImported);
		mir_snprintf(buf, SIZEOF(buf), "Task_type_%d", i);
		db_set_b(0, MODULE, buf, it->type);
		mir_snprintf(buf, SIZEOF(buf), "Task_eventUnit_%d", i);
		db_set_b(0, MODULE, buf, it->eventUnit);
		mir_snprintf(buf, SIZEOF(buf), "Task_trigerType_%d", i);
		db_set_b(0, MODULE, buf, it->trigerType);
		mir_snprintf(buf, SIZEOF(buf), "Task_exportType_%d", i);
		db_set_b(0, MODULE, buf, it->exportType);
		mir_snprintf(buf, SIZEOF(buf), "Task_importType_%d", i);
		db_set_b(0, MODULE, buf, it->importType);
		mir_snprintf(buf, SIZEOF(buf), "Task_eventDeltaTime_%d", i);
		db_set_dw(0, MODULE, buf, it->eventDeltaTime);
		mir_snprintf(buf, SIZEOF(buf), "Task_filterId_%d", i);
		db_set_dw(0, MODULE, buf, it->filterId);
		mir_snprintf(buf, SIZEOF(buf), "Task_dayTime_%d", i);
		db_set_dw(0, MODULE, buf, it->dayTime);
		mir_snprintf(buf, SIZEOF(buf), "Task_dayOfWeek_%d", i);
		db_set_dw(0, MODULE, buf, it->dayOfWeek);
		mir_snprintf(buf, SIZEOF(buf), "Task_dayOfMonth_%d", i);
		db_set_dw(0, MODULE, buf, it->dayOfMonth);
		mir_snprintf(buf, SIZEOF(buf), "Task_deltaTime_%d", i);
		db_set_dw(0, MODULE, buf, it->deltaTime);
		mir_snprintf(buf, SIZEOF(buf), "Task_lastExport_low_%d", i);
		db_set_dw(0, MODULE, buf, (int)it->lastExport);
		mir_snprintf(buf, SIZEOF(buf), "Task_lastExport_hi_%d", i);
		db_set_dw(0, MODULE, buf, ((unsigned long long int)it->lastExport) >> 32);
		mir_snprintf(buf, SIZEOF(buf), "Task_ftpName_%d", i);
		db_set_ws(0, MODULE, buf, it->ftpName.c_str());
		mir_snprintf(buf, SIZEOF(buf), "Task_filterName_%d", i);
		db_set_ws(0, MODULE, buf, it->filterName.c_str());
		mir_snprintf(buf, SIZEOF(buf), "Task_filePath_%d", i);
		db_set_ws(0, MODULE, buf, it->filePath.c_str());
		mir_snprintf(buf, SIZEOF(buf), "Task_taskName_%d", i);
		db_set_ws(0, MODULE, buf, it->taskName.c_str());
		mir_snprintf(buf, SIZEOF(buf), "Task_zipPassword_%d", i);
		db_set_s(0, MODULE, buf, it->zipPassword.c_str());

		mir_snprintf(buf, SIZEOF(buf), "IsInTask_%d", i);
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
			db_unset(hContact, MODULE, buf);

		for (size_t j = 0; j < it->contacts.size(); ++j)
			db_set_b(it->contacts[j], MODULE, buf, 1);

		it->orderNr = i++;
		taskOptions.push_back(*it);
	}

	db_set_dw(0, MODULE, "Task_count", i);
	
	for (i = (int)tasks->size(); i < oldTaskNr; ++i) {
		mir_snprintf(buf, SIZEOF(buf), "Task_compress_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_useFtp_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_isSystem_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_active_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_type_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_eventUnit_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_trigerType_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_exportType_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_eventDeltaTime_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_filterId_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_dayTime_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_dayOfWeek_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_dayOfMonth_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_deltaTime_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_lastExport_low_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_lastExport_hi_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_ftpName_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_filterName_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_filePath_%d", i);
		db_unset(NULL, MODULE, buf);
		mir_snprintf(buf, SIZEOF(buf), "Task_taskName_%d", i);
		db_unset(NULL, MODULE, buf);

		mir_snprintf(buf, SIZEOF(buf), "IsInTask_%d", i);
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
			db_unset(hContact, MODULE, buf);
	}
}

void Options::SaveTaskTime(TaskOptions& to)
{
	int i = to.orderNr;
	char buf[256];
	mir_snprintf(buf, SIZEOF(buf), "Task_lastExport_low_%d", i);
	db_set_dw(0, MODULE, buf, (int)to.lastExport);
	mir_snprintf(buf, SIZEOF(buf), "Task_lastExport_hi_%d", i);
	db_set_dw(0, MODULE, buf, ((unsigned long long int)to.lastExport) >> 32);
}

void Options::LoadTasks()
{
	int taskCount = db_get_dw(0, MODULE, "Task_count", 0);
	char buf[256];
	for (int i = 0; i < taskCount; ++i) {
		TaskOptions to;
		mir_snprintf(buf, SIZEOF(buf), "Task_compress_%d", i);
		to.compress = db_get_b(0, MODULE, buf, to.compress) != 0;
		mir_snprintf(buf, SIZEOF(buf), "Task_useFtp_%d", i);
		to.useFtp = db_get_b(0, MODULE, buf, to.useFtp) != 0;
		mir_snprintf(buf, SIZEOF(buf), "Task_isSystem_%d", i);
		to.isSystem = db_get_b(0, MODULE, buf, to.isSystem) != 0;
		mir_snprintf(buf, SIZEOF(buf), "Task_active_%d", i);
		to.active = db_get_b(0, MODULE, buf, to.active) != 0;
		mir_snprintf(buf, SIZEOF(buf), "Task_exportImported_%d", i);
		to.exportImported = db_get_b(0, MODULE, buf, to.exportImported) != 0;
		mir_snprintf(buf, SIZEOF(buf), "Task_type_%d", i);
		to.type = (TaskOptions::TaskType)db_get_b(0, MODULE, buf, to.type);
		mir_snprintf(buf, SIZEOF(buf), "Task_eventUnit_%d", i);
		to.eventUnit = (TaskOptions::EventUnit)db_get_b(0, MODULE, buf, to.eventUnit);
		mir_snprintf(buf, SIZEOF(buf), "Task_trigerType_%d", i);
		to.trigerType = (TaskOptions::TrigerType)db_get_b(0, MODULE, buf, to.trigerType);
		mir_snprintf(buf, SIZEOF(buf), "Task_exportType_%d", i);
		to.exportType = (IExport::ExportType)db_get_b(0, MODULE, buf, to.exportType);
		mir_snprintf(buf, SIZEOF(buf), "Task_importType_%d", i);
		to.importType = (IImport::ImportType)db_get_b(0, MODULE, buf, to.importType);
		mir_snprintf(buf, SIZEOF(buf), "Task_eventDeltaTime_%d", i);
		to.eventDeltaTime = db_get_dw(0, MODULE, buf, to.eventDeltaTime);
		mir_snprintf(buf, SIZEOF(buf), "Task_filterId_%d", i);
		to.filterId = db_get_dw(0, MODULE, buf, to.filterId);
		mir_snprintf(buf, SIZEOF(buf), "Task_dayTime_%d", i);
		to.dayTime = db_get_dw(0, MODULE, buf, to.dayTime);
		mir_snprintf(buf, SIZEOF(buf), "Task_dayOfWeek_%d", i);
		to.dayOfWeek = db_get_dw(0, MODULE, buf, to.dayOfWeek);
		mir_snprintf(buf, SIZEOF(buf), "Task_dayOfMonth_%d", i);
		to.dayOfMonth = db_get_dw(0, MODULE, buf, to.dayOfMonth);
		mir_snprintf(buf, SIZEOF(buf), "Task_deltaTime_%d", i);
		to.deltaTime = db_get_dw(0, MODULE, buf, to.deltaTime);
		unsigned long long int le = to.lastExport;
		mir_snprintf(buf, SIZEOF(buf), "Task_lastExport_low_%d", i);
		to.lastExport = db_get_dw(0, MODULE, buf, (int)le) & 0xffffffff;
		mir_snprintf(buf, SIZEOF(buf), "Task_lastExport_hi_%d", i);
		to.lastExport |= ((unsigned long long int)db_get_dw(0, MODULE, buf, le >> 32)) << 32;
		mir_snprintf(buf, SIZEOF(buf), "Task_ftpName_%d", i);
		DBVARIANT var;
		if (!db_get_ws(0, MODULE, buf, &var)) {
			to.ftpName = var.ptszVal;
			db_free(&var);
		}
		mir_snprintf(buf, SIZEOF(buf), "Task_filterName_%d", i);
		if (!db_get_ws(0, MODULE, buf, &var)) {
			to.filterName = var.ptszVal;
			db_free(&var);
		}
		mir_snprintf(buf, SIZEOF(buf), "Task_filePath_%d", i);
		if (!db_get_ws(0, MODULE, buf, &var)) {
			to.filePath = var.ptszVal;
			db_free(&var);
		}
		mir_snprintf(buf, SIZEOF(buf), "Task_taskName_%d", i);
		if (!db_get_ws(0, MODULE, buf, &var)) {
			to.taskName = var.ptszVal;
			db_free(&var);
		}
		mir_snprintf(buf, SIZEOF(buf), "Task_zipPassword_%d", i);
		if (!db_get_s(0, MODULE, buf, &var)) {
			to.zipPassword = var.pszVal;
			db_free(&var);
		}

		mir_snprintf(buf, SIZEOF(buf), "IsInTask_%d", i);
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
			if (db_get_b(hContact, MODULE, buf, 0) == 1)
				to.contacts.push_back(hContact);

		to.orderNr = i;
		taskOptions.push_back(to);
	}
}

void OptionsMainChanged();
void OptionsGroupChanged();
void OptionsMessageChanged();
void OptionsSearchingChanged();
void OptionsSchedulerChanged();
void InitTaskMenuItems();

void SetEventCB(HWND hwndCB, int eventId)
{
	int cpCount = SIZEOF(EventNames);
	int selCpIdx = -1;
	for (int i = 0; i < cpCount; ++i)
		if (EventNames[i].id == eventId)
			selCpIdx = i;

	if (selCpIdx == -1) {
		TCHAR buf[24];
		mir_sntprintf(buf, SIZEOF(buf), _T("%d"), eventId);
		ComboBox_SetText(hwndCB, buf);	
	}
	else ComboBox_SetCurSel(hwndCB, selCpIdx);	
}

int GetEventCB(HWND hwndCB, bool errorReport, int &eventId)
{
	int selCpIdx = ComboBox_GetCurSel(hwndCB);
	if (selCpIdx < 0) {
		TCHAR text[24];
		ComboBox_GetText(hwndCB, text, 24);
		TCHAR * stopOn = NULL;
		long cp = _tcstol(text, &stopOn, 10);
		if (errorReport && (stopOn == text || *stopOn != '\0' || cp < 0)) {
			MessageBox(GetParent(hwndCB), TranslateT("Invalid event number"), TranslateT("Error"), MB_OK | MB_ICONERROR);
			SetFocus(hwndCB);
			return -1;
		}

		eventId = cp;
	}
	else if (selCpIdx > 1)
		eventId = EventNames[selCpIdx - 2].id;
	else
		return selCpIdx + 1;
	
	return 0;
}

void ClearLB(HWND hwndLB)
{
	while(ListBox_GetCount(hwndLB) > 0)
		ListBox_DeleteString(hwndLB, 0);
}

void ReloadEventLB(HWND hwndLB, const FilterOptions &sel)
{
	while(ListBox_GetCount(hwndLB) > 0)
		ListBox_DeleteString(hwndLB, 0);

	if (sel.onlyIncomming && !sel.onlyOutgoing)
		ListBox_AddString(hwndLB, TranslateT("Incoming events"));
	else if (sel.onlyOutgoing && !sel.onlyIncomming)
		ListBox_AddString(hwndLB, TranslateT("Outgoing events"));

	for (std::vector<int>::const_iterator it = sel.events.begin(); it != sel.events.end(); ++it) {
		int cpCount = SIZEOF(EventNames);
		int selCpIdx = -1;
		for (int i = 0; i < cpCount; ++i)
			if (EventNames[i].id == *it)
				selCpIdx = i;

		if (selCpIdx == -1) {
			TCHAR buf[24];
			mir_sntprintf(buf, SIZEOF(buf), _T("%d"), *it);
			ListBox_AddString(hwndLB, buf);	
		}
		else ListBox_AddString(hwndLB, TranslateTS(EventNames[selCpIdx].name));	
	}
}

bool CheckFile(HWND hwndEdit)
{
	TCHAR buf[MAX_PATH];
	Edit_GetText(hwndEdit, buf, MAX_PATH);
	DWORD atr = GetFileAttributes(buf);
	if (atr == INVALID_FILE_ATTRIBUTES || atr & FILE_ATTRIBUTE_DIRECTORY) {
		MessageBox(GetParent(hwndEdit), TranslateT("File does not exist. Enter correct file path."), TranslateT("Invalid file"), MB_OK | MB_ICONERROR);
		SetFocus(hwndEdit);
		return false;
	}

	return true;
}

bool OpenFileDlg(HWND hwndDlg, HWND hwndEdit, const TCHAR* defName, const TCHAR* ext, const TCHAR* title, bool open)
{
	TCHAR filter[1024];
	std::locale loc;
	TCHAR extUpper[32];
	_tcscpy_s(extUpper, ext);
	extUpper[0] = std::toupper(ext[0], loc);
	mir_sntprintf(filter, SIZEOF(filter), TranslateT("%s Files (*.%s)"), extUpper, ext);
	size_t len = mir_tstrlen(filter) + 1;
	mir_sntprintf(filter + len, SIZEOF(filter) - len, _T("*.%s"), ext);
	len += mir_tstrlen(filter + len) + 1;
	_tcscpy_s(filter + len, 1024 - len, TranslateT("All Files (*.*)"));
	len += mir_tstrlen(filter + len) + 1;
	_tcscpy_s(filter + len, 1024 - len, _T("*.*"));
	len += mir_tstrlen(filter + len) + 1;
	filter[len] = 0;
	TCHAR stzFilePath[1024];

	Edit_GetText(hwndEdit, stzFilePath, 1023);
	if (stzFilePath[0] == 0) {
		_tcscpy_s(stzFilePath, defName);
		len = mir_tstrlen(stzFilePath) + 1;
		stzFilePath[len] = 0;
	}
	else {
		len = mir_tstrlen(stzFilePath) + 1;
		stzFilePath[len] = 0;
	}

	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwndDlg;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = stzFilePath;
	ofn.lpstrTitle = title;
	ofn.nMaxFile = SIZEOF(stzFilePath);
	ofn.lpstrDefExt = ext;
	if (open) {
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		if (GetOpenFileName(&ofn)) {
			Edit_SetText(hwndEdit, stzFilePath);
			return true;
		}
	}
	else {
		ofn.Flags = OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
		if (GetSaveFileName(&ofn)) {
			Edit_SetText(hwndEdit, stzFilePath);
			return true;
		}
	}

	return false;
}

INT_PTR CALLBACK Options::DlgProcOptsMain(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)FALSE);
			CheckDlgButton(hwndDlg, IDC_SHOWCONTACTS, instance->showContacts ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWCONTACTGROUPS, instance->showContactGroups ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FINDNOBORDER, instance->noFindBorder ? BST_CHECKED : BST_UNCHECKED);
			HWND events = GetDlgItem(hwndDlg, IDC_EVENT);
			HWND defFilter = GetDlgItem(hwndDlg, IDC_DEFFILTER);
			HWND listFilter = GetDlgItem(hwndDlg, IDC_LIST_FILTERS);
			HWND ftp = GetDlgItem(hwndDlg, IDC_WINSCP);
			HWND ftpLog = GetDlgItem(hwndDlg, IDC_WINSCPLOG);
			ComboBox_AddString(events, TranslateT("Incoming events"));
			ComboBox_AddString(events, TranslateT("Outgoing events"));
			for (int i = 0 ; i < SIZEOF(EventNames); ++i)
				ComboBox_AddString(events, TranslateTS(EventNames[i].name));

			ComboBox_AddString(defFilter, TranslateT("Default history events"));
			ComboBox_AddString(defFilter, TranslateT("All events"));
			Edit_LimitText(GetDlgItem(hwndDlg, IDC_FILTER_NAME), 20);
			ComboBox_LimitText(events, 20);

			instance->customFiltersTemp.clear();
			instance->customFiltersTemp.insert(instance->customFiltersTemp.begin(), instance->customFilters.begin(), instance->customFilters.end());
			for (std::vector<FilterOptions>::iterator it = instance->customFiltersTemp.begin(); it != instance->customFiltersTemp.end(); ++it) {
				ComboBox_AddString(defFilter, it->name.c_str());
				ListBox_AddString(listFilter, it->name.c_str());
			}
			ComboBox_SetCurSel(defFilter, instance->defFilter);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_FILTER), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EVENT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_EVENT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_EVENT), FALSE);

			Edit_LimitText(ftp, MAX_PATH);
			Edit_LimitText(ftpLog, MAX_PATH);
			Edit_SetText(ftp, instance->ftpExePath.c_str());
			Edit_SetText(ftpLog, instance->ftpLogPath.c_str());
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)TRUE);
		}
		return TRUE;

	case WM_COMMAND:
		{
			BOOL init = (BOOL)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (HIWORD(wParam) == BN_CLICKED) {
				HWND listFilter = GetDlgItem(hwndDlg, IDC_LIST_FILTERS);
				HWND listEvents = GetDlgItem(hwndDlg, IDC_LIST_EVENTS);
				HWND nameFilter = GetDlgItem(hwndDlg, IDC_FILTER_NAME);
				HWND defFilter = GetDlgItem(hwndDlg, IDC_DEFFILTER);
				HWND eventCB = GetDlgItem(hwndDlg, IDC_EVENT);
				switch(LOWORD(wParam)) {
				case IDC_ADD_FILTER:
					{
						TCHAR name[24];
						Edit_GetText(nameFilter, name, 24);
						if (name[0] == 0) {
							MessageBox(hwndDlg, TranslateT("Enter filter name"), TranslateT("Error"), MB_ICONERROR);
							return TRUE;
						}

						FilterOptions fo(name);
						for (std::vector<FilterOptions>::iterator it = instance->customFiltersTemp.begin(); it != instance->customFiltersTemp.end(); ++it) {
							if (it->name == fo.name) {
								MessageBox(hwndDlg, TranslateT("Filter name exists"), TranslateT("Error"), MB_ICONERROR);
								return TRUE;
							}
						}

						instance->customFiltersTemp.insert(instance->customFiltersTemp.end(), fo);
						int i = ListBox_AddString(listFilter, name);
						ListBox_SetCurSel(listFilter, i);
						ComboBox_AddString(defFilter, name);
						name[0] = 0;
						Edit_SetText(nameFilter, name);
						ReloadEventLB(listEvents, instance->customFiltersTemp[i]);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_FILTER), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_EVENT), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_EVENT), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_EVENT), FALSE);
					}
					break;

				case IDC_DELETE_FILTER:
					{
						int sel = ListBox_GetCurSel(listFilter);
						if (sel < 0)
							return TRUE;

						for (size_t i = sel; i < instance->customFiltersTemp.size() - 1; ++i)
							instance->customFiltersTemp[i] = instance->customFiltersTemp[i + 1];

						instance->customFiltersTemp.resize(instance->customFiltersTemp.size() - 1);
						ListBox_DeleteString(listFilter, sel);
						ComboBox_DeleteString(defFilter, sel + 2);
						if (ComboBox_GetCurSel(defFilter) < 0)
							ComboBox_SetCurSel(defFilter, 0);

						ClearLB(listEvents);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_FILTER), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_EVENT), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_EVENT), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_EVENT), FALSE);
					}
					break;

				case IDC_ADD_EVENT:
					{
						int sel = ListBox_GetCurSel(listFilter);
						if (sel < 0)
							return TRUE;

						int eventId;
						int selCB = GetEventCB(eventCB, true, eventId);
						if (selCB < 0)
							return TRUE;

						if (selCB == 1) {
							if (instance->customFiltersTemp[sel].onlyIncomming) {
								MessageBox(hwndDlg, TranslateT("Event already exists"), TranslateT("Error"), MB_ICONERROR);
								return TRUE;
							}

							if (instance->customFiltersTemp[sel].onlyOutgoing)
								instance->customFiltersTemp[sel].onlyOutgoing = false;
							else
								instance->customFiltersTemp[sel].onlyIncomming = true;
						}
						else if (selCB == 2) {
							if (instance->customFiltersTemp[sel].onlyOutgoing) {
								MessageBox(hwndDlg, TranslateT("Event already exists"), TranslateT("Error"), MB_ICONERROR);
								return TRUE;
							}

							if (instance->customFiltersTemp[sel].onlyIncomming)
								instance->customFiltersTemp[sel].onlyIncomming = false;
							else
								instance->customFiltersTemp[sel].onlyOutgoing = true;
						}
						else {
							if (std::find(instance->customFiltersTemp[sel].events.begin(), instance->customFiltersTemp[sel].events.end(), eventId) != instance->customFiltersTemp[sel].events.end()) {
								MessageBox(hwndDlg, TranslateT("Event already exists"), TranslateT("Error"), MB_ICONERROR);
								return TRUE;
							}

							instance->customFiltersTemp[sel].events.push_back(eventId);
						}

						ReloadEventLB(listEvents, instance->customFiltersTemp[sel]);
						ComboBox_SetCurSel(eventCB, -1);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_EVENT), FALSE);
					}
					break;

				case IDC_DELETE_EVENT:
					{
						int sel = ListBox_GetCurSel(listFilter);
						if (sel < 0)
							return TRUE;

						int eventSel = ListBox_GetCurSel(listEvents);
						if (eventSel < 0)
							return TRUE;

						int stId = 0;
						if (instance->customFiltersTemp[sel].onlyIncomming || instance->customFiltersTemp[sel].onlyOutgoing)
							++stId;
						if (eventSel >= stId) {
							--eventSel;
							for (int i = eventSel; i < (int)instance->customFiltersTemp[sel].events.size() - 1; ++i)
								instance->customFiltersTemp[sel].events[i] = instance->customFiltersTemp[sel].events[i + 1];

							instance->customFiltersTemp[sel].events.resize(instance->customFiltersTemp[sel].events.size() - 1);
						}
						else {
							instance->customFiltersTemp[sel].onlyIncomming = false;
							instance->customFiltersTemp[sel].onlyOutgoing = false;
						}

						ReloadEventLB(listEvents, instance->customFiltersTemp[sel]);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_EVENT), FALSE);
					}
					break;

				case IDC_WINSCP_BROWSE:
					if (!OpenFileDlg(hwndDlg, GetDlgItem(hwndDlg, IDC_WINSCP), _T("WinSCP.exe"), _T("exe"), TranslateT("Browse WinSCP file"), true))
						return TRUE;
					break;

				case IDC_WINSCPLOG_BROWSE:
					if (!OpenFileDlg(hwndDlg, GetDlgItem(hwndDlg, IDC_WINSCPLOG), _T("ftplog.txt"), _T("txt"), TranslateT("Save WinSCP log file"), false))
						return TRUE;
					break;
				}
			}
			else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_LIST_FILTERS) {
				HWND listFilter = GetDlgItem(hwndDlg, IDC_LIST_FILTERS);
				HWND listEvents = GetDlgItem(hwndDlg, IDC_LIST_EVENTS);
				int sel = ListBox_GetCurSel(listFilter);
				if (sel < 0)
					ClearLB(listEvents);
				else
					ReloadEventLB(listEvents, instance->customFiltersTemp[sel]);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_FILTER), sel >= 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EVENT), sel >= 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_EVENT), sel >= 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_EVENT), FALSE);
			}
			else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_LIST_EVENTS) {
				HWND listEvents = GetDlgItem(hwndDlg, IDC_LIST_EVENTS);
				int sel = ListBox_GetCurSel(listEvents);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_EVENT), sel >= 0);
			}

			if (init && (HIWORD(wParam) == BN_CLICKED || (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_DEFFILTER) || (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) != IDC_FILTER_NAME)))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			HWND ftp = GetDlgItem(hwndDlg, IDC_WINSCP);
			TCHAR buf[MAX_PATH];
			Edit_GetText(ftp, buf, MAX_PATH);
			if (buf[0] != 0 && !CheckFile(ftp)) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
				return TRUE;
			}

			instance->ftpExePath = buf;
			Edit_GetText(GetDlgItem(hwndDlg, IDC_WINSCPLOG), buf, MAX_PATH);
			instance->ftpLogPath = buf;

			instance->showContacts = IsDlgButtonChecked(hwndDlg, IDC_SHOWCONTACTS) ? true : false;
			instance->showContactGroups = IsDlgButtonChecked(hwndDlg, IDC_SHOWCONTACTGROUPS) ? true : false;
			instance->noFindBorder = IsDlgButtonChecked(hwndDlg, IDC_FINDNOBORDER) ? true : false;
			instance->defFilter = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_DEFFILTER));
			instance->customFilters.clear();
			instance->customFilters.insert(instance->customFilters.begin(), instance->customFiltersTemp.begin(), instance->customFiltersTemp.end());
			Options::instance->Save();
			OptionsMainChanged();
		}
		return TRUE;
	}

	return FALSE;
}

class OptsData
{
public:
	OptsData()
	{
		init = false;
	}

	bool init;
};

INT_PTR CALLBACK Options::DlgProcOptsGroupList(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			OptsData* optsData = new OptsData();
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)optsData);

			CheckDlgButton(hwndDlg, IDC_NEWONTOP, instance->groupNewOnTop ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWEVENTS, instance->groupShowEvents ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWTIME, instance->groupShowTime ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWNAME, instance->groupShowName ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWMESSAGE, instance->groupShowMessage ? BST_CHECKED : BST_UNCHECKED);
			Edit_LimitText(GetDlgItem(hwndDlg, IDC_MESSAGELEN), 4);
			SetDlgItemInt(hwndDlg, IDC_MESSAGELEN, instance->groupMessageLen, FALSE);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_SHOWMESSAGE, BN_CLICKED), NULL);
			Edit_LimitText(GetDlgItem(hwndDlg, IDC_GROUPTIME), 2);
			Edit_LimitText(GetDlgItem(hwndDlg, IDC_LIMITMESSAGES), 3);
			SetDlgItemInt(hwndDlg, IDC_GROUPTIME, instance->groupTime, FALSE);
			SetDlgItemInt(hwndDlg, IDC_LIMITMESSAGES, instance->groupMessagesNumber, FALSE);

			optsData->init = true;
		}
		return TRUE;

	case WM_COMMAND:
		{
			OptsData* optsData = (OptsData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_SHOWMESSAGE) {
				if (IsDlgButtonChecked(hwndDlg, IDC_SHOWMESSAGE)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGELEN), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGELEN_DESC), TRUE);
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGELEN), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGELEN_DESC), FALSE);
				}
			}
			if (optsData->init && (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			instance->groupNewOnTop = IsDlgButtonChecked(hwndDlg, IDC_NEWONTOP) ? true : false;
			instance->groupShowEvents = IsDlgButtonChecked(hwndDlg, IDC_SHOWEVENTS) ? true : false;
			instance->groupShowTime = IsDlgButtonChecked(hwndDlg, IDC_SHOWTIME) ? true : false;
			instance->groupShowName = IsDlgButtonChecked(hwndDlg, IDC_SHOWNAME) ? true : false;
			instance->groupShowMessage = IsDlgButtonChecked(hwndDlg, IDC_SHOWMESSAGE) ? true : false;
			BOOL success;
			instance->groupMessageLen = GetDlgItemInt(hwndDlg, IDC_MESSAGELEN, &success, FALSE);
			instance->groupTime = GetDlgItemInt(hwndDlg, IDC_GROUPTIME, &success, FALSE);
			instance->groupMessagesNumber = GetDlgItemInt(hwndDlg, IDC_LIMITMESSAGES, &success, FALSE);

			Options::instance->Save();
			OptionsGroupChanged();
		}
		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK Options::DlgProcOptsMessages(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_NEWONTOP, instance->messagesNewOnTop ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWDATE, instance->messagesShowDate ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWSECOND, instance->messagesShowSec ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWNAME, instance->messagesShowName ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWEVENTS, instance->messagesShowEvents ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWSMILEYS, instance->messagesUseSmileys ? BST_CHECKED : BST_UNCHECKED);
		if (!g_SmileyAddAvail)
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSMILEYS), FALSE);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			instance->messagesNewOnTop = IsDlgButtonChecked(hwndDlg, IDC_NEWONTOP) ? true : false;
			instance->messagesShowDate = IsDlgButtonChecked(hwndDlg, IDC_SHOWDATE) ? true : false;
			instance->messagesShowSec = IsDlgButtonChecked(hwndDlg, IDC_SHOWSECOND) ? true : false;
			instance->messagesShowName = IsDlgButtonChecked(hwndDlg, IDC_SHOWNAME) ? true : false;
			instance->messagesShowEvents = IsDlgButtonChecked(hwndDlg, IDC_SHOWEVENTS) ? true : false;
			instance->messagesUseSmileys = IsDlgButtonChecked(hwndDlg, IDC_SHOWSMILEYS) ? true : false;

			Options::instance->Save();
			OptionsMessageChanged();
		}
		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK Options::DlgProcOptsSearching(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_FORLIST, instance->searchForInList ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_FORMES, instance->searchForInMess ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MATCHCASE, instance->searchMatchCase ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MATCHWHOLE, instance->searchMatchWhole ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONLYIN, instance->searchOnlyIn ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONLYOUT, instance->searchOnlyOut ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONLYGROUP, instance->searchOnlyGroup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALLCONTACTS, instance->searchAllContacts ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ONLYIN) {
			if (IsDlgButtonChecked(hwndDlg, IDC_ONLYIN) && IsDlgButtonChecked(hwndDlg, IDC_ONLYOUT))
				CheckDlgButton(hwndDlg, IDC_ONLYOUT, BST_UNCHECKED);
		}
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ONLYOUT) {
			if (IsDlgButtonChecked(hwndDlg, IDC_ONLYOUT) && IsDlgButtonChecked(hwndDlg, IDC_ONLYIN))
				CheckDlgButton(hwndDlg, IDC_ONLYIN, BST_UNCHECKED);
		}

		if (HIWORD(wParam) == BN_CLICKED)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			instance->searchForInList = IsDlgButtonChecked(hwndDlg, IDC_FORLIST) ? true : false;
			instance->searchForInMess = IsDlgButtonChecked(hwndDlg, IDC_FORMES) ? true : false;
			instance->searchMatchCase = IsDlgButtonChecked(hwndDlg, IDC_MATCHCASE) ? true : false;
			instance->searchMatchWhole = IsDlgButtonChecked(hwndDlg, IDC_MATCHWHOLE) ? true : false;
			instance->searchOnlyIn = IsDlgButtonChecked(hwndDlg, IDC_ONLYIN) ? true : false;
			instance->searchOnlyOut = IsDlgButtonChecked(hwndDlg, IDC_ONLYOUT) ? true : false;
			instance->searchOnlyGroup = IsDlgButtonChecked(hwndDlg, IDC_ONLYGROUP) ? true : false;
			instance->searchAllContacts = IsDlgButtonChecked(hwndDlg, IDC_ALLCONTACTS) ? true : false;

			Options::instance->Save();
			OptionsSearchingChanged();
		}
		return TRUE;
	}

	return FALSE;
}

void InitCodepageCB(HWND hwndCB, unsigned int codepage, const std::wstring& name)
{
	int cpCount = sizeof(cpTable) / sizeof(cpTable[0]);
	int selCpIdx = -1;
	ComboBox_LimitText(hwndCB, 256);
	for (int i = 0; i < cpCount; ++i) {
		ComboBox_AddString(hwndCB, TranslateTS(cpTable[i].cpName));
		if (cpTable[i].cpId == codepage && name == cpTable[i].cpName)
			selCpIdx = i;
	}

	if (selCpIdx == -1) {
		TCHAR buf[300];
		mir_sntprintf(buf, SIZEOF(buf), _T("%d;%s"), codepage, name.c_str());
		ComboBox_SetText(hwndCB, buf);	
	}
	else ComboBox_SetCurSel(hwndCB, selCpIdx);	

	ComboBox_LimitText(hwndCB, 127);
}

unsigned int GetCodepageCB(HWND hwndCB, bool errorReport, unsigned int defCp, const std::wstring& defName, std::wstring& name)
{
	int selCpIdx = ComboBox_GetCurSel(hwndCB);
	if (selCpIdx < 0) {
		TCHAR text[128];
		ComboBox_GetText(hwndCB, text, 128);
		std::wstring str = text;
		name = _T("");
		size_t pos = str.find_first_of(_T(';'));
		if (pos < str.length()) {
			text[pos] = 0;
			name = str.substr(pos + 1);
		}

		TCHAR * stopOn = NULL;
		long cp = _tcstol(text, &stopOn, 10);
		if ((pos >= str.length() || name.empty() || stopOn == text || *stopOn != '\0' || cp < 0 || cp > 0xffff)) {
			if (errorReport) {
				MessageBox(GetParent(hwndCB), TranslateT("You've entered invalid codepage. Select codepage from combo box or enter correct number."), TranslateT("Invalid codepage"), MB_OK | MB_ICONERROR);
				SetFocus(hwndCB);
			}

			name = defName;
			return -1;
		}

		return cp;
	}
	else {
		name = cpTable[selCpIdx].cpName;
		return cpTable[selCpIdx].cpId;
	}
}

INT_PTR CALLBACK Options::DlgProcOptsExport(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)FALSE);
		InitCodepageCB(GetDlgItem(hwndDlg, IDC_TXTENC), instance->codepageTxt, instance->encodingTxt);
		InitCodepageCB(GetDlgItem(hwndDlg, IDC_HTML1ENC), instance->codepageHtml1, instance->encodingHtml1);
		InitCodepageCB(GetDlgItem(hwndDlg, IDC_HTML2ENC), instance->codepageHtml2, instance->encodingHtml2);
		CheckDlgButton(hwndDlg, IDC_HTML1DATE, instance->exportHtml1ShowDate ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HTML2DATE, instance->exportHtml2ShowDate ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HTML2SHOWSMILEYS, instance->exportHtml2UseSmileys ? BST_CHECKED : BST_UNCHECKED);
		Edit_LimitText(GetDlgItem(hwndDlg, IDC_HTML2EXTCSSFILE), MAX_PATH);
		if (instance->extCssHtml2.empty()) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_HTML2EXTCSSFILE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CSS_BROWSE), FALSE);
		}
		else {
			CheckDlgButton(hwndDlg, IDC_HTML2EXTCSS, BST_CHECKED);
			Edit_SetText(GetDlgItem(hwndDlg, IDC_HTML2EXTCSSFILE), instance->extCssHtml2.c_str());
		}

		if (!g_SmileyAddAvail)
			EnableWindow(GetDlgItem(hwndDlg, IDC_HTML2SHOWSMILEYS), FALSE);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)TRUE);

		return TRUE;

	case WM_COMMAND:
		{
			BOOL init = (BOOL)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_HTML2EXTCSS) {
				BOOL en = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_HTML2EXTCSS);
				EnableWindow(GetDlgItem(hwndDlg, IDC_HTML2EXTCSSFILE), en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CSS_BROWSE), en);
			}
			else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_CSS_BROWSE) {
				if (!OpenFileDlg(hwndDlg, GetDlgItem(hwndDlg, IDC_HTML2EXTCSSFILE), _T(""), _T("css"), TranslateT("Browse CSS file"), true))
					return TRUE;
			}

			if (init && (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam)==CBN_SELCHANGE || HIWORD(wParam)==CBN_EDITCHANGE || HIWORD(wParam) == EN_CHANGE))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			std::wstring newName1, newName2, newName3;
			unsigned int cp1 = GetCodepageCB(GetDlgItem(hwndDlg, IDC_TXTENC), true, instance->codepageTxt, instance->encodingTxt, newName1);
			if (cp1 == -1) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
				return TRUE;
			}
			unsigned int cp2 = GetCodepageCB(GetDlgItem(hwndDlg, IDC_HTML1ENC), true, instance->codepageHtml1, instance->encodingHtml1, newName2);
			if (cp2 == -1) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
				return TRUE;
			}
			unsigned int cp3 = GetCodepageCB(GetDlgItem(hwndDlg, IDC_HTML2ENC), true, instance->codepageHtml2, instance->encodingHtml2, newName3);
			if (cp3 == -1) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
				return TRUE;
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_HTML2EXTCSS)) {
				if (!CheckFile(GetDlgItem(hwndDlg, IDC_HTML2EXTCSSFILE))) {
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
					return TRUE;
				}

				TCHAR buf[MAX_PATH];
				Edit_GetText(GetDlgItem(hwndDlg, IDC_HTML2EXTCSSFILE), buf, MAX_PATH);
				instance->extCssHtml2 = buf;
			}
			else instance->extCssHtml2 = _T("");

			instance->codepageTxt = cp1;
			instance->encodingTxt = newName1;
			instance->codepageHtml1 = cp2;
			instance->encodingHtml1 = newName2;
			instance->codepageHtml2 = cp3;
			instance->encodingHtml2 = newName3;
			instance->exportHtml1ShowDate = IsDlgButtonChecked(hwndDlg, IDC_HTML1DATE) ? true : false;
			instance->exportHtml2ShowDate = IsDlgButtonChecked(hwndDlg, IDC_HTML2DATE) ? true : false;
			instance->exportHtml2UseSmileys = IsDlgButtonChecked(hwndDlg, IDC_HTML2SHOWSMILEYS) ? true : false;

			Options::instance->Save();
		}
		return TRUE;
	}

	return FALSE;
}


struct DlgTaskOpt
{
	std::list<TaskOptions>* tasks;
	TaskOptions* to;
};

INT_PTR CALLBACK Options::DlgProcOptsScheduler(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	std::list<TaskOptions>* tasks = (std::list<TaskOptions>*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			tasks = new std::list<TaskOptions>(Options::instance->taskOptions.begin(), Options::instance->taskOptions.end());
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)tasks);
			HWND listTasks = GetDlgItem(hwndDlg, IDC_LIST_TASKS);
			for (std::list<TaskOptions>::iterator it = tasks->begin(); it != tasks->end(); ++it)
				ListBox_AddString(listTasks, it->taskName.c_str());

			if (!bPopupsEnabled)
				EnableWindow(GetDlgItem(hwndDlg, IDC_SCHEDULER_ALERTS), FALSE);

			CheckDlgButton(hwndDlg, IDC_SCHEDULER_ALERTS, instance->schedulerAlerts ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SCHEDULER_HISTORY_ALERTS, instance->schedulerHistoryAlerts ? BST_CHECKED : BST_UNCHECKED);

			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_TASK), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_TASK), FALSE);
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			HWND listTasks = GetDlgItem(hwndDlg, IDC_LIST_TASKS);
			int sel = ListBox_GetCurSel(listTasks);
			TaskOptions toAdd;
			TaskOptions* to = &toAdd;
			switch(LOWORD(wParam)) {
			case IDC_EDIT_TASK:
				if (sel >= 0) {
					std::list<TaskOptions>::iterator it = tasks->begin();
					while(sel-- > 0 && it != tasks->end())
						++it;
					if (it == tasks->end())
						break;
					to = &(*it);
				}
				else break;

			case IDC_ADD_TASK:
				{
					DlgTaskOpt top;
					top.tasks = tasks;
					top.to = to;
					if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DLG_TASK), hwndDlg, DlgProcOptsTask, (LPARAM)&top) == IDOK) {
						if (LOWORD(wParam) == IDC_ADD_TASK) {
							tasks->push_back(*to); 
							ListBox_AddString(listTasks, to->taskName.c_str());
							ListBox_SetCurSel(listTasks, tasks->size() - 1);
						}
						else {
							sel = ListBox_GetCurSel(listTasks);
							ListBox_DeleteString(listTasks, sel);
							ListBox_InsertString(listTasks, sel, to->taskName.c_str());
							ListBox_SetCurSel(listTasks, sel);
						}
						EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_TASK), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_TASK), TRUE);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
				break;

			case IDC_DELETE_TASK:
				if (sel >= 0) {
					ListBox_DeleteString(listTasks, sel);
					std::list<TaskOptions>::iterator it = tasks->begin();
					while(sel-- > 0 && it != tasks->end())
						++it;
					if (it != tasks->end())
						tasks->erase(it);
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_TASK), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_TASK), FALSE);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;

			default:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_LIST_TASKS) {
			HWND listTasks = GetDlgItem(hwndDlg, IDC_LIST_TASKS);
			int sel = ListBox_GetCurSel(listTasks);
			if (sel < 0) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_TASK), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_TASK), FALSE);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_TASK), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_TASK), TRUE);
			}
		}
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			instance->schedulerAlerts = IsDlgButtonChecked(hwndDlg, IDC_SCHEDULER_ALERTS) ? true : false;
			instance->schedulerHistoryAlerts = IsDlgButtonChecked(hwndDlg, IDC_SCHEDULER_HISTORY_ALERTS) ? true : false;
			Options::instance->SaveTasks(tasks);
			OptionsSchedulerChanged();
			InitTaskMenuItems();
		}
		return TRUE;

	case WM_DESTROY:
		delete tasks;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}

	return FALSE;
}

void RebuildList(HWND hwnd, MCONTACT hSystem, TaskOptions* to)
{
	HANDLE hItem;
	if (to->isSystem && hSystem)
		SendMessage(hwnd, CLM_SETCHECKMARK, (WPARAM) hSystem, 1);

	for (size_t i = 0; i < to->contacts.size(); ++i) {
		hItem = (HANDLE) SendMessage(hwnd, CLM_FINDCONTACT, (WPARAM) to->contacts[i], 0);
		if (hItem)
			SendMessage(hwnd, CLM_SETCHECKMARK, (WPARAM) hItem, 1);
	}
}

void SaveList(HWND hwnd, MCONTACT hSystem, TaskOptions* to)
{
	to->contacts.clear();
	if (hSystem)
		to->isSystem = SendMessage(hwnd, CLM_GETCHECKMARK, (WPARAM)hSystem, 0) != 0;

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE) SendMessage(hwnd, CLM_FINDCONTACT, hContact, 0);
		if (hItem && SendMessage(hwnd, CLM_GETCHECKMARK, (WPARAM) hItem, 0))
			to->contacts.push_back(hContact);
	} 
}

bool IsValidTask(TaskOptions& to, std::list<TaskOptions>* top = NULL, std::wstring* err = NULL, std::wstring* errDescr = NULL);

#ifndef LOCALE_SSHORTTIME
#define LOCALE_SSHORTTIME             0x00000079
#endif

INT_PTR CALLBACK Options::DlgProcOptsTask(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static MCONTACT hSystem;
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			DlgTaskOpt* top = (DlgTaskOpt*)lParam;
			TaskOptions* to = top->to;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			HWND comboType = GetDlgItem(hwndDlg, IDC_TASK_TYPE);
			HWND filter = GetDlgItem(hwndDlg, IDC_TASK_FILTER);
			HWND eventUnit = GetDlgItem(hwndDlg, IDC_EVENT_UNIT);
			HWND trigerType = GetDlgItem(hwndDlg, IDC_TRIGER_TYPE);
			HWND exportType = GetDlgItem(hwndDlg, IDC_EXPORT_TYPE);
			HWND importType = GetDlgItem(hwndDlg, IDC_IMPORT_TYPE);
			HWND compress = GetDlgItem(hwndDlg, IDC_COMPRESS);
			HWND exportPath = GetDlgItem(hwndDlg, IDC_EXPORT_PATH);
			HWND ftpFile = GetDlgItem(hwndDlg, IDC_FTP);
			HWND ftpFileButton = GetDlgItem(hwndDlg, IDC_UPLOAD);
			HWND contactList = GetDlgItem(hwndDlg, IDC_LIST_CONTACTSEX);
			HWND weekList = GetDlgItem(hwndDlg, IDC_TRIGER_WEEK);
			HWND day = GetDlgItem(hwndDlg, IDC_TRIGER_DAY);
			HWND deltaTime = GetDlgItem(hwndDlg, IDC_TRIGER_DELTA_TIME);
			HWND time = GetDlgItem(hwndDlg, IDC_TRIGER_TIME);
			HWND name = GetDlgItem(hwndDlg, IDC_TASK_NAME);
			HWND active = GetDlgItem(hwndDlg, IDC_TASK_ACTIVE);
			HWND star = GetDlgItem(hwndDlg, IDC_TASK_STAR);
			HWND password = GetDlgItem(hwndDlg, IDC_PASSWORD);
			HWND expImp = GetDlgItem(hwndDlg, IDC_EXPIMP);

			Edit_LimitText(name, 16);
			Edit_SetText(name, to->taskName.c_str());

			Button_SetCheck(active, to->active);

			Button_SetCheck(expImp, to->exportImported);

			ComboBox_AddString(comboType, TranslateT("Export"));
			ComboBox_AddString(comboType, TranslateT("Delete"));
			ComboBox_AddString(comboType, TranslateT("Export and Delete"));
			ComboBox_AddString(comboType, TranslateT("Import"));
			ComboBox_AddString(comboType, TranslateT("Import and Marge"));
			ComboBox_SetCurSel(comboType, to->type);

			Edit_LimitText(GetDlgItem(hwndDlg, IDC_EVENT_TIME), 6);
			SetDlgItemInt(hwndDlg, IDC_EVENT_TIME, to->eventDeltaTime, TRUE);
			ComboBox_AddString(eventUnit, TranslateT("Minute"));
			ComboBox_AddString(eventUnit, TranslateT("Hour"));
			ComboBox_AddString(eventUnit, TranslateT("Day"));
			ComboBox_SetCurSel(eventUnit, to->eventUnit);

			ComboBox_AddString(filter, TranslateT("Default history events"));
			ComboBox_AddString(filter, TranslateT("All events"));
			int selFilter = to->filterId;
			if (selFilter > 1)
				selFilter = 0;
			int i = 1;
			for (std::vector<FilterOptions>::iterator it = instance->customFilters.begin(); it != instance->customFilters.end(); ++it) {
				++i;
				ComboBox_AddString(filter, it->name.c_str());
				if (to->filterId > 1 && it->name == to->filterName)
					selFilter = i;
			}
			ComboBox_SetCurSel(filter, selFilter);

			ComboBox_AddString(trigerType, TranslateT("At Start"));
			ComboBox_AddString(trigerType, TranslateT("At Finish"));
			ComboBox_AddString(trigerType, TranslateT("Daily"));
			ComboBox_AddString(trigerType, TranslateT("Weekly"));
			ComboBox_AddString(trigerType, TranslateT("Monthly"));
			ComboBox_AddString(trigerType, TranslateT("Delta time (minutes)"));
			ComboBox_AddString(trigerType, TranslateT("Delta time (hours)"));
			ComboBox_SetCurSel(trigerType, to->trigerType);

			ComboBox_AddString(exportType, TranslateT("Rich Html"));
			ComboBox_AddString(exportType, TranslateT("Plain Html"));
			ComboBox_AddString(exportType, TranslateT("Txt"));
			ComboBox_AddString(exportType, TranslateT("Binary"));
			ComboBox_AddString(exportType, TranslateT("Dat (mContacts)"));
			ComboBox_SetCurSel(exportType, to->exportType);

			ComboBox_AddString(importType, TranslateT("Binary"));
			ComboBox_AddString(importType, TranslateT("Dat (mContacts)"));
			ComboBox_SetCurSel(importType, to->importType);

			Button_SetCheck(compress, to->compress);
			Edit_LimitText(password, 99);
			SetWindowTextA(password, to->zipPassword.c_str());

			Edit_LimitText(exportPath, MAX_PATH);
			Edit_SetText(exportPath, to->filePath.c_str());

			if (!FTPAvail()) {
				EnableWindow(ftpFile, FALSE);
				EnableWindow(ftpFileButton, FALSE);
				to->useFtp = false;
			}

			Button_SetCheck(ftpFileButton, to->useFtp);
			Edit_SetText(ftpFile, to->ftpName.c_str());

			ComboBox_AddString(weekList, TranslateT("Monday"));
			ComboBox_AddString(weekList, TranslateT("Tuesday"));
			ComboBox_AddString(weekList, TranslateT("Wednesday"));
			ComboBox_AddString(weekList, TranslateT("Thursday"));
			ComboBox_AddString(weekList, TranslateT("Friday"));
			ComboBox_AddString(weekList, TranslateT("Saturday"));
			ComboBox_AddString(weekList, TranslateT("Sunday"));
			ComboBox_SetCurSel(weekList, to->dayOfWeek);

			Edit_LimitText(day, 2);
			SetDlgItemInt(hwndDlg, IDC_TRIGER_DAY, to->dayOfMonth, FALSE);

			Edit_LimitText(deltaTime, 4);
			SetDlgItemInt(hwndDlg, IDC_TRIGER_DELTA_TIME, to->deltaTime, FALSE);

			TCHAR timeFormat[10];
			if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTTIME, timeFormat, 10) == 0) {
				TCHAR sep = _T(':');
				if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, timeFormat, 10) > 0)
					sep = timeFormat[0];
				mir_sntprintf(timeFormat, SIZEOF(timeFormat), _T("HH%cmm"), sep);
			}

			SYSTEMTIME st;
			GetSystemTime (&st);
			st.wHour = to->dayTime/60;
			st.wMinute = to->dayTime%60;
			st.wSecond = 0;
			st.wMilliseconds = 0;
			DateTime_SetFormat(time, timeFormat);
			DateTime_SetSystemtime(time, GDT_VALID, &st);

			CLCINFOITEM cii = { 0 };
			cii.cbSize = sizeof(cii);
			cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX | CLCIIF_BELOWCONTACTS;
			cii.pszText = TranslateT("System");
			hSystem = (MCONTACT)SendMessage(contactList, CLM_ADDINFOITEM, 0, (LPARAM) & cii);
			SendMessage(contactList, CLM_AUTOREBUILD, 0, 0);
			RebuildList(contactList, hSystem, to);

			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_TASK_TYPE, CBN_SELCHANGE), NULL);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_TRIGER_TYPE, CBN_SELCHANGE), NULL);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_COMPRESS, BN_CLICKED), NULL);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_UPLOAD, BN_CLICKED), NULL);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_COMPRESS, BN_CLICKED), NULL);
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			if (LOWORD(wParam) == IDOK) {
				DlgTaskOpt* top = (DlgTaskOpt*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				TaskOptions* to = top->to;
				TaskOptions toCp(*to);
				toCp.taskName.resize(17);
				int nameLen = Edit_GetText(GetDlgItem(hwndDlg, IDC_TASK_NAME), (wchar_t*)toCp.taskName.c_str(), 17);
				toCp.taskName.resize(nameLen);
				toCp.active = Button_GetCheck(GetDlgItem(hwndDlg, IDC_TASK_ACTIVE)) != 0;
				toCp.exportImported = Button_GetCheck(GetDlgItem(hwndDlg, IDC_EXPIMP)) != 0;
				toCp.type = (enum TaskOptions::TaskType)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_TASK_TYPE));
				toCp.filterId = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_TASK_FILTER));
				if (toCp.filterId > 1)
					toCp.filterName = instance->customFilters[toCp.filterId - 2].name;
				BOOL isOK = FALSE;
				toCp.eventDeltaTime = GetDlgItemInt(hwndDlg, IDC_EVENT_TIME, &isOK, TRUE);
				if (!isOK) {
					TCHAR msg[256];
					mir_sntprintf(msg, SIZEOF(msg), TranslateT("Invalid '%s' value."), TranslateT("Events older than"));
					MessageBox(hwndDlg, msg, TranslateT("Error"), MB_ICONERROR);
					break;
				}
				toCp.eventUnit = (enum TaskOptions::EventUnit)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_EVENT_UNIT));
				toCp.trigerType = (enum TaskOptions::TrigerType)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_TRIGER_TYPE));
				toCp.exportType = (enum IExport::ExportType)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_EXPORT_TYPE));
				toCp.importType = (enum IImport::ImportType)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_IMPORT_TYPE));
				toCp.compress = Button_GetCheck(GetDlgItem(hwndDlg, IDC_COMPRESS)) != 0;
				char bufC[100];
				GetDlgItemTextA(hwndDlg, IDC_PASSWORD, bufC, SIZEOF(bufC));
				toCp.zipPassword = bufC;
				HWND exportPath = GetDlgItem(hwndDlg, IDC_EXPORT_PATH);
				int exLen = Edit_GetTextLength(exportPath);
				toCp.filePath.resize(exLen + 1);
				Edit_GetText(exportPath, (wchar_t*)toCp.filePath.c_str(), exLen + 1);
				toCp.filePath.resize(exLen);
				toCp.useFtp = Button_GetCheck(GetDlgItem(hwndDlg, IDC_UPLOAD)) != 0;
				HWND ftpFile = GetDlgItem(hwndDlg, IDC_FTP);
				exLen = Edit_GetTextLength(ftpFile);
				toCp.ftpName.resize(exLen + 1);
				Edit_GetText(ftpFile, (wchar_t*)toCp.ftpName.c_str(), exLen + 1);
				toCp.ftpName.resize(exLen);
				SYSTEMTIME st;
				DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_TRIGER_TIME), &st);
				toCp.dayTime = st.wHour * 60 + st.wMinute;
				toCp.dayOfWeek = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_TRIGER_WEEK));
				toCp.dayOfMonth = GetDlgItemInt(hwndDlg, IDC_TRIGER_DAY, &isOK, FALSE);
				if (!isOK) {
					if (toCp.trigerType == TaskOptions::Monthly) {
						TCHAR msg[256];
						mir_sntprintf(msg, SIZEOF(msg), TranslateT("Invalid '%s' value."), TranslateT("Day"));
						MessageBox(hwndDlg, msg, TranslateT("Error"), MB_ICONERROR);
						break;
					}
					else toCp.dayOfMonth = to->dayOfMonth;
				}
				toCp.deltaTime = GetDlgItemInt(hwndDlg, IDC_TRIGER_DELTA_TIME, &isOK, FALSE);
				if (!isOK) {
					if (toCp.trigerType == TaskOptions::DeltaMin || toCp.trigerType == TaskOptions::DeltaHour) {
						TCHAR msg[256];
						mir_sntprintf(msg, SIZEOF(msg), TranslateT("Invalid '%s' value."), TranslateT("Delta time"));
						MessageBox(hwndDlg, msg, TranslateT("Error"), MB_ICONERROR);
						break;
					}
					else toCp.deltaTime = to->deltaTime;
				}
				SaveList(GetDlgItem(hwndDlg, IDC_LIST_CONTACTSEX), hSystem, &toCp);
				std::wstring err;
				std::wstring errDescr;
				std::wstring lastName = to->taskName;
				to->taskName = L"";
				if (!IsValidTask(toCp, top->tasks, &err, &errDescr)) {
					to->taskName = lastName;
					TCHAR msg[256];
					if (err.empty())
						_tcscpy_s(msg, TranslateT("Some value is invalid"));
					else if (errDescr.empty())
						mir_sntprintf(msg, SIZEOF(msg), TranslateT("Invalid '%s' value."), err.c_str());
					else
						mir_sntprintf(msg, SIZEOF(msg), TranslateT("Invalid '%s' value.\n%s"), err.c_str(), errDescr.c_str());
					MessageBox(hwndDlg, msg, TranslateT("Error"), MB_ICONERROR);
					break;
				}

				toCp.lastExport = time(NULL);

				*to = toCp;
				EndDialog(hwndDlg, IDOK);
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hwndDlg, IDCANCEL);
			}
			else if (LOWORD(wParam) == IDC_UPLOAD) {
				if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_UPLOAD)) == 0)
					EnableWindow(GetDlgItem(hwndDlg, IDC_FTP), FALSE);
				else
					EnableWindow(GetDlgItem(hwndDlg, IDC_FTP), TRUE);
			}
			else if (LOWORD(wParam) == IDC_COMPRESS)
			{
				if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_COMPRESS)) == 0) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD_LABEL), FALSE);
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD_LABEL), TRUE);
				}
			}
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE) {
			if (LOWORD(wParam) == IDC_TASK_TYPE) {
				TaskOptions::TaskType sel = (enum TaskOptions::TaskType)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_TASK_TYPE));
				int show = sel == TaskOptions::Delete ? SW_HIDE : SW_SHOW;
				int showFilter = (sel == TaskOptions::Import || sel == TaskOptions::ImportAndMarge) ? SW_HIDE : SW_SHOW;
				int showImport = (sel == TaskOptions::Import || sel == TaskOptions::ImportAndMarge) ? SW_SHOW : SW_HIDE;
				ShowWindow(GetDlgItem(hwndDlg, IDC_EXPORT_TYPE), show);  
				ShowWindow(GetDlgItem(hwndDlg, IDC_EXPORT_TYPE_LABEL), show); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_COMPRESS), show); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), show); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_PASSWORD_LABEL), show); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_EXPORT_PATH), show); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_EXPORT_PATH_LABEL), show); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_FTP), show); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_UPLOAD), show); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_FTP_LABEL), show); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_EXPIMP), show);  
				ShowWindow(GetDlgItem(hwndDlg, IDC_TASK_FILTER), showFilter); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_TASK_FILTER_LABEL), showFilter); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_EVENT_TIME), showFilter); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_EVENT_UNIT), showFilter); 
				ShowWindow(GetDlgItem(hwndDlg, IDC_EVENT_LABEL), showFilter); 
				if (show != showFilter) {
					ShowWindow(GetDlgItem(hwndDlg, IDC_EXPORT_TYPE), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_EXPIMP), SW_HIDE);
				}

				ShowWindow(GetDlgItem(hwndDlg, IDC_IMPORT_TYPE), showImport);
				std::wstring str;
				TCHAR* compressText = TranslateT("Compress output files");
				TCHAR* uploadText = TranslateT("Upload to FTP (WinSCP requred)");
				TCHAR* typeText = TranslateT("Export to");
				if (showFilter == SW_HIDE) {
					str = TranslateT("** Use <ext> to insert extension, <contact> to insert contact name");
					compressText = TranslateT("Input files are compressed");
					uploadText = TranslateT("Download from FTP (WinSCP requred)");
					typeText = TranslateT("Import from");
				}
				else if (show == SW_HIDE)
					str = TranslateT("* Use negative values to filter younger events");
				else {
					str = TranslateT("* Use negative values to filter younger events");
					str += _T("\n");
					str += TranslateT("** Use <date> to insert date, <ext> to insert extension, <contact> to insert contact name");
				}

				Static_SetText(GetDlgItem(hwndDlg, IDC_TASK_STAR), str.c_str());
				Button_SetText(GetDlgItem(hwndDlg, IDC_COMPRESS), compressText);
				Button_SetText(GetDlgItem(hwndDlg, IDC_UPLOAD), uploadText);
				Static_SetText(GetDlgItem(hwndDlg, IDC_EXPORT_TYPE_LABEL), typeText);
			}
			else if (LOWORD(wParam) == IDC_TRIGER_TYPE) {
				TaskOptions::TrigerType sel = (enum TaskOptions::TrigerType)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_TRIGER_TYPE));
				int showT = (sel == TaskOptions::Daily || sel == TaskOptions::Weekly || sel == TaskOptions::Monthly) ? SW_SHOW : SW_HIDE;
				int showW = sel == TaskOptions::Weekly ? SW_SHOW : SW_HIDE;
				int showM = sel == TaskOptions::Monthly ? SW_SHOW : SW_HIDE;
				int showDT = (sel == TaskOptions::DeltaMin || sel == TaskOptions::DeltaHour) ? SW_SHOW : SW_HIDE;
				ShowWindow(GetDlgItem(hwndDlg, IDC_TRIGER_TIME), showT);  
				ShowWindow(GetDlgItem(hwndDlg, IDC_TRIGER_TIME_LABEL), showT);  
				ShowWindow(GetDlgItem(hwndDlg, IDC_TRIGER_WEEK), showW);  
				ShowWindow(GetDlgItem(hwndDlg, IDC_TRIGER_WEEK_LABEL), showW);  
				ShowWindow(GetDlgItem(hwndDlg, IDC_TRIGER_DAY), showM);  
				ShowWindow(GetDlgItem(hwndDlg, IDC_TRIGER_DAY_LABEL), showM);  
				ShowWindow(GetDlgItem(hwndDlg, IDC_TRIGER_DELTA_TIME), showDT);  
				ShowWindow(GetDlgItem(hwndDlg, IDC_TRIGER_DELTA_TIME_LABEL), showDT);  
			}
		}
		return TRUE;
	}

	return FALSE;
}

bool Options::FTPAvail()
{
	DWORD atr = GetFileAttributes(instance->ftpExePath.c_str());
	return !(atr == INVALID_FILE_ATTRIBUTES || atr & FILE_ATTRIBUTE_DIRECTORY);
}
