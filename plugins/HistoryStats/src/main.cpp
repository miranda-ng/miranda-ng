#include "stdafx.h"
#include "main.h"

#include <clocale>

#include "dlgoption.h"
#include "resource.h"
#include "column.h"
#include "bandctrlimpl.h"
#include "optionsctrlimpl.h"
#include "iconlib.h"
#include "dlgconfigure.h"

CLIST_INTERFACE *pcli;
HINSTANCE g_hInst;
int hLangpack;

static const int g_pluginFileListID = 2535;

PLUGININFOEX g_pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {F184F5A0-C198-4454-A9B4-F6E2FD534133}
	{0xf184f5a0, 0xc198, 0x4454, {0xa9, 0xb4, 0xf6, 0xe2, 0xfd, 0x53, 0x41, 0x33}},
};

SettingsSerializer* g_pSettings = NULL;

bool g_bMainMenuExists    = false;
bool g_bContactMenuExists = false;
bool g_bExcludeLock       = false;
bool g_bConfigureLock     = false;

static HGENMENU g_hMenuCreateStatistics = NULL;
static HGENMENU g_hMenuShowStatistics   = NULL;
static HGENMENU g_hMenuConfigure        = NULL;
static HGENMENU g_hMenuToggleExclude    = NULL;

#if defined(HISTORYSTATS_HISTORYCOPY)
	static HGENMENU g_hMenuHistoryCopy    = NULL;
	static HGENMENU g_hMenuHistoryPaste   = NULL;
	static HGENMENU g_hHistoryCopyContact = NULL;
#endif

/*
 * services (see m_historystats.h for details)
 */

static INT_PTR SvcIsExcluded(WPARAM hContact, LPARAM)
{
	if (hContact) {
		MirandaSettings db;

		db.setContact(hContact);
		db.setModule(con::ModHistoryStats);

		return db.readBool(con::SettExclude, false) ? 1 : 0;
	}

	return 0;
}

static INT_PTR SvcSetExclude(WPARAM hContact, LPARAM lParam)
{
	if (hContact) {
		MirandaSettings db;

		db.setContact(hContact);
		db.setModule(con::ModHistoryStats);

		if (db.readBool(con::SettExclude, false))
		{
			if (!lParam)
			{
				db.delSetting(con::SettExclude);
			}
		}
		else
		{
			if (lParam)
			{
				db.writeBool(con::SettExclude, true);
			}
		}
	}

	return 0;
}

/*
 * global menu stuff
 */

static void MenuIconsChanged(LPARAM)
{
	if (g_hMenuCreateStatistics)
		Menu_ModifyItem(g_hMenuCreateStatistics, NULL, IconLib::getIcon(IconLib::iiMenuCreateStatistics));

	if (g_hMenuShowStatistics)
		Menu_ModifyItem(g_hMenuShowStatistics, NULL, IconLib::getIcon(IconLib::iiMenuShowStatistics));

	if (g_hMenuConfigure)
		Menu_ModifyItem(g_hMenuConfigure, NULL, IconLib::getIcon(IconLib::iiMenuConfigure));

	if (g_hMenuToggleExclude)
		Menu_ModifyItem(g_hMenuToggleExclude, NULL, IconLib::getIcon(IconLib::iiContactMenu));
}

/*
 * main menu related stuff
 */

static INT_PTR MenuCreateStatistics(WPARAM, LPARAM)
{
	Statistic::run(*g_pSettings, Statistic::fromMenu, g_hInst);
	return 0;
}

static INT_PTR MenuShowStatistics(WPARAM, LPARAM)
{
	if (g_pSettings->canShowStatistics())
		g_pSettings->showStatistics();
	else
		MessageBox(NULL,
			TranslateT("The statistics can't be found. Either you never created them or the last created statistics were moved to a different location and can't be found anymore."),
			TranslateT("HistoryStats - Warning"), MB_ICONWARNING | MB_OK);

	return 0;
}

static INT_PTR MenuConfigure(WPARAM, LPARAM)
{
	DlgConfigure::showModal();
	return 0;
}

void AddMainMenu()
{
	if (!g_pSettings->m_ShowMainMenu || g_bMainMenuExists)
	{
		return;
	}

	g_bMainMenuExists = true;

	bool bInPopup = g_pSettings->m_ShowMainMenuSub;

	CreateServiceFunction(con::SvcCreateStatistics, MenuCreateStatistics);
	CreateServiceFunction(con::SvcShowStatistics, MenuShowStatistics);
	CreateServiceFunction(con::SvcConfigure, MenuConfigure);

	HGENMENU hRoot = bInPopup ? Menu_CreateRoot(MO_MAIN, LPGENT("Statistics"), 1910000000) : NULL;
	Menu_ConfigureItem(hRoot, MCI_OPT_UID, "7F116B24-9D84-4D04-B6AA-EED95051A184");

	g_hMenuCreateStatistics = mu::clist::addMainMenuItem(
		LPGENT("Create statistics"), 0, 1910000000, IconLib::getIcon(IconLib::iiMenuCreateStatistics), con::SvcCreateStatistics, hRoot);

	g_hMenuShowStatistics = mu::clist::addMainMenuItem(
		LPGENT("Show statistics"), // MEMO: implicit translation
		0, 1910000001, IconLib::getIcon(IconLib::iiMenuShowStatistics), con::SvcShowStatistics, hRoot);

	g_hMenuConfigure = mu::clist::addMainMenuItem(
		bInPopup ? LPGENT("Configure...") : LPGENT("Configure statistics..."), // MEMO: implicit translation
		0, 1910000002, IconLib::getIcon(IconLib::iiMenuConfigure), con::SvcConfigure, hRoot);
}

/*
 * contact menu related stuff
 */

static INT_PTR MenuToggleExclude(WPARAM hContact, LPARAM)
{
	if (hContact) {
		MirandaSettings db;

		db.setContact(hContact);
		db.setModule(con::ModHistoryStats);

		if (db.readBool(con::SettExclude, false))
			db.delSetting(con::SettExclude);
		else
			db.writeBool(con::SettExclude, true);
	}

	return 0;
}

#if defined(HISTORYSTATS_HISTORYCOPY)
static INT_PTR MenuHistoryCopy(WPARAM hContact, LPARAM lParam)
{
	if (hContact)
		g_hHistoryCopyContact = hContact;
	return 0;
}

static INT_PTR MenuHistoryPaste(WPARAM wParam, LPARAM lParam)
{
	HANDLE hTarget = reinterpret_cast<HANDLE>(wParam);

	if (!hTarget)
	{
		return 0;
	}

	// ask user if this is really what he wants
	ext::string strConfirm = ext::str(ext::kformat(TranslateT("You're going to copy the complete history of #{source_name} (#{source_proto}) to #{target_name} (#{target_proto}). Afterwards, the target history will contain entries from both histories. There is no way to revert this operation. Be careful! This is a rather big operation and has the potential to damage your database. Be sure to have a backup of this database before performing this operation.\r\n\r\nAre you sure you would like to continue?")))
		% _T("#{source_name}") * mu::clist::getContactDisplayName(g_hHistoryCopyContact)
		% _T("#{source_proto}") * utils::fromA(GetContactProto(g_hHistoryCopyContact))
		% _T("#{target_name}") * mu::clist::getContactDisplayName(hTarget)
		% _T("#{target_proto}") * utils::fromA(GetContactProto(hTarget)));

	if (MessageBox(0, strConfirm.c_str(), TranslateT("HistoryStats - Confirm")), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES)
	{
		return 0;
	}

	// turn off safety mode
	mu::db::setSafetyMode(false);

	// copy history
	DWORD dwCountSuccess = 0, dwCountFailRead = 0, dwCountFailAdd = 0;
	DBEVENTINFO dbe;
	int blobBuffer = 4096;
	HANDLE hEvent = mu::db_event::findFirst(g_hHistoryCopyContact);

	memset(&dbe, 0, sizeof(dbe));
	dbe.cbSize = sizeof(dbe);
	dbe.pBlob = reinterpret_cast<BYTE*>(malloc(blobBuffer));

	while (hEvent)
	{
		dbe.cbBlob = db_event_getBlobSize(hEvent);

		if (blobBuffer < dbe.cbBlob)
		{
			blobBuffer = 4096 * ((4095 + dbe.cbBlob) / 4096);
			dbe.pBlob = reinterpret_cast<BYTE*>(realloc(dbe.pBlob, blobBuffer));
		}

		if (db_event_get(hEvent, &dbe) == 0) {
			++dwCountSuccess;

			// clear "first" flag
			dbe.flags &= ~DBEF_FIRST;

			if (mu::db_event::add(hTarget, &dbe) == NULL)
				++dwCountFailAdd;
		}
		else ++dwCountFailRead;

		hEvent = db_event_findNext(hEvent);
	}

	free(dbe.pBlob);

	// turn safety mode back on
	mu::db::setSafetyMode(true);

	// output summary
	ext::string strSummary = ext::str(ext::kformat(TranslateT("Successfully read #{success} events of which #{fail_add} couldn't be added to the target history. #{fail} events couldn't be read from the source history.")))
		% _T("#{success}") * dwCountSuccess
		% _T("#{fail}") * dwCountFailRead
		% _T("#{fail_add}") * dwCountFailAdd);

	MessageBox(0, strSummary.c_str(), TranslateT("HistoryStats - Information")), MB_ICONINFORMATION);

	g_hHistoryCopyContact = NULL;

	return 0;
}
#endif

static int EventPreBuildContactMenu(WPARAM hContact, LPARAM)
{
	if (hContact)
	{
		const char* szProto = GetContactProto(hContact);

		if ((!g_pSettings->m_ShowContactMenuPseudo && (!szProto || !(mu::protosvc::getCaps(szProto, PFLAGNUM_2) & ~mu::protosvc::getCaps(szProto, PFLAGNUM_5)))) ||
			g_pSettings->m_HideContactMenuProtos.find(szProto) != g_pSettings->m_HideContactMenuProtos.end())
		{
			Menu_ShowItem(g_hMenuToggleExclude, false);
		}
		else {
			MirandaSettings db;
			db.setContact(hContact);
			db.setModule(con::ModHistoryStats);

			Menu_SetChecked(g_hMenuToggleExclude, db.readBool(con::SettExclude, false));
			Menu_EnableItem(g_hMenuToggleExclude, !g_bExcludeLock); // avoid collision with options page
		}

#if defined(HISTORYSTATS_HISTORYCOPY)
		int menuStateCopy = (g_hHistoryCopyContact && g_hHistoryCopyContact != hContact) ? 0 : CMIF_GRAYED;

		Menu_ModifyItem(g_hMenuHistoryPaste, CMIM_FLAGS, NULL, menuStateCopy);
#endif
	}

	return 0;
}

void AddContactMenu()
{
	if (!g_pSettings->m_ShowContactMenu || g_bContactMenuExists)
		return;

	g_bContactMenuExists = true;

	CreateServiceFunction(con::SvcToggleExclude, MenuToggleExclude);

	g_hMenuToggleExclude = mu::clist::addContactMenuItem(
		LPGENT("Exclude from statistics"), // MEMO: implicit translation
		0,
		800000,
		IconLib::getIcon(IconLib::iiContactMenu),
		con::SvcToggleExclude);

#if defined(HISTORYSTATS_HISTORYCOPY)
	CreateServiceFunction(con::SvcHistoryCopy, MenuHistoryCopy);
	CreateServiceFunction(con::SvcHistoryPaste, MenuHistoryPaste);

	g_hMenuHistoryCopy = mu::clist::addContactMenuItem(
		LPGENT("Copy history")), // MEMO: implicit translation
		0,
		800001,
		NULL,
		con::SvcHistoryCopy);

	g_hMenuHistoryPaste = mu::clist::addContactMenuItem(
		LPGENT("Paste history...")), // MEMO: implicit translation
		0,
		800002,
		NULL,
		con::SvcHistoryPaste);
#endif

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, EventPreBuildContactMenu);
}

/*
 * options integration
 */

static int EventOptInitialise(WPARAM wParam, LPARAM)
{
	mu::opt::addPage(
		wParam,
		TranslateT("History"),
		TranslateT("Statistics"),
		NULL,
		DlgOption::staticDlgProc,
		MAKEINTRESOURCEA(IDD_OPTIONS),
		g_hInst);

	return 0;
}

/*
 * second initialization phase
 */

static int EventModulesLoaded(WPARAM, LPARAM)
{
	// register all known columns
	Column::registerColumns();

	// read settings
	g_pSettings = new SettingsSerializer(con::ModHistoryStats);
	g_pSettings->readFromDB();

	// integrate into options dialog
	HookEvent(ME_OPT_INITIALISE, EventOptInitialise);

	// integrate with icolib
	IconLib::init();
	IconLib::registerCallback(MenuIconsChanged, 0);

	// integrate into main/contact menu, if selected
	AddMainMenu();
	AddContactMenu();

	// create statistics on startup, if activated
	if (g_pSettings->m_OnStartup)
		Statistic::run(*g_pSettings, Statistic::fromStartup, g_hInst);

	return 0;
}

/*
 * external interface
 */

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
		g_hInst = hinstDLL;
		break;
	}

	return TRUE;
}

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	OutputDebugString(_T("HistoryStats: MirandaPluginInfoEx() was called.\n"));

	// MEMO: (don't) fail, if version is below minimum
	return &g_pluginInfoEx;
}

extern "C" __declspec(dllexport) int Load()
{
	mir_getLP(&g_pluginInfoEx);
	mir_getCLI();

	// init COM, needed for GUID generation
	CoInitialize(NULL);

	// register our own window classes
	if (!BandCtrlImpl::registerClass() || !OptionsCtrlImpl::registerClass())
	{
		MessageBox(
			0,
			_T("Failed to register a required window class. Can't continue loading plugin."),
			_T("HistoryStats - Error"),
			MB_OK | MB_ICONERROR);

		return 1;
	}
    
	// load "mu" system (includes version check)
	if (!mu::load())
	{
		MessageBox(
			0,
			_T("This version of HistoryStats isn't compatible with your Miranda NG ")
			_T("version. Possibly, your Miranda NG is outdated or you are trying to ")
			_T("use the Unicode version with a non-Unicode Miranda NG.\r\n\r\n")
			_T("Please go to the plugin's homepage and check the requirements."),
			_T("HistoryStats - Error"),
			MB_OK | MB_ICONERROR);

		return 1;
	}

	// use system's locale to format date/time/numbers
	Locale::init();

	// load rtfconv.dll if available
	RTFFilter::init();
	
	// init global variables
	g_bMainMenuExists = false;
	g_bContactMenuExists = false;

	// register provided services
	CreateServiceFunction(MS_HISTORYSTATS_ISEXCLUDED, SvcIsExcluded);
	CreateServiceFunction(MS_HISTORYSTATS_SETEXCLUDE, SvcSetExclude);

	// hook "modules loaded" to perform further initialization
	HookEvent(ME_SYSTEM_MODULESLOADED, EventModulesLoaded);

	return 0;
}

extern "C" __declspec(dllexport) int Unload()
{
	// free global settings object
	delete g_pSettings;

	// uninit iconlib
	IconLib::uninit();

	// free rtfconv.dll if loaded
	RTFFilter::uninit();

	// unload "mu" system
	mu::unload();

	// unregister our own window classes
	OptionsCtrlImpl::unregisterClass();
	BandCtrlImpl::unregisterClass();

	// uninit COM, needed for GUID generation
	CoUninitialize();
	return 0;
}
