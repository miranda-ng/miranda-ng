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

extern bool g_SmileyAddAvail;

Options *Options::instance;

struct EventNamesType
{
	int id;
	wchar_t* name;
}
EventNames[] =
{
	EVENTTYPE_MESSAGE, LPGENW("Message"),
	EVENTTYPE_FILE, LPGENW("File transfer"),
	EVENTTYPE_AUTHREQUEST, LPGENW("Authorization request"),
	EVENTTYPE_ADDED, LPGENW("You were added"),
	EVENTTYPE_CONTACTS, LPGENW("Contacts received")
};

struct TCpTable
{
	UINT cpId;
	wchar_t *cpName;
}
cpTable[] = {
	{ CP_UTF8,	L"UTF-8"	 },
	{ 1250,	L"windows-1250"	 },
	{ 1251,	L"windows-1251" },
	{ 1252,	L"windows-1252" },
	{ 1253,	L"windows-1253" },
	{ 1254,	L"windows-1254" },
	{ 1255,	L"windows-1255" },
	{ 1256,	L"windows-1256" },
	{ 1257,	L"windows-1257" },
	{ 1258,	L"windows-1258" },
	{ 28591,	L"iso-8859-1" },
	{ 28592,	L"iso-8859-2" },
	{ 28593,	L"iso-8859-3" },
	{ 28594,	L"iso-8859-4" },
	{ 28595,	L"iso-8859-5" },
	{ 28596,	L"iso-8859-6" },
	{ 28597,	L"iso-8859-7" },
	{ 28598,	L"iso-8859-8" },
	{ 28599,	L"iso-8859-9" },
	{ 28603,	L"iso-8859-13" },
	{ 28605,	L"iso-8859-15" },
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
	encodingTxt = L"UTF-8";
	encodingHtml1 = L"UTF-8";
	encodingHtml2 = L"UTF-8";
	exportHtml1ShowDate = true;
	exportHtml2ShowDate = false;
	exportHtml2UseSmileys = true;
}

Options::~Options()
{
}

int Options::InitOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szTitle.w = LPGENW("History");

	odp.szTab.w = LPGENW("Group list");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GROUPLIST);
	odp.pfnDlgProc = Options::DlgProcOptsGroupList;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Messages");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MESSAGES);
	odp.pfnDlgProc = Options::DlgProcOptsMessages;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Searching");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SEARCHING);
	odp.pfnDlgProc = Options::DlgProcOptsSearching;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Export");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_EXPORT);
	odp.pfnDlgProc = Options::DlgProcOptsExport;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Scheduler");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SCHEDULER);
	odp.pfnDlgProc = Options::DlgProcOptsScheduler;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc = Options::DlgProcOptsMain;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

struct FontOptionsList
{
	wchar_t*   szDescr;
	COLORREF defColour;
	wchar_t*   szDefFace;
	uint8_t     defStyle;
	char     defSize;
	wchar_t*   szBackgroundName;
	uint32_t     flags;
};

struct ColorOptionsList
{
	wchar_t*		tszName;
	COLORREF 	def;
};

struct HotkeyOptionsList
{
	const char *pszName;
	wchar_t *pwszDescription;
	wchar_t *pwszSection;
	const char *pszService;
	uint16_t DefHotKey;
	LPARAM lParam;
};

static FontOptionsList g_FontOptionsList[] = {
	{LPGENW(">> Outgoing timestamp"), RGB(0, 0, 0), L"MS Shell Dlg 2", DBFONTF_BOLD, -11, LPGENW("Outgoing background"), FIDF_ALLOWEFFECTS},
	{LPGENW("<< Incoming timestamp"), RGB(0, 0, 0), L"MS Shell Dlg 2", DBFONTF_BOLD, -11, LPGENW("Incoming background"), FIDF_ALLOWEFFECTS},
	{LPGENW(">> Outgoing name"), RGB(100,100,100), L"MS Shell Dlg 2", DBFONTF_BOLD, -11, LPGENW("Outgoing background"), FIDF_ALLOWEFFECTS},
	{LPGENW("<< Incoming name"), RGB(90,160,90), L"MS Shell Dlg 2", DBFONTF_BOLD, -11, LPGENW("Incoming background"), FIDF_ALLOWEFFECTS},
	{LPGENW(">> Outgoing messages"), RGB(0, 0, 0), L"MS Shell Dlg 2", 0, -11, LPGENW("Outgoing background"), FIDF_ALLOWEFFECTS},
	{LPGENW("<< Incoming messages"), RGB(0, 0, 0), L"MS Shell Dlg 2", 0, -11, LPGENW("Incoming background"), FIDF_ALLOWEFFECTS},
	{LPGENW("Group list"), RGB(0, 0, 0), L"MS Shell Dlg 2", 0, -11, LPGENW("Group list background"), FIDF_DISABLESTYLES},
	{LPGENW("Find window"), RGB(0, 0, 0), L"MS Shell Dlg 2", 0, -11, LPGENW("Find window background"), FIDF_DISABLESTYLES},
};

static ColorOptionsList g_ColorOptionsList[] = {
	LPGENW("Outgoing background"), RGB(245,245,255),
	LPGENW("Incoming background"), RGB(245,255,245),
	LPGENW("Group list background"), GetSysColor(COLOR_3DFACE),
	LPGENW("Window background"), GetSysColor(COLOR_3DFACE),
	LPGENW("Contact list background"), GetSysColor(COLOR_3DFACE),
	LPGENW("Find window background"), GetSysColor(COLOR_WINDOW),
};

static HotkeyOptionsList g_HotkeyOptionsList[] = {
	{ "basichistory_hot_showall", LPGENW("Open global history"), LPGENW("History"), MS_HISTORY_SHOWCONTACTHISTORY, HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'H') | HKF_MIRANDA_LOCAL, 0 },
	{ "basichistory_hot_find", LPGENW("Find"), LPGENW("History"), nullptr, HOTKEYCODE(HOTKEYF_CONTROL, 'F'), HISTORY_HK_FIND },
	{ "basichistory_hot_findnext", LPGENW("Find next"), LPGENW("History"), nullptr, VK_F3, HISTORY_HK_FINDNEXT },
	{ "basichistory_hot_findprev", LPGENW("Find previous"), LPGENW("History"), nullptr, VK_F2, HISTORY_HK_FINDPREV },
	{ "basichistory_hot_matchcase", LPGENW("Switch match case"), LPGENW("History"), nullptr, 0, HISTORY_HK_MATCHCASE },
	{ "basichistory_hot_matchwhole", LPGENW("Switch match whole word"), LPGENW("History"), nullptr, 0, HISTORY_HK_MATCHWHOLE },
	{ "basichistory_hot_showcontacts", LPGENW("Show/hide contacts"), LPGENW("History"), nullptr, 0, HISTORY_HK_SHOWCONTACTS },
	{ "basichistory_hot_onlyin", LPGENW("Switch only incoming messages"), LPGENW("History"), nullptr, 0, HISTORY_HK_ONLYIN },
	{ "basichistory_hot_onlyout", LPGENW("Switch only outgoing messages"), LPGENW("History"), nullptr, 0, HISTORY_HK_ONLYOUT },
	{ "basichistory_hot_onlygroup", LPGENW("Switch only selected group"), LPGENW("History"), nullptr, 0, HISTORY_HK_ONLYGROUP },
	{ "basichistory_hot_allcontacts", LPGENW("Switch all contacts"), LPGENW("History"), nullptr, 0, HISTORY_HK_ALLCONTACTS },
	{ "basichistory_hot_delete", LPGENW("Delete"), LPGENW("History"), nullptr, VK_DELETE, HISTORY_HK_DELETE },
	{ "basichistory_hot_exrhtml", LPGENW("Export to rich HTML"), LPGENW("History"), nullptr, 0, HISTORY_HK_EXRHTML },
	{ "basichistory_hot_exphtml", LPGENW("Export to plain HTML"), LPGENW("History"), nullptr, 0, HISTORY_HK_EXPHTML },
	{ "basichistory_hot_extxt", LPGENW("Export to text file"), LPGENW("History"), nullptr, 0, HISTORY_HK_EXTXT },
	{ "basichistory_hot_exbin", LPGENW("Export to binary"), LPGENW("History"), nullptr, 0, HISTORY_HK_EXBIN },
	{ "basichistory_hot_impbin", LPGENW("Import from binary"), LPGENW("History"), nullptr, 0, HISTORY_HK_IMPBIN },
	{ "basichistory_hot_exdat", LPGENW("Export to dat (mContacts)"), LPGENW("History"), nullptr, 0, HISTORY_HK_EXDAT },
	{ "basichistory_hot_impdat", LPGENW("Import from dat (mContacts)"), LPGENW("History"), nullptr, 0, HISTORY_HK_IMPDAT },
};

const int g_fontsSize = _countof(g_FontOptionsList);

const int g_colorsSize = _countof(g_ColorOptionsList);

const int g_hotkeysSize = _countof(g_HotkeyOptionsList);

void Options::Load(void)
{
	FontIDW fid = {};
	ColourIDW cid = {};
	HOTKEYDESC hid = {};
	strncpy_s(fid.dbSettingsGroup, "BasicHistory_Fonts", _TRUNCATE);
	wcsncpy_s(fid.backgroundGroup, L"History", _TRUNCATE);
	wcsncpy_s(fid.group, LPGENW("History"), _TRUNCATE);
	for (int i = 0; i < g_fontsSize; ++i) {
		fid.order = i;
		wcsncpy_s(fid.deffontsettings.szFace, g_FontOptionsList[i].szDefFace, _TRUNCATE);
		fid.deffontsettings.size = g_FontOptionsList[i].defSize;
		fid.deffontsettings.colour = g_FontOptionsList[i].defColour;
		fid.deffontsettings.style = g_FontOptionsList[i].defStyle;
		fid.deffontsettings.charset = DEFAULT_CHARSET;
		mir_snprintf(fid.setting, _countof(fid.setting), "Font%d", i);
		wcsncpy_s(fid.name, g_FontOptionsList[i].szDescr, _TRUNCATE);
		wcsncpy_s(fid.backgroundName, g_FontOptionsList[i].szBackgroundName, _TRUNCATE);
		fid.flags = FIDF_DEFAULTVALID | FIDF_CLASSGENERAL | g_FontOptionsList[i].flags;
		g_plugin.addFont(&fid);
	}

	strncpy_s(cid.dbSettingsGroup, "BasicHistory_Fonts", _TRUNCATE);
	wcsncpy_s(cid.group, LPGENW("History"), _TRUNCATE);
	for (int i = 0; i < g_colorsSize; ++i) {
		wcsncpy_s(cid.name, g_ColorOptionsList[i].tszName, _TRUNCATE);
		mir_snprintf(cid.setting, _countof(cid.setting), "Color%d", i);
		cid.order = i;
		cid.defcolour = g_ColorOptionsList[i].def;
		g_plugin.addColor(&cid);
	}

	hid.dwFlags = HKD_UNICODE;
	for (int i = 0; i < g_hotkeysSize; ++i) {
		hid.pszName = g_HotkeyOptionsList[i].pszName;
		hid.szDescription.w = g_HotkeyOptionsList[i].pwszDescription;
		hid.szSection.w = g_HotkeyOptionsList[i].pwszSection;
		hid.pszService = g_HotkeyOptionsList[i].pszService;
		hid.DefHotKey = g_HotkeyOptionsList[i].DefHotKey;
		hid.lParam = g_HotkeyOptionsList[i].lParam;
		g_plugin.addHotkey(&hid);
	}

	showContacts = g_plugin.getByte("showContacts", 0) ? true : false;
	showContactGroups = g_plugin.getByte("showContactGroups", 1) ? true : false;
	noFindBorder = g_plugin.getByte("noFindBorder", 0) ? true : false;
	groupNewOnTop = g_plugin.getByte("groupNewOnTop", 1) ? true : false;
	groupShowEvents = g_plugin.getByte("groupShowEvents", 1) ? true : false;
	groupShowTime = g_plugin.getByte("groupShowTime", 1) ? true : false;
	groupShowName = g_plugin.getByte("groupShowName", 0) ? true : false;
	groupShowMessage = g_plugin.getByte("groupShowMessage", 1) ? true : false;
	groupMessageLen = g_plugin.getDword("groupMessageLen", 43);
	if (groupMessageLen < 5) groupMessageLen = 5;
	groupTime = g_plugin.getDword("groupTime", 5);
	if (groupTime < 1) groupTime = 1;
	groupMessagesNumber = g_plugin.getDword("groupMessagesNumber", 100);
	if (groupMessagesNumber < 1) groupMessagesNumber = 1;
	messagesNewOnTop = g_plugin.getByte("messagesNewOnTop", 0) ? true : false;
	messagesShowDate = g_plugin.getByte("messagesShowDate", 0) ? true : false;
	messagesShowSec = g_plugin.getByte("messagesShowSec", 0) ? true : false;
	messagesShowName = g_plugin.getByte("messagesShowName", 1) ? true : false;
	messagesShowEvents = g_plugin.getByte("messagesShowEvents", 0) ? true : false;
	messagesUseSmileys = g_plugin.getByte("messagesUseSmileys", 1) ? true : false;
	searchForInList = g_plugin.getByte("searchForInList", 1) ? true : false;
	searchForInMess = g_plugin.getByte("searchForInMess", 1) ? true : false;
	searchMatchCase = g_plugin.getByte("searchMatchCase", 0) ? true : false;
	searchMatchWhole = g_plugin.getByte("searchMatchWhole", 0) ? true : false;
	searchOnlyIn = g_plugin.getByte("searchOnlyIn", 0) ? true : false;
	searchOnlyOut = g_plugin.getByte("searchOnlyOut", 0) ? true : false;
	searchOnlyGroup = g_plugin.getByte("searchOnlyGroup", 0) ? true : false;
	searchAllContacts = g_plugin.getByte("searchAllContacts", 0) ? true : false;
	schedulerAlerts = g_plugin.getByte("schedulerAlerts", 1) ? true : false;
	schedulerHistoryAlerts = g_plugin.getByte("schedulerHistoryAlerts", 1) ? true : false;

	defFilter = g_plugin.getByte("defFilter", defFilter);
	int filtersCount = g_plugin.getDword("customFiltersCount", 0);
	for (int i = 0; i < filtersCount; ++i) {
		char buf[256];
		FilterOptions fo;
		mir_snprintf(buf, "filterName_%d", i);
		DBVARIANT nameV;
		if (!g_plugin.getWString(buf, &nameV)) {
			fo.name = nameV.pwszVal;
			db_free(&nameV);
		}
		else break;
		mir_snprintf(buf, "filterInOut_%d", i);
		int inOut = g_plugin.getByte(buf, 0);
		if (inOut == 1)
			fo.onlyIncomming = true;
		else if (inOut == 2)
			fo.onlyOutgoing = true;
		mir_snprintf(buf, "filterEvents_%d", i);
		DBVARIANT eventsV;
		if (!g_plugin.getString(buf, &eventsV)) {
			int k = 0;
			char* id = eventsV.pszVal;
			while (eventsV.pszVal[k]) {
				if (eventsV.pszVal[k] == ';') {
					eventsV.pszVal[k] = 0;
					fo.events.push_back(strtol(id, nullptr, 16));
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
		if (!g_plugin.getWString("defFilterStr", &defFilterStrV)) {
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

	codepageTxt = g_plugin.getDword("codepageTxt", CP_UTF8);
	codepageHtml1 = g_plugin.getDword("codepageHtml1", CP_UTF8);
	codepageHtml2 = g_plugin.getDword("codepageHtml2", CP_UTF8);
	DBVARIANT encodingV;
	if (!g_plugin.getWString("encodingTxt", &encodingV)) {
		encodingTxt = encodingV.pwszVal;
		db_free(&encodingV);
	}
	else encodingTxt = L"UTF-8";

	if (!g_plugin.getWString("encodingHtml1", &encodingV)) {
		encodingHtml1 = encodingV.pwszVal;
		db_free(&encodingV);
	}
	else encodingHtml1 = L"UTF-8";

	if (!g_plugin.getWString("encodingHtml2", &encodingV)) {
		encodingHtml2 = encodingV.pwszVal;
		db_free(&encodingV);
	}
	else encodingHtml2 = L"UTF-8";

	exportHtml1ShowDate = g_plugin.getByte("exportHtml1ShowDate", 1) ? true : false;
	exportHtml2ShowDate = g_plugin.getByte("exportHtml2ShowDate", 0) ? true : false;
	exportHtml2UseSmileys = g_plugin.getByte("exportHtml2UseSmileys", 1) ? true : false;
	if (!g_plugin.getWString("extCssHtml2", &encodingV)) {
		extCssHtml2 = encodingV.pwszVal;
		db_free(&encodingV);
	}
	else extCssHtml2 = L"";

	if (!g_plugin.getWString("ftpLogPath", &encodingV)) {
		ftpLogPath = encodingV.pwszVal;
		db_free(&encodingV);
	}

	if (!g_plugin.getWString("ftpExePath", &encodingV)) {
		ftpExePath = encodingV.pwszVal;
		db_free(&encodingV);
	}
	else ftpExePath = ftpExePathDef;

	LoadTasks();
}

COLORREF Options::GetFont(Fonts fontId, PLOGFONT font)
{
	return Font_GetW(LPGENW("History"), g_FontOptionsList[fontId].szDescr, font);
}

COLORREF Options::GetColor(Colors colorId)
{
	return Colour_GetW(LPGENW("History"), g_ColorOptionsList[colorId].tszName);
}

void Options::Save()
{
	g_plugin.setByte("showContacts", showContacts ? 1 : 0);
	g_plugin.setByte("showContactGroups", showContactGroups ? 1 : 0);
	g_plugin.setByte("noFindBorder", noFindBorder ? 1 : 0);
	g_plugin.setByte("groupNewOnTop", groupNewOnTop ? 1 : 0);
	g_plugin.setByte("groupShowEvents", groupShowEvents ? 1 : 0);
	g_plugin.setByte("groupShowTime", groupShowTime ? 1 : 0);
	g_plugin.setByte("groupShowName", groupShowName ? 1 : 0);
	g_plugin.setByte("groupShowMessage", groupShowMessage ? 1 : 0);
	if (groupMessageLen < 5)
		groupMessageLen = 5;
	g_plugin.setDword("groupMessageLen", groupMessageLen);
	if (groupTime < 1)
		groupTime = 1;
	g_plugin.setDword("groupTime", groupTime);
	if (groupMessagesNumber < 1)
		groupMessagesNumber = 1;
	g_plugin.setDword("groupMessagesNumber", groupMessagesNumber);
	g_plugin.setByte("messagesNewOnTop", messagesNewOnTop ? 1 : 0);
	g_plugin.setByte("messagesShowDate", messagesShowDate ? 1 : 0);
	g_plugin.setByte("messagesShowSec", messagesShowSec ? 1 : 0);
	g_plugin.setByte("messagesShowName", messagesShowName ? 1 : 0);
	g_plugin.setByte("messagesShowEvents", messagesShowEvents ? 1 : 0);
	g_plugin.setByte("messagesUseSmileys", messagesUseSmileys ? 1 : 0);
	g_plugin.setByte("searchForInList", searchForInList ? 1 : 0);
	g_plugin.setByte("searchForInMess", searchForInMess ? 1 : 0);
	g_plugin.setByte("searchMatchCase", searchMatchCase ? 1 : 0);
	g_plugin.setByte("searchMatchWhole", searchMatchWhole ? 1 : 0);
	g_plugin.setByte("searchOnlyIn", searchOnlyIn ? 1 : 0);
	g_plugin.setByte("searchOnlyOut", searchOnlyOut ? 1 : 0);
	g_plugin.setByte("searchOnlyGroup", searchOnlyGroup ? 1 : 0);
	g_plugin.setByte("searchAllContacts", searchAllContacts ? 1 : 0);
	g_plugin.setByte("schedulerAlerts", schedulerAlerts ? 1 : 0);
	g_plugin.setByte("schedulerHistoryAlerts", schedulerHistoryAlerts ? 1 : 0);
	if (defFilter < 0 || defFilter - 2 >= (int)customFilters.size())
		defFilter = 0;
	g_plugin.setByte("defFilter", defFilter < 2 ? defFilter : 2);
	if (defFilter >= 2)
		g_plugin.setWString("defFilterStr", customFilters[defFilter - 2].name.c_str());
	g_plugin.setDword("customFiltersCount", (uint32_t)customFilters.size());
	for (int i = 0; i < (int)customFilters.size(); ++i) {
		char buf[256];
		mir_snprintf(buf, "filterName_%d", i);
		g_plugin.setWString(buf, customFilters[i].name.c_str());
		mir_snprintf(buf, "filterInOut_%d", i);
		g_plugin.setByte(buf, customFilters[i].onlyIncomming ? 1 : (customFilters[i].onlyOutgoing ? 2 : 0));
		std::string events;
		for (std::vector<int>::iterator it = customFilters[i].events.begin(); it != customFilters[i].events.end(); ++it) {
			_itoa_s(*it, buf, 16);
			events += buf;
			events += ";";
		}

		mir_snprintf(buf, "filterEvents_%d", i);
		g_plugin.setString(buf, events.c_str());
	}

	g_plugin.setDword("codepageTxt", codepageTxt);
	g_plugin.setDword("codepageHtml1", codepageHtml1);
	g_plugin.setDword("codepageHtml2", codepageHtml2);
	g_plugin.setWString("encodingTxt", encodingTxt.c_str());
	g_plugin.setWString("encodingHtml1", encodingHtml1.c_str());
	g_plugin.setWString("encodingHtml2", encodingHtml2.c_str());
	g_plugin.setByte("exportHtml1ShowDate", exportHtml1ShowDate ? 1 : 0);
	g_plugin.setByte("exportHtml2ShowDate", exportHtml2ShowDate ? 1 : 0);
	g_plugin.setByte("exportHtml2UseSmileys", exportHtml2UseSmileys ? 1 : 0);
	g_plugin.setWString("extCssHtml2", extCssHtml2.c_str());
	g_plugin.setWString("ftpLogPath", ftpLogPath.c_str());
	if (ftpExePath != ftpExePathDef)
		g_plugin.setWString("ftpExePath", ftpExePath.c_str());
	else
		g_plugin.delSetting("ftpExePath");
}

void Options::SaveTasks(std::list<TaskOptions>* tasks)
{
	mir_cslock lck(criticalSection);
	int oldTaskNr = (int)taskOptions.size();
	taskOptions.clear();
	int i = 0;
	char buf[256];
	for (std::list<TaskOptions>::iterator it = tasks->begin(); it != tasks->end(); ++it) {
		mir_snprintf(buf, "Task_compress_%d", i);
		g_plugin.setByte(buf, it->compress);
		mir_snprintf(buf, "Task_useFtp_%d", i);
		g_plugin.setByte(buf, it->useFtp);
		mir_snprintf(buf, "Task_isSystem_%d", i);
		g_plugin.setByte(buf, it->isSystem);
		mir_snprintf(buf, "Task_active_%d", i);
		g_plugin.setByte(buf, it->active);
		mir_snprintf(buf, "Task_exportImported_%d", i);
		g_plugin.setByte(buf, it->exportImported);
		mir_snprintf(buf, "Task_type_%d", i);
		g_plugin.setByte(buf, it->type);
		mir_snprintf(buf, "Task_eventUnit_%d", i);
		g_plugin.setByte(buf, it->eventUnit);
		mir_snprintf(buf, "Task_trigerType_%d", i);
		g_plugin.setByte(buf, it->trigerType);
		mir_snprintf(buf, "Task_exportType_%d", i);
		g_plugin.setByte(buf, it->exportType);
		mir_snprintf(buf, "Task_importType_%d", i);
		g_plugin.setByte(buf, it->importType);
		mir_snprintf(buf, "Task_eventDeltaTime_%d", i);
		g_plugin.setDword(buf, it->eventDeltaTime);
		mir_snprintf(buf, "Task_filterId_%d", i);
		g_plugin.setDword(buf, it->filterId);
		mir_snprintf(buf, "Task_dayTime_%d", i);
		g_plugin.setDword(buf, it->dayTime);
		mir_snprintf(buf, "Task_dayOfWeek_%d", i);
		g_plugin.setDword(buf, it->dayOfWeek);
		mir_snprintf(buf, "Task_dayOfMonth_%d", i);
		g_plugin.setDword(buf, it->dayOfMonth);
		mir_snprintf(buf, "Task_deltaTime_%d", i);
		g_plugin.setDword(buf, it->deltaTime);
		mir_snprintf(buf, "Task_lastExport_low_%d", i);
		g_plugin.setDword(buf, (int)it->lastExport);
		mir_snprintf(buf, "Task_lastExport_hi_%d", i);
		g_plugin.setDword(buf, ((unsigned long long int)it->lastExport) >> 32);
		mir_snprintf(buf, "Task_ftpName_%d", i);
		g_plugin.setWString(buf, it->ftpName.c_str());
		mir_snprintf(buf, "Task_filterName_%d", i);
		g_plugin.setWString(buf, it->filterName.c_str());
		mir_snprintf(buf, "Task_filePath_%d", i);
		g_plugin.setWString(buf, it->filePath.c_str());
		mir_snprintf(buf, "Task_taskName_%d", i);
		g_plugin.setWString(buf, it->taskName.c_str());
		mir_snprintf(buf, "Task_zipPassword_%d", i);
		g_plugin.setString(buf, it->zipPassword.c_str());

		mir_snprintf(buf, "IsInTask_%d", i);
		for (auto &hContact : Contacts())
			g_plugin.delSetting(hContact, buf);

		for (size_t j = 0; j < it->contacts.size(); ++j)
			g_plugin.setByte(it->contacts[j], buf, 1);

		it->orderNr = i++;
		taskOptions.push_back(*it);
	}

	g_plugin.setDword("Task_count", i);

	for (i = (int)tasks->size(); i < oldTaskNr; ++i) {
		mir_snprintf(buf, "Task_compress_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_useFtp_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_isSystem_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_active_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_type_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_eventUnit_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_trigerType_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_exportType_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_eventDeltaTime_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_filterId_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_dayTime_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_dayOfWeek_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_dayOfMonth_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_deltaTime_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_lastExport_low_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_lastExport_hi_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_ftpName_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_filterName_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_filePath_%d", i);
		g_plugin.delSetting(buf);
		mir_snprintf(buf, "Task_taskName_%d", i);
		g_plugin.delSetting(buf);

		mir_snprintf(buf, "IsInTask_%d", i);
		for (auto &hContact : Contacts())
			g_plugin.delSetting(hContact, buf);
	}
}

void Options::SaveTaskTime(TaskOptions& to)
{
	int i = to.orderNr;
	char buf[256];
	mir_snprintf(buf, "Task_lastExport_low_%d", i);
	g_plugin.setDword(buf, (int)to.lastExport);
	mir_snprintf(buf, "Task_lastExport_hi_%d", i);
	g_plugin.setDword(buf, ((unsigned long long int)to.lastExport) >> 32);
}

void Options::LoadTasks()
{
	int taskCount = g_plugin.getDword("Task_count", 0);
	char buf[256];
	for (int i = 0; i < taskCount; ++i) {
		TaskOptions to;
		mir_snprintf(buf, "Task_compress_%d", i);
		to.compress = g_plugin.getByte(buf, to.compress) != 0;
		mir_snprintf(buf, "Task_useFtp_%d", i);
		to.useFtp = g_plugin.getByte(buf, to.useFtp) != 0;
		mir_snprintf(buf, "Task_isSystem_%d", i);
		to.isSystem = g_plugin.getByte(buf, to.isSystem) != 0;
		mir_snprintf(buf, "Task_active_%d", i);
		to.active = g_plugin.getByte(buf, to.active) != 0;
		mir_snprintf(buf, "Task_exportImported_%d", i);
		to.exportImported = g_plugin.getByte(buf, to.exportImported) != 0;
		mir_snprintf(buf, "Task_type_%d", i);
		to.type = (TaskOptions::TaskType)g_plugin.getByte(buf, to.type);
		mir_snprintf(buf, "Task_eventUnit_%d", i);
		to.eventUnit = (TaskOptions::EventUnit)g_plugin.getByte(buf, to.eventUnit);
		mir_snprintf(buf, "Task_trigerType_%d", i);
		to.trigerType = (TaskOptions::TrigerType)g_plugin.getByte(buf, to.trigerType);
		mir_snprintf(buf, "Task_exportType_%d", i);
		to.exportType = (IExport::ExportType)g_plugin.getByte(buf, to.exportType);
		mir_snprintf(buf, "Task_importType_%d", i);
		to.importType = (IImport::ImportType)g_plugin.getByte(buf, to.importType);
		mir_snprintf(buf, "Task_eventDeltaTime_%d", i);
		to.eventDeltaTime = g_plugin.getDword(buf, to.eventDeltaTime);
		mir_snprintf(buf, "Task_filterId_%d", i);
		to.filterId = g_plugin.getDword(buf, to.filterId);
		mir_snprintf(buf, "Task_dayTime_%d", i);
		to.dayTime = g_plugin.getDword(buf, to.dayTime);
		mir_snprintf(buf, "Task_dayOfWeek_%d", i);
		to.dayOfWeek = g_plugin.getDword(buf, to.dayOfWeek);
		mir_snprintf(buf, "Task_dayOfMonth_%d", i);
		to.dayOfMonth = g_plugin.getDword(buf, to.dayOfMonth);
		mir_snprintf(buf, "Task_deltaTime_%d", i);
		to.deltaTime = g_plugin.getDword(buf, to.deltaTime);
		unsigned long long int le = to.lastExport;
		mir_snprintf(buf, "Task_lastExport_low_%d", i);
		to.lastExport = g_plugin.getDword(buf, (int)le) & 0xffffffff;
		mir_snprintf(buf, "Task_lastExport_hi_%d", i);
		to.lastExport |= ((unsigned long long int)g_plugin.getDword(buf, le >> 32)) << 32;
		mir_snprintf(buf, "Task_ftpName_%d", i);
		DBVARIANT var;
		if (!g_plugin.getWString(buf, &var)) {
			to.ftpName = var.pwszVal;
			db_free(&var);
		}
		mir_snprintf(buf, "Task_filterName_%d", i);
		if (!g_plugin.getWString(buf, &var)) {
			to.filterName = var.pwszVal;
			db_free(&var);
		}
		mir_snprintf(buf, "Task_filePath_%d", i);
		if (!g_plugin.getWString(buf, &var)) {
			to.filePath = var.pwszVal;
			db_free(&var);
		}
		mir_snprintf(buf, "Task_taskName_%d", i);
		if (!g_plugin.getWString(buf, &var)) {
			to.taskName = var.pwszVal;
			db_free(&var);
		}
		mir_snprintf(buf, "Task_zipPassword_%d", i);
		if (!g_plugin.getString(buf, &var)) {
			to.zipPassword = var.pszVal;
			db_free(&var);
		}

		mir_snprintf(buf, "IsInTask_%d", i);
		for (auto &hContact : Contacts())
			if (g_plugin.getByte(hContact, buf) == 1)
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
	int cpCount = _countof(EventNames);
	int selCpIdx = -1;
	for (int i = 0; i < cpCount; ++i)
		if (EventNames[i].id == eventId)
			selCpIdx = i;

	if (selCpIdx == -1) {
		wchar_t buf[24];
		mir_snwprintf(buf, L"%d", eventId);
		ComboBox_SetText(hwndCB, buf);
	}
	else ComboBox_SetCurSel(hwndCB, selCpIdx);
}

int GetEventCB(HWND hwndCB, bool errorReport, int &eventId)
{
	int selCpIdx = ComboBox_GetCurSel(hwndCB);
	if (selCpIdx < 0) {
		wchar_t text[24];
		ComboBox_GetText(hwndCB, text, 24);
		wchar_t * stopOn = nullptr;
		long cp = wcstol(text, &stopOn, 10);
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
	while (ListBox_GetCount(hwndLB) > 0)
		ListBox_DeleteString(hwndLB, 0);
}

void ReloadEventLB(HWND hwndLB, const FilterOptions &sel)
{
	while (ListBox_GetCount(hwndLB) > 0)
		ListBox_DeleteString(hwndLB, 0);

	if (sel.onlyIncomming && !sel.onlyOutgoing)
		ListBox_AddString(hwndLB, TranslateT("Incoming events"));
	else if (sel.onlyOutgoing && !sel.onlyIncomming)
		ListBox_AddString(hwndLB, TranslateT("Outgoing events"));

	for (std::vector<int>::const_iterator it = sel.events.begin(); it != sel.events.end(); ++it) {
		int cpCount = _countof(EventNames);
		int selCpIdx = -1;
		for (int i = 0; i < cpCount; ++i)
			if (EventNames[i].id == *it)
				selCpIdx = i;

		if (selCpIdx == -1) {
			wchar_t buf[24];
			mir_snwprintf(buf, L"%d", *it);
			ListBox_AddString(hwndLB, buf);
		}
		else ListBox_AddString(hwndLB, TranslateW(EventNames[selCpIdx].name));
	}
}

bool CheckFile(HWND hwndEdit)
{
	wchar_t buf[MAX_PATH];
	Edit_GetText(hwndEdit, buf, MAX_PATH);
	uint32_t atr = GetFileAttributes(buf);
	if (atr == INVALID_FILE_ATTRIBUTES || atr & FILE_ATTRIBUTE_DIRECTORY) {
		MessageBox(GetParent(hwndEdit), TranslateT("File does not exist. Enter correct file path."), TranslateT("Invalid file"), MB_OK | MB_ICONERROR);
		SetFocus(hwndEdit);
		return false;
	}

	return true;
}

bool OpenFileDlg(HWND hwndDlg, HWND hwndEdit, const wchar_t* defName, const wchar_t* ext, const wchar_t* title, bool open)
{
	wchar_t filter[1024];
	std::locale loc;
	wchar_t extUpper[32];
	wcscpy_s(extUpper, ext);
	extUpper[0] = std::toupper(ext[0], loc);
	mir_snwprintf(filter, TranslateT("%s files (*.%s)"), extUpper, ext);
	size_t len = mir_wstrlen(filter) + 1;
	mir_snwprintf(filter + len, _countof(filter) - len, L"*.%s", ext);
	len += mir_wstrlen(filter + len) + 1;
	wcscpy_s(filter + len, 1024 - len, TranslateT("All files (*.*)"));
	len += mir_wstrlen(filter + len) + 1;
	wcscpy_s(filter + len, 1024 - len, L"*.*");
	len += mir_wstrlen(filter + len) + 1;
	filter[len] = 0;
	wchar_t stzFilePath[1024];

	Edit_GetText(hwndEdit, stzFilePath, 1023);
	if (stzFilePath[0] == 0) {
		wcscpy_s(stzFilePath, defName);
		len = mir_wstrlen(stzFilePath) + 1;
		stzFilePath[len] = 0;
	}
	else {
		len = mir_wstrlen(stzFilePath) + 1;
		stzFilePath[len] = 0;
	}

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwndDlg;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = stzFilePath;
	ofn.lpstrTitle = title;
	ofn.nMaxFile = _countof(stzFilePath);
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
	switch (msg) {
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
			for (auto &it : EventNames)
				ComboBox_AddString(events, TranslateW(it.name));

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
				switch (LOWORD(wParam)) {
				case IDC_ADD_FILTER:
					{
						wchar_t name[24];
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
					if (!OpenFileDlg(hwndDlg, GetDlgItem(hwndDlg, IDC_WINSCP), L"WinSCP.exe", L"exe", TranslateT("Browse WinSCP file"), true))
						return TRUE;
					break;

				case IDC_WINSCPLOG_BROWSE:
					if (!OpenFileDlg(hwndDlg, GetDlgItem(hwndDlg, IDC_WINSCPLOG), L"ftplog.txt", L"txt", TranslateT("Save WinSCP log file"), false))
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
			wchar_t buf[MAX_PATH];
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
	switch (msg) {
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
	switch (msg) {
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
	switch (msg) {
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
		ComboBox_AddString(hwndCB, TranslateW(cpTable[i].cpName));
		if (cpTable[i].cpId == codepage && name == cpTable[i].cpName)
			selCpIdx = i;
	}

	if (selCpIdx == -1) {
		wchar_t buf[300];
		mir_snwprintf(buf, L"%d;%s", codepage, name.c_str());
		ComboBox_SetText(hwndCB, buf);
	}
	else ComboBox_SetCurSel(hwndCB, selCpIdx);

	ComboBox_LimitText(hwndCB, 127);
}

unsigned int GetCodepageCB(HWND hwndCB, bool errorReport, unsigned int, const std::wstring& defName, std::wstring& name)
{
	int selCpIdx = ComboBox_GetCurSel(hwndCB);
	if (selCpIdx < 0) {
		wchar_t text[128];
		ComboBox_GetText(hwndCB, text, 128);
		std::wstring str = text;
		name = L"";
		size_t pos = str.find_first_of(';');
		if (pos < str.length()) {
			text[pos] = 0;
			name = str.substr(pos + 1);
		}

		wchar_t * stopOn = nullptr;
		long cp = wcstol(text, &stopOn, 10);
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
	switch (msg) {
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
				if (!OpenFileDlg(hwndDlg, GetDlgItem(hwndDlg, IDC_HTML2EXTCSSFILE), L"", L"css", TranslateT("Browse CSS file"), true))
					return TRUE;
			}

			if (init && (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == EN_CHANGE))
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

				wchar_t buf[MAX_PATH];
				Edit_GetText(GetDlgItem(hwndDlg, IDC_HTML2EXTCSSFILE), buf, MAX_PATH);
				instance->extCssHtml2 = buf;
			}
			else instance->extCssHtml2 = L"";

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

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			tasks = new std::list<TaskOptions>(Options::instance->taskOptions.begin(), Options::instance->taskOptions.end());
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)tasks);
			HWND listTasks = GetDlgItem(hwndDlg, IDC_LIST_TASKS);
			for (std::list<TaskOptions>::iterator it = tasks->begin(); it != tasks->end(); ++it)
				ListBox_AddString(listTasks, it->taskName.c_str());

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
			switch (LOWORD(wParam)) {
			case IDC_EDIT_TASK:
				if (sel >= 0) {
					std::list<TaskOptions>::iterator it = tasks->begin();
					while (sel-- > 0 && it != tasks->end())
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
					if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DLG_TASK), hwndDlg, DlgProcOptsTask, (LPARAM)&top) == IDOK) {
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
					while (sel-- > 0 && it != tasks->end())
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
		SendMessage(hwnd, CLM_SETCHECKMARK, (WPARAM)hSystem, 1);

	for (size_t i = 0; i < to->contacts.size(); ++i) {
		hItem = (HANDLE)SendMessage(hwnd, CLM_FINDCONTACT, (WPARAM)to->contacts[i], 0);
		if (hItem)
			SendMessage(hwnd, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
	}
}

void SaveList(HWND hwnd, MCONTACT hSystem, TaskOptions* to)
{
	to->contacts.clear();
	if (hSystem)
		to->isSystem = SendMessage(hwnd, CLM_GETCHECKMARK, (WPARAM)hSystem, 0) != 0;

	for (auto &hContact : Contacts()) {
		HANDLE hItem = (HANDLE)SendMessage(hwnd, CLM_FINDCONTACT, hContact, 0);
		if (hItem && SendMessage(hwnd, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
			to->contacts.push_back(hContact);
	}
}

bool IsValidTask(TaskOptions& to, std::list<TaskOptions>* top = nullptr, std::wstring* err = nullptr, std::wstring* errDescr = nullptr);

#ifndef LOCALE_SSHORTTIME
#define LOCALE_SSHORTTIME             0x00000079
#endif

INT_PTR CALLBACK Options::DlgProcOptsTask(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static MCONTACT hSystem;
	switch (msg) {
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
			HWND password = GetDlgItem(hwndDlg, IDC_PASSWORD);
			HWND expImp = GetDlgItem(hwndDlg, IDC_EXPIMP);

			Edit_LimitText(name, 16);
			Edit_SetText(name, to->taskName.c_str());

			Button_SetCheck(active, to->active);

			Button_SetCheck(expImp, to->exportImported);

			ComboBox_AddString(comboType, TranslateT("Export"));
			ComboBox_AddString(comboType, TranslateT("Delete"));
			ComboBox_AddString(comboType, TranslateT("Export and delete"));
			ComboBox_AddString(comboType, TranslateT("Import"));
			ComboBox_AddString(comboType, TranslateT("Import and merge"));
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

			ComboBox_AddString(trigerType, TranslateT("At start"));
			ComboBox_AddString(trigerType, TranslateT("At finish"));
			ComboBox_AddString(trigerType, TranslateT("Daily"));
			ComboBox_AddString(trigerType, TranslateT("Weekly"));
			ComboBox_AddString(trigerType, TranslateT("Monthly"));
			ComboBox_AddString(trigerType, TranslateT("Delta time (minutes)"));
			ComboBox_AddString(trigerType, TranslateT("Delta time (hours)"));
			ComboBox_SetCurSel(trigerType, to->trigerType);

			ComboBox_AddString(exportType, TranslateT("Rich HTML"));
			ComboBox_AddString(exportType, TranslateT("Plain HTML"));
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

			wchar_t timeFormat[10];
			if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTTIME, timeFormat, 10) == 0) {
				wchar_t sep = ':';
				if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, timeFormat, 10) > 0)
					sep = timeFormat[0];
				mir_snwprintf(timeFormat, L"HH%cmm", sep);
			}

			SYSTEMTIME st;
			GetSystemTime(&st);
			st.wHour = to->dayTime / 60;
			st.wMinute = to->dayTime % 60;
			st.wSecond = 0;
			st.wMilliseconds = 0;
			DateTime_SetFormat(time, timeFormat);
			DateTime_SetSystemtime(time, GDT_VALID, &st);

			CLCINFOITEM cii = { 0 };
			cii.cbSize = sizeof(cii);
			cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX | CLCIIF_BELOWCONTACTS;
			cii.pszText = TranslateT("System");
			hSystem = (MCONTACT)SendMessage(contactList, CLM_ADDINFOITEM, 0, (LPARAM)& cii);
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
					wchar_t tszBuf[256];
					mir_snwprintf(tszBuf, TranslateT("Invalid '%s' value."), TranslateT("Events older than"));
					MessageBox(hwndDlg, tszBuf, TranslateT("Error"), MB_ICONERROR);
					break;
				}
				toCp.eventUnit = (enum TaskOptions::EventUnit)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_EVENT_UNIT));
				toCp.trigerType = (enum TaskOptions::TrigerType)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_TRIGER_TYPE));
				toCp.exportType = (enum IExport::ExportType)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_EXPORT_TYPE));
				toCp.importType = (enum IImport::ImportType)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_IMPORT_TYPE));
				toCp.compress = Button_GetCheck(GetDlgItem(hwndDlg, IDC_COMPRESS)) != 0;
				char bufC[100];
				GetDlgItemTextA(hwndDlg, IDC_PASSWORD, bufC, _countof(bufC));
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
						wchar_t tszBuf[256];
						mir_snwprintf(tszBuf, TranslateT("Invalid '%s' value."), TranslateT("Day"));
						MessageBox(hwndDlg, tszBuf, TranslateT("Error"), MB_ICONERROR);
						break;
					}
					else toCp.dayOfMonth = to->dayOfMonth;
				}
				toCp.deltaTime = GetDlgItemInt(hwndDlg, IDC_TRIGER_DELTA_TIME, &isOK, FALSE);
				if (!isOK) {
					if (toCp.trigerType == TaskOptions::DeltaMin || toCp.trigerType == TaskOptions::DeltaHour) {
						wchar_t tszBuf[256];
						mir_snwprintf(tszBuf, TranslateT("Invalid '%s' value."), TranslateT("Delta time"));
						MessageBox(hwndDlg, tszBuf, TranslateT("Error"), MB_ICONERROR);
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
					wchar_t tszBuf[256];
					if (err.empty())
						wcscpy_s(tszBuf, TranslateT("Some value is invalid"));
					else if (errDescr.empty())
						mir_snwprintf(tszBuf, TranslateT("Invalid '%s' value."), err.c_str());
					else
						mir_snwprintf(tszBuf, TranslateT("Invalid '%s' value.\n%s"), err.c_str(), errDescr.c_str());
					MessageBox(hwndDlg, tszBuf, TranslateT("Error"), MB_ICONERROR);
					break;
				}

				toCp.lastExport = time(0);

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
			else if (LOWORD(wParam) == IDC_COMPRESS) {
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
				wchar_t* compressText = TranslateT("Compress output files");
				wchar_t* uploadText = TranslateT("Upload to FTP (WinSCP required)");
				wchar_t* typeText = TranslateT("Export to");
				if (showFilter == SW_HIDE) {
					str = TranslateT("** Use <ext> to insert extension, <contact> to insert contact name");
					compressText = TranslateT("Input files are compressed");
					uploadText = TranslateT("Download from FTP (WinSCP required)");
					typeText = TranslateT("Import from");
				}
				else if (show == SW_HIDE)
					str = TranslateT("* Use negative values to filter younger events");
				else {
					str = TranslateT("* Use negative values to filter younger events");
					str += L"\n";
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
	uint32_t atr = GetFileAttributes(instance->ftpExePath.c_str());
	return !(atr == INVALID_FILE_ATTRIBUTES || atr & FILE_ATTRIBUTE_DIRECTORY);
}
