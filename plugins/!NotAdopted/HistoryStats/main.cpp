#include "_globals.h"
#include "main.h"

#include <clocale>

#include "dlgoption.h"
#include "resource.h"
#include "column.h"
#include "bandctrlimpl.h"
#include "optionsctrlimpl.h"
#include "themeapi.h"
#include "iconlib.h"
#include "dlgconfigure.h"

HINSTANCE g_hInst;

static const int g_pluginFileListID = MU_DO_BOTH(2534, 2535);
static const MUUID g_MPIDHistoryStatsA = { 0x287dd477, 0xf217, 0x4482, { 0xab, 0x88, 0x60, 0x0d, 0x2e, 0x8d, 0x31, 0x0f } };
static const MUUID g_MPIDHistoryStatsW = { 0xf184f5a0, 0xc198, 0x4454, { 0xa9, 0xb4, 0xf6, 0xe2, 0xfd, 0x53, 0x41, 0x33 } };

const PLUGININFOEX g_pluginInfoEx = {
	/* .cbSize                = */ sizeof(PLUGININFOEX),
	/* .shortName             = */ MU_DO_BOTH(muA("HistoryStats"), muA("HistoryStats (Unicode)")),
	/* .version               = */ PLUGIN_MAKE_VERSION(0, 1, 5, 3),
	/* .description           = */ muA("Creates nice statistics using your message history.\r\n")
	                               muA("(Requires Miranda IM ") MU_DO_BOTH(muA("0.6.7"), muA("0.6.7 Unicode")) muA(" or above.)"),
	/* .author                = */ muA("Martin Afanasjew"),
	/* .authorEmail           = */ muA("miranda@dark-passage.de"),	
	/* .copyright             = */ muA("2005-2007 by Martin Afanasjew (see README for further credits)"),
	/* .homepage              = */ muA("http://addons.miranda-im.org/details.php?action=viewfile&id=") MU_DO_BOTH(muA("2534"), muA("2535")),
	/* .flags                 = */ MU_DO_BOTH(0, UNICODE_AWARE),
	/* .replacesDefaultModule = */ 0,
	/* .uuid                  = */ MU_DO_BOTH(g_MPIDHistoryStatsA, g_MPIDHistoryStatsW),
};

const PLUGININFO g_pluginInfo = {
	/* .cbSize                = */ sizeof(PLUGININFO),
	/* .shortName             = */ g_pluginInfoEx.shortName,
	/* .version               = */ g_pluginInfoEx.version,
	/* .description           = */ g_pluginInfoEx.description,
	/* .author                = */ g_pluginInfoEx.author,
	/* .authorEmail           = */ g_pluginInfoEx.authorEmail,
	/* .copyright             = */ g_pluginInfoEx.copyright,
	/* .homepage              = */ g_pluginInfoEx.homepage,
	/* .flags                 = */ g_pluginInfoEx.flags,
	/* .replacesDefaultModule = */ g_pluginInfoEx.replacesDefaultModule,
};

SettingsSerializer* g_pSettings = NULL;

bool g_bMainMenuExists    = false;
bool g_bContactMenuExists = false;
bool g_bExcludeLock       = false;
bool g_bConfigureLock     = false;

std::vector<HANDLE> g_hRegisteredServices;
std::vector<HANDLE> g_hHookedEvents;

static HANDLE g_hMenuCreateStatistics = NULL;
static HANDLE g_hMenuShowStatistics   = NULL;
static HANDLE g_hMenuConfigure        = NULL;
static HANDLE g_hMenuToggleExclude    = NULL;

#if defined(HISTORYSTATS_HISTORYCOPY)
static HANDLE g_hMenuHistoryCopy    = NULL;
static HANDLE g_hMenuHistoryPaste   = NULL;

static HANDLE g_hHistoryCopyContact = NULL;
#endif

/*
 * services (see m_historystats.h for details)
 */

static int SvcIsExcluded(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	if (hContact)
	{
		MirandaSettings db;

		db.setContact(hContact);
		db.setModule(con::ModHistoryStats);

		return db.readBool(con::SettExclude, false) ? 1 : 0;
	}

	return 0;
}

static int SvcSetExclude(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	if (hContact)
	{
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

static void MenuIconsChanged(LPARAM lParam)
{
	if (g_hMenuCreateStatistics)
	{
		mu::clist::modifyMenuItem(g_hMenuCreateStatistics, CMIM_ICON, NULL, 0, IconLib::getIcon(IconLib::iiMenuCreateStatistics));
	}

	if (g_hMenuShowStatistics)
	{
		mu::clist::modifyMenuItem(g_hMenuShowStatistics, CMIM_ICON, NULL, 0, IconLib::getIcon(IconLib::iiMenuShowStatistics));
	}

	if (g_hMenuConfigure)
	{
		mu::clist::modifyMenuItem(g_hMenuConfigure, CMIM_ICON, NULL, 0, IconLib::getIcon(IconLib::iiMenuConfigure));
	}

	if (g_hMenuToggleExclude)
	{
		mu::clist::modifyMenuItem(g_hMenuToggleExclude, CMIM_ICON, NULL, 0, IconLib::getIcon(IconLib::iiContactMenu));
	}
}

/*
 * main menu related stuff
 */

static int MenuCreateStatistics(WPARAM wParam, LPARAM lParam)
{
	Statistic::run(*g_pSettings, Statistic::fromMenu, g_hInst);

	return 0;
}

static int MenuShowStatistics(WPARAM wParam, LPARAM lParam)
{
	if (g_pSettings->canShowStatistics())
	{
		g_pSettings->showStatistics();
	}
	else
	{
		MessageBox(
			0,
			i18n(muT("The statistics can't be found. Either you never created them or the last created statistics were moved to a different location and can't be found anymore.")),
			i18n(muT("HistoryStats - Warning")),
			MB_ICONWARNING | MB_OK);
	}

	return 0;
}

static int MenuConfigure(WPARAM wParam, LPARAM lParam)
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

	g_hRegisteredServices << mu::_link::createServiceFunction(con::SvcCreateStatistics, MenuCreateStatistics);
	g_hRegisteredServices << mu::_link::createServiceFunction(con::SvcShowStatistics  , MenuShowStatistics  );
	g_hRegisteredServices << mu::_link::createServiceFunction(con::SvcConfigure       , MenuConfigure       );

	g_hMenuCreateStatistics = mu::clist::addMainMenuItem(
		I18N(muT("Create statistics")), // MEMO: implicit translation
		0,
		1910000000,
		IconLib::getIcon(IconLib::iiMenuCreateStatistics),
		con::SvcCreateStatistics,
		bInPopup ? I18N(muT("Statistics")) : NULL, // MEMO: implicit translation
		bInPopup ? 1910000000 : 0);

	g_hMenuShowStatistics = mu::clist::addMainMenuItem(
		I18N(muT("Show statistics")), // MEMO: implicit translation
		0,
		1910000001,
		IconLib::getIcon(IconLib::iiMenuShowStatistics),
		con::SvcShowStatistics,
		bInPopup ? I18N(muT("Statistics")) : NULL, // MEMO: implicit translation
		bInPopup ? 1910000000 : 0);

	g_hMenuConfigure = mu::clist::addMainMenuItem(
		bInPopup ? I18N(muT("Configure...")) : I18N(muT("Configure statistics...")), // MEMO: implicit translation
		0,
		1910000002,
		IconLib::getIcon(IconLib::iiMenuConfigure),
		con::SvcConfigure,
		bInPopup ? I18N(muT("Statistics")) : NULL, // MEMO: implicit translation
		bInPopup ? 1910000000 : 0);
}

/*
 * contact menu related stuff
 */

static int MenuToggleExclude(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	if (hContact)
	{
		MirandaSettings db;

		db.setContact(hContact);
		db.setModule(con::ModHistoryStats);

		if (db.readBool(con::SettExclude, false))
		{
			db.delSetting(con::SettExclude);
		}
		else
		{
			db.writeBool(con::SettExclude, true);
		}
	}

	return 0;
}

#if defined(HISTORYSTATS_HISTORYCOPY)
static int MenuHistoryCopy(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	if (hContact)
	{
		g_hHistoryCopyContact = hContact;
	}

	return 0;
}

static int MenuHistoryPaste(WPARAM wParam, LPARAM lParam)
{
	HANDLE hTarget = reinterpret_cast<HANDLE>(wParam);

	if (!hTarget)
	{
		return 0;
	}

	// ask user if this is really what he wants
	ext::string strConfirm = ext::str(ext::kformat(i18n(muT("You're going to copy the complete history of #{source_name} (#{source_proto}) to #{target_name} (#{target_proto}). Afterwards, the target history will contain entries from both histories. There is no way to revert this operation. Be careful! This is a rather big operation and has the potential to damage your database. Be sure to have a backup of this database before performing this operation.\r\n\r\nAre you sure you would like to continue?")))
		% muT("#{source_name}") * mu::clist::getContactDisplayName(g_hHistoryCopyContact)
		% muT("#{source_proto}") * utils::fromA(mu::proto::getContactBaseProto(g_hHistoryCopyContact))
		% muT("#{target_name}") * mu::clist::getContactDisplayName(hTarget)
		% muT("#{target_proto}") * utils::fromA(mu::proto::getContactBaseProto(hTarget)));

	if (MessageBox(0, strConfirm.c_str(), i18n(muT("HistoryStats - Confirm")), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES)
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

	ZeroMemory(&dbe, sizeof(dbe));
	dbe.cbSize = sizeof(dbe);
	dbe.pBlob = reinterpret_cast<BYTE*>(malloc(blobBuffer));

	while (hEvent)
	{
		dbe.cbBlob = mu::db_event::getBlobSize(hEvent);

		if (blobBuffer < dbe.cbBlob)
		{
			blobBuffer = 4096 * ((4095 + dbe.cbBlob) / 4096);
			dbe.pBlob = reinterpret_cast<BYTE*>(realloc(dbe.pBlob, blobBuffer));
		}

		if (mu::db_event::get(hEvent, &dbe) == 0)
		{
			++dwCountSuccess;

			// clear "first" flag
			dbe.flags &= ~DBEF_FIRST;

			if (mu::db_event::add(hTarget, &dbe) == NULL)
			{
				++dwCountFailAdd;
			}
		}
		else
		{
			++dwCountFailRead;
		}

		hEvent = mu::db_event::findNext(hEvent);
	}

	free(dbe.pBlob);

	// turn safety mode back on
	mu::db::setSafetyMode(true);

	// output summary
	ext::string strSummary = ext::str(ext::kformat(i18n(muT("Successfully read #{success} events of which #{fail_add} couldn't be added to the target history. #{fail} events couldn't be read from the source history.")))
		% muT("#{success}") * dwCountSuccess
		% muT("#{fail}") * dwCountFailRead
		% muT("#{fail_add}") * dwCountFailAdd);

	MessageBox(0, strSummary.c_str(), i18n(muT("HistoryStats - Information")), MB_ICONINFORMATION);

	g_hHistoryCopyContact = NULL;

	return 0;
}
#endif

static int EventPreBuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	if (hContact)
	{
		const mu_ansi* szProto = mu::proto::getContactBaseProto(hContact);

		if ((!g_pSettings->m_ShowContactMenuPseudo && (!szProto || !(mu::protosvc::getCaps(szProto, PFLAGNUM_2) & ~mu::protosvc::getCaps(szProto, PFLAGNUM_5)))) ||
			g_pSettings->m_HideContactMenuProtos.find(szProto) != g_pSettings->m_HideContactMenuProtos.end())
		{
			mu::clist::modifyMenuItem(g_hMenuToggleExclude, CMIM_FLAGS, NULL, CMIF_HIDDEN);
		}
		else
		{
			MirandaSettings db;

			db.setContact(hContact);
			db.setModule(con::ModHistoryStats);

			int menuState = db.readBool(con::SettExclude, false) ? CMIF_CHECKED : 0;
			
			// avoid collision with options page
			if (g_bExcludeLock)
			{
				menuState |= CMIF_GRAYED;
			}

			// set menu state
			mu::clist::modifyMenuItem(g_hMenuToggleExclude, CMIM_FLAGS, NULL, menuState);
		}

#if defined(HISTORYSTATS_HISTORYCOPY)
		int menuStateCopy = (g_hHistoryCopyContact && g_hHistoryCopyContact != hContact) ? 0 : CMIF_GRAYED;

		mu::clist::modifyMenuItem(g_hMenuHistoryPaste, CMIM_FLAGS, NULL, menuStateCopy);
#endif
	}

	return 0;
}

void AddContactMenu()
{
	if (!g_pSettings->m_ShowContactMenu || g_bContactMenuExists)
	{
		return;
	}

	g_bContactMenuExists = true;

	g_hRegisteredServices << mu::_link::createServiceFunction(con::SvcToggleExclude, MenuToggleExclude);

	g_hMenuToggleExclude = mu::clist::addContactMenuItem(
		I18N(muT("Exclude from statistics")), // MEMO: implicit translation
		0,
		800000,
		IconLib::getIcon(IconLib::iiContactMenu),
		con::SvcToggleExclude);

#if defined(HISTORYSTATS_HISTORYCOPY)
	g_hRegisteredServices << mu::_link::createServiceFunction(con::SvcHistoryCopy , MenuHistoryCopy );
	g_hRegisteredServices << mu::_link::createServiceFunction(con::SvcHistoryPaste, MenuHistoryPaste);

	g_hMenuHistoryCopy = mu::clist::addContactMenuItem(
		I18N(muT("Copy history")), // MEMO: implicit translation
		0,
		800001,
		NULL,
		con::SvcHistoryCopy);

	g_hMenuHistoryPaste = mu::clist::addContactMenuItem(
		I18N(muT("Paste history...")), // MEMO: implicit translation
		0,
		800002,
		NULL,
		con::SvcHistoryPaste);
#endif

	g_hHookedEvents << mu::_link::hookEvent(ME_CLIST_PREBUILDCONTACTMENU, EventPreBuildContactMenu);
}

/*
 * options integration
 */

static int EventOptInitialise(WPARAM wParam, LPARAM lParam)
{
	mu::opt::addPage(
		wParam,
		i18n(muT("History")),
		i18n(muT("Statistics")),
		NULL,
		DlgOption::staticDlgProc,
		MAKEINTRESOURCEA(IDD_OPTIONS),
		g_hInst);

	return 0;
}

/*
 * second initialization phase
 */

static int EventModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	// register all known columns
	Column::registerColumns();

	// read settings
	g_pSettings = new SettingsSerializer(con::ModHistoryStats);
	g_pSettings->readFromDB();

	// integrate into options dialog
	g_hHookedEvents << mu::_link::hookEvent(ME_OPT_INITIALISE, EventOptInitialise);

	// integrate with icolib
	IconLib::init();
	IconLib::registerCallback(MenuIconsChanged, 0);

	// register with Updater plugin, if available
	if (mu::updater::_available())
	{
		mu::updater::registerFL(g_pluginFileListID, &g_pluginInfo);
	}
	
	// integrate into main/contact menu, if selected
	AddMainMenu();
	AddContactMenu();

	// create statistics on startup, if activated
	if (g_pSettings->m_OnStartup)
	{
		Statistic::run(*g_pSettings, Statistic::fromStartup, g_hInst);
	}

	return 0;
}

/*
 * external interface
 */

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			{
				DisableThreadLibraryCalls(hinstDLL);
				g_hInst = hinstDLL;

#if defined(_DEBUG)
				// dump memory leak report at end of program
				_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
			}
			break;
	}

	return TRUE;
}

extern "C" __declspec(dllexport) const PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	OutputDebugString(muT("HistoryStats: MirandaPluginInfo() was called.\n"));

	// MEMO: (don't) fail, if version is below minimum
	return &g_pluginInfo;
}

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	OutputDebugString(muT("HistoryStats: MirandaPluginInfoEx() was called.\n"));

	// MEMO: (don't) fail, if version is below minimum
	return &g_pluginInfoEx;
}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces()
{
	OutputDebugString(muT("HistoryStats: MirandaPluginInterfaces() was called.\n"));

	static const MUUID interfaces[] = {
		MIID_HISTORYSTATS,
		MIID_LAST,
	};

	return interfaces;
}

extern "C" __declspec(dllexport) int Load(PLUGINLINK* link)
{
	// init themeing api
	ThemeAPI::init();

	// init COM, needed for GUID generation
	CoInitialize(NULL);

	// register our own window classes
	if (!BandCtrlImpl::registerClass() || !OptionsCtrlImpl::registerClass())
	{
		MessageBox(
			0,
			muT("Failed to register a required window class. Can't continue loading plugin."),
			muT("HistoryStats - Error"),
			MB_OK | MB_ICONERROR);

		return 1;
	}
    
	// load "mu" system (includes version check)
	if (!mu::load(link))
	{
		MessageBox(
			0,
			muT("This version of HistoryStats isn't compatible with your Miranda IM ")
			muT("version. Possibly, your Miranda IM is outdated or you are trying to ")
			muT("use the Unicode version with a non-Unicode Miranda IM.\r\n\r\n")
			muT("Please go to the plugin's homepage and check the requirements."),
			muT("HistoryStats - Error"),
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
	g_hRegisteredServices << mu::_link::createServiceFunction(MS_HISTORYSTATS_ISEXCLUDED, SvcIsExcluded);
	g_hRegisteredServices << mu::_link::createServiceFunction(MS_HISTORYSTATS_SETEXCLUDE, SvcSetExclude);

	// hook "modules loaded" to perform further initialization
	g_hHookedEvents << mu::_link::hookEvent(ME_SYSTEM_MODULESLOADED, EventModulesLoaded);

	return 0;
}

extern "C" __declspec(dllexport) int Unload()
{
	// unhook previously hooked events
	std::for_each(g_hHookedEvents.begin(), g_hHookedEvents.end(), std::ptr_fun(mu::_link::unhookEvent));
	g_hHookedEvents.clear();

	// unregister previously registered services
	std::for_each(g_hRegisteredServices.begin(), g_hRegisteredServices.end(), std::ptr_fun(mu::_link::destroyServiceFunction));
	g_hRegisteredServices.clear();

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

	// free themeing api
	ThemeAPI::uninit();

	return 0;
}
