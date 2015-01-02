// Forex.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

int hLangpack;
HINSTANCE g_hInstance = NULL;
HANDLE g_hEventWorkThreadStop;
//int g_nStatus = ID_STATUS_OFFLINE;
bool g_bAutoUpdate = true;
HGENMENU g_hMenuEditSettings = NULL;
HGENMENU g_hMenuOpenLogFile = NULL;
#ifdef CHART_IMPLEMENT
HGENMENU g_hMenuChart = NULL;
#endif
HGENMENU g_hMenuRefresh = NULL;

#define DB_STR_AUTO_UPDATE "AutoUpdate"

namespace
{
	typedef std::vector<HANDLE> THandles;
	THandles g_ahThreads;
	HGENMENU g_hEnableDisableMenu;
	HANDLE g_hTBButton;

	LPSTR g_pszAutoUpdateCmd = "Quotes/Enable-Disable Auto Update";
	LPSTR g_pszCurrencyConverter = "Quotes/CurrencyConverter";

	PLUGININFOEX Global_pluginInfo =
	{
		sizeof(PLUGININFOEX),
		__PLUGIN_NAME,
		PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
		__DESCRIPTION,
		__AUTHOR,
		__AUTHOREMAIL,
		__COPYRIGHT,
		__AUTHORWEB,
		UNICODE_AWARE,
		// {E882056D-0D1D-4131-9A98-404CBAEA6A9C}
		{ 0xe882056d, 0xd1d, 0x4131, { 0x9a, 0x98, 0x40, 0x4c, 0xba, 0xea, 0x6a, 0x9c } }
	};

	void UpdateMenu(bool bAutoUpdate)
	{
		CLISTMENUITEM mi = { sizeof(mi) };

		if (bAutoUpdate) { // to enable auto-update
			mi.pszName = LPGEN("Auto Update Enabled");
			mi.icolibItem = Quotes_GetIconHandle(IDI_ICON_MAIN);
			//opt.AutoUpdate = 1;
		}
		else { // to disable auto-update
			mi.pszName = LPGEN("Auto Update Disabled");
			mi.icolibItem = Quotes_GetIconHandle(IDI_ICON_DISABLED);
			//opt.AutoUpdate = 0;
		}

		mi.flags = CMIM_ICON | CMIM_NAME;
		Menu_ModifyItem(g_hEnableDisableMenu, &mi);
		CallService(MS_TTB_SETBUTTONSTATE, reinterpret_cast<WPARAM>(g_hTBButton), !bAutoUpdate ? TTBST_PUSHED : 0);
	}


	// 	INT_PTR QuoteProtoFunc_SetStatus(WPARAM wp,LPARAM /*lp*/)
	// 	{
	// 		if ((ID_STATUS_ONLINE == wp) || (ID_STATUS_OFFLINE == wp))
	// 		{
	// 			bool bAutoUpdate = (ID_STATUS_ONLINE == wp);
	// 			bool bOldFlag = g_bAutoUpdate;
	// 
	// 			if(bAutoUpdate != g_bAutoUpdate)
	// 			{
	// 				g_bAutoUpdate = bAutoUpdate;
	// 				db_set_b(NULL,QUOTES_MODULE_NAME,DB_STR_AUTO_UPDATE,g_bAutoUpdate);
	// 				if (bOldFlag && !g_bAutoUpdate)
	// 				{
	// 					BOOL b = ::SetEvent(g_hEventWorkThreadStop);
	// 					assert(b);
	// 				}
	// 				else if (g_bAutoUpdate && !bOldFlag)
	// 				{
	// 					BOOL b = ::ResetEvent(g_hEventWorkThreadStop);
	// 					assert(b && "Failed to reset event");
	// 
	// 					const CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
	// 					const CQuotesProviders::TQuotesProviders& rapProviders = pProviders->GetProviders();
	// 					for(CQuotesProviders::TQuotesProviders::const_iterator i = rapProviders.begin();i != rapProviders.end();++i)
	// 					{
	// 						const CQuotesProviders::TQuotesProviderPtr& pProvider = *i;
	// 						g_ahThreads.push_back( mir_forkthread(WorkingThread, pProvider.get()));
	// 					}
	// 				}
	// 
	// 				UpdateMenu(g_bAutoUpdate);
	// 				//ProtoBroadcastAck(QUOTES_PROTOCOL_NAME,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,reinterpret_cast<HANDLE>(nOldStatus),g_nStatus);
	// 			}
	// 
	// 		}
	// 
	// 		return 0;
	// 	}

	INT_PTR QuotesMenu_RefreshAll(WPARAM, LPARAM)
	{
		const CQuotesProviders::TQuotesProviders& apProviders = CModuleInfo::GetQuoteProvidersPtr()->GetProviders();
		std::for_each(apProviders.begin(), apProviders.end(), boost::bind(&IQuotesProvider::RefreshAllContacts, _1));
		return 0;
	}

	INT_PTR QuotesMenu_EnableDisable(WPARAM, LPARAM)
	{
		//QuoteProtoFunc_SetStatus(g_bAutoUpdate ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE,0L);

		g_bAutoUpdate = (g_bAutoUpdate) ? false : true;
		db_set_b(NULL, QUOTES_MODULE_NAME, DB_STR_AUTO_UPDATE, g_bAutoUpdate);

		const CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
		const CQuotesProviders::TQuotesProviders& rapProviders = pProviders->GetProviders();
		std::for_each(std::begin(rapProviders), std::end(rapProviders), [](const CQuotesProviders::TQuotesProviderPtr& pProvider)
		{
			pProvider->RefreshSettings();
			if (g_bAutoUpdate)
			{
				pProvider->RefreshAllContacts();
			}
		});
		UpdateMenu(g_bAutoUpdate);

		return 0;
	}

	void InitMenu()
	{
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.ptszName = LPGENT("Quotes");
		mi.flags = CMIF_TCHAR | CMIF_ROOTPOPUP;
		mi.icolibItem = Quotes_GetIconHandle(IDI_ICON_MAIN);
		HGENMENU hMenuRoot = Menu_AddMainMenuItem(&mi);

		mi.ptszName = LPGENT("Enable/Disable Auto Update");
		mi.flags = CMIF_TCHAR | CMIF_ROOTHANDLE;
		mi.position = 10100001;
		mi.icolibItem = Quotes_GetIconHandle(IDI_ICON_MAIN);
		mi.pszService = g_pszAutoUpdateCmd;
		mi.hParentMenu = hMenuRoot;
		g_hEnableDisableMenu = Menu_AddMainMenuItem(&mi);
		CreateServiceFunction(mi.pszService, QuotesMenu_EnableDisable);
		UpdateMenu(g_bAutoUpdate);

		mi.ptszName = LPGENT("Refresh All Quotes\\Rates");
		mi.flags = CMIF_TCHAR | CMIF_ROOTHANDLE;
		mi.position = 20100001;
		mi.icolibItem = Quotes_GetIconHandle(IDI_ICON_MAIN);
		mi.pszService = "Quotes/RefreshAll";
		mi.hParentMenu = hMenuRoot;
		Menu_AddMainMenuItem(&mi);
		CreateServiceFunction(mi.pszService, QuotesMenu_RefreshAll);

		mi.ptszName = LPGENT("Currency Converter...");
		//mi.flags = CMIF_TCHAR|CMIF_ICONFROMICOLIB|CMIF_ROOTHANDLE;
		mi.position = 20100002;
		mi.icolibItem = Quotes_GetIconHandle(IDI_ICON_CURRENCY_CONVERTER);
		mi.pszService = g_pszCurrencyConverter;
		Menu_AddMainMenuItem(&mi);
		CreateServiceFunction(mi.pszService, QuotesMenu_CurrencyConverter);

		mi.ptszName = LPGENT("Export All Quotes");
		//mi.flags = CMIF_TCHAR|CMIF_ICONFROMICOLIB|CMIF_ROOTHANDLE;
		mi.icolibItem = Quotes_GetIconHandle(IDI_ICON_EXPORT);
		mi.pszService = "Quotes/ExportAll";
		mi.position = 20100003;
		Menu_AddMainMenuItem(&mi);
		CreateServiceFunction(mi.pszService, QuotesMenu_ExportAll);

		mi.ptszName = LPGENT("Import All Quotes");
		//mi.flags = CMIF_TCHAR|CMIF_ICONFROMICOLIB|CMIF_ROOTHANDLE;
		mi.icolibItem = Quotes_GetIconHandle(IDI_ICON_IMPORT);
		mi.pszService = "Quotes/ImportAll";
		mi.position = 20100004;
		Menu_AddMainMenuItem(&mi);
		CreateServiceFunction(mi.pszService, QuotesMenu_ImportAll);

		bool bSubGroups = 1 == ServiceExists(MS_CLIST_MENUBUILDSUBGROUP);

		HookEvent(ME_CLIST_PREBUILDCONTACTMENU, Quotes_PrebuildContactMenu);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.pszContactOwner = QUOTES_PROTOCOL_NAME;
		hMenuRoot = NULL;
		if (bSubGroups)
		{
			mi.pszPopupName = (char *)-1;
			mi.icolibItem = Quotes_GetIconHandle(IDI_ICON_MAIN);
			mi.flags = CMIF_TCHAR | CMIF_ROOTPOPUP;
			tstring sProtocolName = quotes_a2t(QUOTES_PROTOCOL_NAME);
			mi.ptszName = const_cast<TCHAR*>(sProtocolName.c_str());//A2T(QUOTES_PROTOCOL_NAME);
			mi.position = 0;

			hMenuRoot = Menu_AddContactMenuItem(&mi);
		}

		mi.flags = CMIF_TCHAR;
		if (bSubGroups)
		{
			mi.flags |= CMIF_CHILDPOPUP;
			mi.pszPopupName = (char*)hMenuRoot;
		}

		mi.ptszName = LPGENT("Refresh");
		mi.popupPosition = 0;
		mi.icolibItem = Quotes_GetIconHandle(IDI_ICON_REFRESH);
		mi.pszService = "Quotes/RefreshContact";
		g_hMenuRefresh = Menu_AddContactMenuItem(&mi);
		CreateServiceFunction(mi.pszService, QuotesMenu_RefreshContact);

		mi.ptszName = LPGENT("Open Log File...");
		mi.popupPosition = 1;
		mi.icolibItem = NULL;
		mi.pszService = "Quotes/OpenLogFile";
		g_hMenuOpenLogFile = Menu_AddContactMenuItem(&mi);
		CreateServiceFunction(mi.pszService, QuotesMenu_OpenLogFile);

#ifdef CHART_IMPLEMENT
		mi.ptszName = LPGENT("Chart...");
		mi.popupPosition = 2;
		mi.icolibItem = NULL;
		mi.pszService = "Quotes/Chart";
		g_hMenuChart = Menu_AddContactMenuItem(&mi);
		CreateServiceFunction(mi.pszService, QuotesMenu_Chart);
#endif

		mi.ptszName = LPGENT("Edit Settings...");
#ifdef CHART_IMPLEMENT
		mi.popupPosition = 3;
#else
		mi.popupPosition = 2;
#endif
		mi.icolibItem = NULL;
		mi.pszService = "Quotes/EditSettings";
		g_hMenuEditSettings = Menu_AddContactMenuItem(&mi);
		CreateServiceFunction(mi.pszService, QuotesMenu_EditSettings);
	}

	int Quotes_OnToolbarLoaded(WPARAM, LPARAM)
	{
		TTBButton ttb = { sizeof(ttb) };
		ttb.name = LPGEN("Enable/Disable Quotes Auto Update");
		ttb.pszService = g_pszAutoUpdateCmd;
		ttb.pszTooltipUp = LPGEN("Quotes Auto Update Enabled");
		ttb.pszTooltipDn = LPGEN("Quotes Auto Update Disabled");
		ttb.hIconHandleUp = Quotes_GetIconHandle(IDI_ICON_MAIN);
		ttb.hIconHandleDn = Quotes_GetIconHandle(IDI_ICON_DISABLED);
		ttb.dwFlags = ((g_bAutoUpdate) ? 0 : TTBBF_PUSHED) | TTBBF_ASPUSHBUTTON | TTBBF_VISIBLE;
		g_hTBButton = TopToolbar_AddButton(&ttb);

		ttb.name = LPGEN("Currency Converter");
		ttb.pszService = g_pszCurrencyConverter;
		ttb.pszTooltipUp = LPGEN("Currency Converter");
		ttb.pszTooltipDn = LPGEN("Currency Converter");
		ttb.hIconHandleUp = Quotes_GetIconHandle(IDI_ICON_CURRENCY_CONVERTER);
		ttb.hIconHandleDn = Quotes_GetIconHandle(IDI_ICON_CURRENCY_CONVERTER);
		ttb.dwFlags = TTBBF_VISIBLE;
		TopToolbar_AddButton(&ttb);

		return 0;
	}

	int QuotesEventFunc_OnModulesLoaded(WPARAM, LPARAM)
	{
		CHTTPSession::Init();

		// 		HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY,QuotesEventFunc_onExtraImageApply);

		g_hEventWorkThreadStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		HookEvent(ME_USERINFO_INITIALISE, QuotesEventFunc_OnUserInfoInit);

		HookEvent(ME_CLIST_DOUBLECLICKED, Quotes_OnContactDoubleClick);

		HookEvent(ME_TTB_MODULELOADED, Quotes_OnToolbarLoaded);

		g_bAutoUpdate = 1 == db_get_b(NULL, QUOTES_MODULE_NAME, DB_STR_AUTO_UPDATE, 1);

		InitMenu();

		BOOL b = ::ResetEvent(g_hEventWorkThreadStop);
		assert(b && "Failed to reset event");

		const CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
		const CQuotesProviders::TQuotesProviders& rapProviders = pProviders->GetProviders();
		for (CQuotesProviders::TQuotesProviders::const_iterator i = rapProviders.begin(); i != rapProviders.end(); ++i)
		{
			const CQuotesProviders::TQuotesProviderPtr& pProvider = *i;
			g_ahThreads.push_back(mir_forkthread(WorkingThread, pProvider.get()));
		}

		return 0;
	}

	int QuotesEventFunc_OnContactDeleted(WPARAM wParam, LPARAM)
	{
		MCONTACT hContact = MCONTACT(wParam);

		const CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
		CQuotesProviders::TQuotesProviderPtr pProvider = pProviders->GetContactProviderPtr(hContact);
		if (pProvider)
		{
			pProvider->DeleteContact(hContact);
		}

		return 0;
	}

	INT_PTR QuoteProtoFunc_GetStatus(WPARAM,LPARAM)
	{
		return g_bAutoUpdate ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
	}

	void WaitForWorkingThreads()
	{
		size_t cThreads = g_ahThreads.size();
		if (cThreads > 0)
		{
			HANDLE* paHandles = &*(g_ahThreads.begin());
			::WaitForMultipleObjects((DWORD)cThreads, paHandles, TRUE, INFINITE);
		}
	}


	int QuotesEventFunc_PreShutdown(WPARAM, LPARAM)
	{
		//QuoteProtoFunc_SetStatus(ID_STATUS_OFFLINE,0);
		::SetEvent(g_hEventWorkThreadStop);

		CModuleInfo::GetInstance().OnMirandaShutdown();
		return 0;
	}

	// 	INT_PTR QuoteProtoFunc_GetName(WPARAM wParam, LPARAM lParam)
	// 	{
	// 		if(lParam)
	// 		{
	// 			mir_strncpy(reinterpret_cast<char*>(lParam),QUOTES_PROTOCOL_NAME,wParam);
	// 			return 0;
	// 		}
	// 		else
	// 		{
	// 			return 1;
	// 		}
	// 	}
	// 
	// 	INT_PTR QuoteProtoFunc_GetCaps(WPARAM wp,LPARAM lp)
	// 	{
	// 		int ret = 0;
	// 		switch(wp)
	// 		{
	// 		case PFLAGNUM_1:
	// 			ret = PF1_PEER2PEER;
	// 			break;
	// 		case PFLAGNUM_3:
	// 		case PFLAGNUM_2:
	// 			ret = PF2_ONLINE|PF2_LONGAWAY;
	// 			if(CModuleInfo::GetInstance().GetExtendedStatusFlag())
	// 			{
	// 				ret |= PF2_LIGHTDND;
	// 			}
	// 			break;
	// 		}
	// 
	// 		return ret;
	// 	}
	// 
	// 	INT_PTR QuoteProtoFunc_LoadIcon(WPARAM wp,LPARAM /*lp*/)
	// 	{
	// 		if ((wp & 0xffff) == PLI_PROTOCOL)
	// 		{
	// 			return reinterpret_cast<int>(::CopyIcon(Quotes_LoadIconEx(ICON_STR_MAIN)));
	// 		}
	// 
	// 		return 0;
	// 	}

	int QuotesEventFunc_OptInitialise(WPARAM wp, LPARAM/* lp*/)
	{
		const CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
		const CQuotesProviders::TQuotesProviders& rapProviders = pProviders->GetProviders();

		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.position = 910000000;
		odp.hInstance = g_hInstance;
		odp.ptszTitle = _T(QUOTES_PROTOCOL_NAME);
		odp.ptszGroup = LPGENT("Network");
		odp.hIcon = Quotes_LoadIconEx(ICON_STR_MAIN);
		odp.flags = ODPF_USERINFOTAB | ODPF_TCHAR;

		std::for_each(rapProviders.begin(), rapProviders.end(), boost::bind(&IQuotesProvider::ShowPropertyPage, _1, wp, boost::ref(odp)));
		return 0;
	}

	inline int Quotes_DestroyServiceFunction(HANDLE h)
	{
		return DestroyServiceFunction(h);
	}

	inline int Quotes_UnhookEvent(HANDLE h)
	{
		return UnhookEvent(h);
	}

	inline int Quotes_RemoveMenuItem(HGENMENU h)
	{
		return CallService(MO_REMOVEMENUITEM, reinterpret_cast<WPARAM>(h), 0);
	}

}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	g_hInstance = hinstDLL;
	return TRUE;
}

extern "C"
{
	__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
	{
		return &Global_pluginInfo;
	}

	int __declspec(dllexport) Load(void)
	{
		mir_getLP(&Global_pluginInfo);

		if (false == CModuleInfo::Verify())
		{
			return 1;
		}

		Quotes_IconsInit();
		Quotes_InitExtraIcons();

		PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
		pd.szName = QUOTES_PROTOCOL_NAME;
		pd.type = PROTOTYPE_VIRTUAL;
		CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

		// 		CreateProtoServiceFunction(QUOTES_PROTOCOL_NAME, PS_GETNAME, QuoteProtoFunc_GetName);
		// 		CreateProtoServiceFunction(QUOTES_PROTOCOL_NAME, PS_GETCAPS, QuoteProtoFunc_GetCaps);
		// 		CreateProtoServiceFunction(QUOTES_PROTOCOL_NAME, PS_SETSTATUS, QuoteProtoFunc_SetStatus);
		CreateProtoServiceFunction(QUOTES_PROTOCOL_NAME, PS_GETSTATUS, QuoteProtoFunc_GetStatus);
		// 		CreateProtoServiceFunction(QUOTES_PROTOCOL_NAME, PS_LOADICON, QuoteProtoFunc_LoadIcon);

		HookEvent(ME_SYSTEM_MODULESLOADED, QuotesEventFunc_OnModulesLoaded);
		HookEvent(ME_DB_CONTACT_DELETED, QuotesEventFunc_OnContactDeleted);
		HookEvent(ME_SYSTEM_PRESHUTDOWN, QuotesEventFunc_PreShutdown);
		HookEvent(ME_OPT_INITIALISE, QuotesEventFunc_OptInitialise);

		CreateServiceFunction(MS_QUOTES_EXPORT, Quotes_Export);
		CreateServiceFunction(MS_QUOTES_IMPORT, Quotes_Import);

		return 0;
	}

	__declspec(dllexport) int Unload(void)
	{
		WaitForWorkingThreads();

		::CloseHandle(g_hEventWorkThreadStop);

		return 0;
	}
}
