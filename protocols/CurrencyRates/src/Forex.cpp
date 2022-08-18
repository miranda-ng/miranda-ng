// Forex.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#define MS_FOREX_ENABLE "CurrencyRates/Enable-Disable Auto Update"
#define MS_FOREX_CONVERTER "CurrencyRates/CurrencyConverter"

#define DB_STR_AUTO_UPDATE "AutoUpdate"

CMPlugin	g_plugin;

HANDLE g_hEventWorkThreadStop;

bool g_bAutoUpdate = true;
HGENMENU g_hMenuEditSettings = nullptr;
HGENMENU g_hMenuOpenLogFile = nullptr;
#ifdef CHART_IMPLEMENT
HGENMENU g_hMenuChart = nullptr;
#endif
HGENMENU g_hMenuRefresh = nullptr, g_hMenuRoot = nullptr;

typedef std::vector<HANDLE> THandles;
static THandles g_ahThreads;

static HGENMENU g_hEnableDisableMenu;
static HANDLE g_hTBButton;

static void UpdateMenu()
{
	if (g_bAutoUpdate) // to enable auto-update
		Menu_ModifyItem(g_hEnableDisableMenu, LPGENW("Auto Update Enabled"), g_plugin.getIconHandle(IDI_ICON_MAIN));
	else // to disable auto-update
		Menu_ModifyItem(g_hEnableDisableMenu, LPGENW("Auto Update Disabled"), g_plugin.getIconHandle(IDI_ICON_DISABLED));

	CallService(MS_TTB_SETBUTTONSTATE, reinterpret_cast<WPARAM>(g_hTBButton), g_bAutoUpdate ? 0 : TTBST_PUSHED);
}

static INT_PTR CurrencyRatesMenu_RefreshAll(WPARAM, LPARAM)
{
	for (auto &pProvider : g_apProviders)
		pProvider->RefreshAllContacts();
	return 0;
}

static INT_PTR CurrencyRatesMenu_EnableDisable(WPARAM, LPARAM)
{
	g_bAutoUpdate = !g_bAutoUpdate;
	g_plugin.setByte(DB_STR_AUTO_UPDATE, g_bAutoUpdate);

	for (auto &pProvider : g_apProviders) {
		pProvider->RefreshSettings();
		if (g_bAutoUpdate)
			pProvider->RefreshAllContacts();
	}

	UpdateMenu();
	return 0;
}

void InitMenu()
{
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Currency Rates"), 0, g_plugin.getIconHandle(IDI_ICON_MAIN));
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "B474F556-22B6-42A1-A91E-22FE4F671388");

	SET_UID(mi, 0x9de6716, 0x3591, 0x48c4, 0x9f, 0x64, 0x1b, 0xfd, 0xc6, 0xd1, 0x34, 0x97);
	mi.name.w = LPGENW("Enable/Disable Auto Update");
	mi.position = 10100001;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON_MAIN);
	mi.pszService = MS_FOREX_ENABLE;
	g_hEnableDisableMenu = Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, CurrencyRatesMenu_EnableDisable);
	UpdateMenu();

	SET_UID(mi, 0x91cbabf6, 0x5073, 0x4a78, 0x84, 0x8, 0x34, 0x61, 0xc1, 0x8a, 0x34, 0xd9);
	mi.name.w = LPGENW("Refresh All Rates");
	mi.position = 20100001;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON_MAIN);
	mi.pszService = "CurrencyRates/RefreshAll";
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, CurrencyRatesMenu_RefreshAll);

	SET_UID(mi, 0x3663409c, 0xbd36, 0x473b, 0x9b, 0x4f, 0xff, 0x80, 0xf6, 0x2c, 0xdf, 0x9b);
	mi.name.w = LPGENW("Currency Converter...");
	mi.position = 20100002;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON_CURRENCY_CONVERTER);
	mi.pszService = MS_FOREX_CONVERTER;
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, CurrencyRatesMenu_CurrencyConverter);

	SET_UID(mi, 0x7cca4fd9, 0x903f, 0x4b7d, 0x93, 0x7a, 0x18, 0x63, 0x23, 0xd4, 0xa9, 0xa9);
	mi.name.w = LPGENW("Export All Currency Rates");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON_EXPORT);
	mi.pszService = MS_CURRENCYRATES_EXPORT;
	mi.position = 20100003;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xa994d3b, 0x77c2, 0x4612, 0x8d, 0x5, 0x6a, 0xae, 0x8c, 0x21, 0xbd, 0xc9);
	mi.name.w = LPGENW("Import All Currency Rates");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON_IMPORT);
	mi.pszService = MS_CURRENCYRATES_IMPORT;
	mi.position = 20100004;
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, CurrencyRates_PrebuildContactMenu);

	g_hMenuRoot = mi.root = g_plugin.addRootMenu(MO_CONTACT, LPGENW("Currency Rates"), 0, g_plugin.getIconHandle(IDI_ICON_MAIN));
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "C259BE01-642C-461E-997D-0E756B2A3AD6");

	SET_UID(mi, 0xb9812194, 0x3235, 0x4e76, 0xa3, 0xa4, 0x73, 0x32, 0x96, 0x1c, 0x1c, 0xf4);
	mi.name.w = LPGENW("Refresh");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON_REFRESH);
	mi.pszService = "CurrencyRates/RefreshContact";
	g_hMenuRefresh = Menu_AddContactMenuItem(&mi, MODULENAME);
	Menu_ConfigureItem(g_hMenuRefresh, MCI_OPT_EXECPARAM, INT_PTR(0));
	CreateServiceFunction(mi.pszService, CurrencyRatesMenu_RefreshContact);

	SET_UID(mi, 0x19a16fa2, 0xf370, 0x4201, 0x92, 0x9, 0x25, 0xde, 0x4e, 0x55, 0xf9, 0x1a);
	mi.name.w = LPGENW("Open Log File...");
	mi.hIcolibItem = nullptr;
	mi.pszService = "CurrencyRates/OpenLogFile";
	g_hMenuOpenLogFile = Menu_AddContactMenuItem(&mi, MODULENAME);
	Menu_ConfigureItem(g_hMenuOpenLogFile, MCI_OPT_EXECPARAM, 1);
	CreateServiceFunction(mi.pszService, CurrencyRatesMenu_OpenLogFile);

	#ifdef CHART_IMPLEMENT
	SET_UID(mi, 0x65da7256, 0x43a2, 0x4857, 0xac, 0x52, 0x1c, 0xb7, 0xff, 0xd7, 0x96, 0xfa);
	mi.name.w = LPGENW("Chart...");
	mi.hIcolibItem = nullptr;
	mi.pszService = "CurrencyRates/Chart";
	g_hMenuChart = Menu_AddContactMenuItem(&mi, MODULENAME);
	CreateServiceFunction(mi.pszService, CurrencyRatesMenu_Chart);
	#endif

	SET_UID(mi, 0xac5fc17, 0x5640, 0x4f81, 0xa3, 0x44, 0x8c, 0xb6, 0x9a, 0x5c, 0x98, 0xf);
	mi.name.w = LPGENW("Edit Settings...");
	mi.hIcolibItem = nullptr;
	mi.pszService = "CurrencyRates/EditSettings";
	g_hMenuEditSettings = Menu_AddContactMenuItem(&mi, MODULENAME);
	#ifdef CHART_IMPLEMENT
	Menu_ConfigureItem(g_hMenuEditSettings, MCI_OPT_EXECPARAM, 3);
	#else
	Menu_ConfigureItem(g_hMenuEditSettings, MCI_OPT_EXECPARAM, 2);
	#endif
	CreateServiceFunction(mi.pszService, CurrencyRatesMenu_EditSettings);
}

int CurrencyRates_OnToolbarLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.name = LPGEN("Enable/Disable Currency Rates Auto Update");
	ttb.pszService = MS_FOREX_ENABLE;
	ttb.pszTooltipUp = LPGEN("Currency Rates Auto Update Enabled");
	ttb.pszTooltipDn = LPGEN("Currency Rates Auto Update Disabled");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_ICON_MAIN);
	ttb.hIconHandleDn = g_plugin.getIconHandle(IDI_ICON_DISABLED);
	ttb.dwFlags = ((g_bAutoUpdate) ? 0 : TTBBF_PUSHED) | TTBBF_ASPUSHBUTTON | TTBBF_VISIBLE;
	g_hTBButton = g_plugin.addTTB(&ttb);

	ttb.name = LPGEN("Currency Converter");
	ttb.pszService = MS_FOREX_CONVERTER;
	ttb.pszTooltipUp = LPGEN("Currency Converter");
	ttb.pszTooltipDn = LPGEN("Currency Converter");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_ICON_CURRENCY_CONVERTER);
	ttb.hIconHandleDn = g_plugin.getIconHandle(IDI_ICON_CURRENCY_CONVERTER);
	ttb.dwFlags = TTBBF_VISIBLE;
	g_plugin.addTTB(&ttb);

	return 0;
}

static void WorkingThread(void *pParam)
{
	ICurrencyRatesProvider *pProvider = reinterpret_cast<ICurrencyRatesProvider*>(pParam);
	assert(pProvider);

	if (pProvider)
		pProvider->Run();
}

int CurrencyRatesEventFunc_OnModulesLoaded(WPARAM, LPARAM)
{
	CHTTPSession::Init();

	g_hEventWorkThreadStop = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
	HookEvent(ME_USERINFO_INITIALISE, CurrencyRatesEventFunc_OnUserInfoInit);

	HookEvent(ME_CLIST_DOUBLECLICKED, CurrencyRates_OnContactDoubleClick);

	HookEvent(ME_TTB_MODULELOADED, CurrencyRates_OnToolbarLoaded);

	g_bAutoUpdate = 1 == g_plugin.getByte(DB_STR_AUTO_UPDATE, 1);

	InitMenu();

	::ResetEvent(g_hEventWorkThreadStop);

	for (auto &pProvider : g_apProviders)
		g_ahThreads.push_back(mir_forkthread(WorkingThread, pProvider));
	return 0;
}

int CurrencyRatesEventFunc_OnContactDeleted(WPARAM hContact, LPARAM)
{
	auto pProvider = GetContactProviderPtr(hContact);
	if (pProvider)
		pProvider->DeleteContact(hContact);
	return 0;
}

INT_PTR CurrencyRateProtoFunc_GetCaps(WPARAM wParam, LPARAM)
{
	switch (wParam) {
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("Currency Symbol");
	}

	return 0;
}

INT_PTR CurrencyRateProtoFunc_GetStatus(WPARAM, LPARAM)
{
	return g_bAutoUpdate ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
}

void WaitForWorkingThreads()
{
	size_t cThreads = g_ahThreads.size();
	if (cThreads > 0) {
		HANDLE* paHandles = &*(g_ahThreads.begin());
		::WaitForMultipleObjects((uint32_t)cThreads, paHandles, TRUE, INFINITE);
	}
}

int CurrencyRatesEventFunc_PreShutdown(WPARAM, LPARAM)
{
	::SetEvent(g_hEventWorkThreadStop);

	CModuleInfo::OnMirandaShutdown();
	return 0;
}

int CurrencyRatesEventFunc_OptInitialise(WPARAM wp, LPARAM/* lp*/)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.szTitle.w = LPGENW("Currency Rates");
	odp.szGroup.w = LPGENW("Network");
	odp.flags = ODPF_USERINFOTAB | ODPF_UNICODE;

	for (auto &it : g_apProviders)
		it->ShowPropertyPage(wp, odp);
	return 0;
}

inline int CurrencyRates_UnhookEvent(HANDLE h)
{
	return UnhookEvent(h);
}

/////////////////////////////////////////////////////////////////////////////////////////

EXTERN_C __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {E882056D-0D1D-4131-9A98-404CBAEA6A9C}
	{ 0xe882056d, 0xd1d, 0x4131, { 0x9a, 0x98, 0x40, 0x4c, 0xba, 0xea, 0x6a, 0x9c } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
	RegisterProtocol(PROTOTYPE_VIRTUAL);
	SetUniqueId(DB_STR_CURRENCYRATE_SYMBOL);
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"),             "main",                    IDI_ICON_MAIN               },
	{ LPGEN("Auto Update Disabled"),      "auto_update_disabled",    IDI_ICON_DISABLED           },
	{ LPGEN("Currency Rate up"),          "currencyrate_up",         IDI_ICON_UP                 },
	{ LPGEN("Currency Rate down"),        "currencyrate_down",       IDI_ICON_DOWN               },
	{ LPGEN("Currency Rate not changed"), "currencyrate_not_changed",IDI_ICON_NOTCHANGED         },
	{ LPGEN("Currency Rate Section"),     "currencyrate_section",    IDI_ICON_SECTION            },
	{ LPGEN("Currency Rate"),             "currencyrate",            IDI_ICON_CURRENCYRATE       },
	{ LPGEN("Currency Converter"),        "currency_converter",      IDI_ICON_CURRENCY_CONVERTER },
	{ LPGEN("Refresh"),                   "refresh",                 IDI_ICON_REFRESH            },
	{ LPGEN("Export"),                    "export",                  IDI_ICON_EXPORT             },
	{ LPGEN("Swap button"),               "swap",                    IDI_ICON_SWAP               },
	{ LPGEN("Import"),                    "import",                  IDI_ICON_IMPORT             }
};

int CMPlugin::Load(void)
{
	registerIcon(MODULENAME, iconList, MODULENAME);

	CurrencyRates_InitExtraIcons();

	InitProviders();

	CreateProtoServiceFunction(MODULENAME, PS_GETCAPS, CurrencyRateProtoFunc_GetCaps);
	CreateProtoServiceFunction(MODULENAME, PS_GETSTATUS, CurrencyRateProtoFunc_GetStatus);

	HookEvent(ME_SYSTEM_MODULESLOADED, CurrencyRatesEventFunc_OnModulesLoaded);
	HookEvent(ME_DB_CONTACT_DELETED, CurrencyRatesEventFunc_OnContactDeleted);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, CurrencyRatesEventFunc_PreShutdown);
	HookEvent(ME_OPT_INITIALISE, CurrencyRatesEventFunc_OptInitialise);

	CreateServiceFunction(MS_CURRENCYRATES_EXPORT, CurrencyRates_Export);
	CreateServiceFunction(MS_CURRENCYRATES_IMPORT, CurrencyRates_Import);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload(void)
{
	WaitForWorkingThreads();

	ClearProviders();
	::CloseHandle(g_hEventWorkThreadStop);
	return 0;
}
