/*
proxySwitch

The plugin watches IP address changes, reports them via popups and adjusts
the proxy settings of Miranda and Internet Explorer accordingly.
*/

#include "stdafx.h"

CMPlugin g_plugin;

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
	// {4DF0C267-6EFB-4410-B651-385F87158509}
	{ 0x4df0c267, 0x6efb, 0x4410,{ 0xb6, 0x51, 0x38, 0x5f, 0x87, 0x15, 0x85, 0x9 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bPopups(MODULENAME, "PopupEnabled", true)
{
}

HGENMENU hMenuRoot;

OBJLIST<ACTIVE_CONNECTION> g_arConnections(10, PtrKeySortT);
mir_cs csConnection_List;

OBJLIST<NETWORK_INTERFACE> g_arNIF(10);
mir_cs csNIF_List;

HANDLE hEventRebound = NULL;

wchar_t opt_useProxy[MAX_IPLIST_LENGTH];
wchar_t opt_noProxy[MAX_IPLIST_LENGTH];
wchar_t opt_hideIntf[MAX_IPLIST_LENGTH];
UINT opt_defaultColors;
UINT opt_showProxyState;
UINT opt_miranda;
UINT opt_ie;
UINT opt_firefox;
UINT opt_alwayReconnect;
UINT opt_startup;
UINT opt_not_restarted;
COLORREF opt_bgColor;
COLORREF opt_txtColor;

static HANDLE hEventConnect = NULL;
static HANDLE hEventDisconnect = NULL;
static HANDLE hSvcProxyDisable = NULL;
static HANDLE hSvcProxyEnable = NULL;
static HANDLE hSvcShowMyIP = NULL;

/* ################################################################################ */

static INT_PTR ShowMyIPAddrs(WPARAM, LPARAM)
{
	PopupMyIPAddrs(NULL);
	return 0;
}

void PopupMyIPAddrs(const wchar_t *msg)
{
	OBJLIST<NETWORK_INTERFACE> list(10);
	if (Create_NIF_List_Ex(&list) >= 0) {
		POPUPDATAW ppd;
		wcsncpy_s(ppd.lpwzText, Print_NIF_List(list, msg), _TRUNCATE);

		LoadSettings();
		ppd.lchIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_PROXY));
		wcsncpy_s(ppd.lpwzContactName, TranslateT("Current IP address"), _TRUNCATE);
		ppd.colorBack = opt_defaultColors ? 0 : opt_bgColor;
		ppd.colorText = opt_defaultColors ? 0 : opt_txtColor;
		PUAddPopupW(&ppd);
	}
}

static INT_PTR ProxyEnable(WPARAM, LPARAM)
{
	Set_IE_Proxy_Status(1);
	Set_Miranda_Proxy_Status(1);
	Set_Firefox_Proxy_Status(1);
	return 0;
}

static INT_PTR ProxyDisable(WPARAM, LPARAM)
{
	Set_IE_Proxy_Status(0);
	Set_Miranda_Proxy_Status(0);
	Set_Firefox_Proxy_Status(0);
	return 0;
}

/* ################################################################################ */

static INT_PTR CopyIP2Clipboard(WPARAM, LPARAM, LPARAM idx)
{
	mir_cslock lck(csNIF_List);
	if (g_arNIF[idx].IPcount != 0)
		Utils_ClipboardCopy(g_arNIF[idx].IPstr);
	return 0;
}

void UpdateInterfacesMenu(void)
{
	CMenuItem mi(&g_plugin);

	mir_cslock lck(csNIF_List);
	for (auto &it : g_arNIF) {
		if (it->MenuItem) {
			// update menu item
			Menu_ModifyItem(it->MenuItem, Print_NIF(it), INVALID_HANDLE_VALUE, CMIF_GRAYED);
		}
		else {
			int idx = g_arNIF.indexOf(&it);
			
			// add a new menu item
			char svc[60];
			sprintf(svc, "%s%d", MS_PROXYSWITCH_COPYIP2CLIP, idx);
			mi.position = 0xC00000;
			mi.flags = CMIF_UNICODE;
			mi.root = hMenuRoot;

			SET_UID(mi, 0x8295e40d, 0xa262, 0x434b, 0xa4, 0xb3, 0x57, 0x6b, 0xe0, 0xfc, 0x8f, 0x68);
			mi.name.w = Print_NIF(it);
			mi.pszService = svc;
			it->MenuItem = Menu_AddMainMenuItem(&mi);
			// menu cannot be grayed when creating, so we have to do it after that
			if (it->IPcount == 0)
				Menu_ModifyItem(it->MenuItem, Print_NIF(it), INVALID_HANDLE_VALUE, CMIF_GRAYED);

			// create and register service for this menu item
			if (idx >= 0 && idx < 6)
				CreateServiceFunctionParam(svc, CopyIP2Clipboard, idx);
		}
	}
}

/* ################################################################################ */

int CMPlugin::Load()
{
	char proxy = -1;
	IP_RANGE_LIST range;

	opt_startup = FALSE;
	opt_not_restarted = FALSE;

	LoadSettings();

	addPopupOption(LPGEN("IP change notification"), bPopups);

	Create_NIF_List_Ex(&g_arNIF);

	if (opt_ie || opt_miranda || opt_firefox) {
		Create_Range_List(&range, opt_useProxy, TRUE);
		if (Match_Range_List(range, g_arNIF))
			proxy = 1;
		Free_Range_List(&range);
		if (proxy == -1) {
			Create_Range_List(&range, opt_noProxy, FALSE);
			if (Match_Range_List(range, g_arNIF))
				proxy = 0;
			Free_Range_List(&range);
		}
		if (proxy == -1) {
			Create_Range_List(&range, opt_useProxy, FALSE);
			if (Match_Range_List(range, g_arNIF))
				proxy = 1;
			Free_Range_List(&range);
		}
		if (proxy != -1) {
			if (opt_miranda && Get_Miranda_Proxy_Status() != proxy)
				Set_Miranda_Proxy_Status(proxy);
			if (opt_ie && Get_IE_Proxy_Status() != proxy)
				Set_IE_Proxy_Status(proxy);
			if (opt_firefox && Get_Firefox_Proxy_Status() != proxy)
				Set_Firefox_Proxy_Status(proxy);
		}
	}

	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, Init);
	HookEvent(ME_NETLIB_EVENT_CONNECTED, ManageConnections);
	HookEvent(ME_NETLIB_EVENT_DISCONNECTED, ManageConnections);

	return 0;
}

int Init(WPARAM, LPARAM)
{
	hEventRebound = CreateEvent(NULL, TRUE, FALSE, NULL);
	mir_forkthread(IP_WatchDog, 0);

	// menu item
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x53b0835b, 0x7162, 0x4272, 0x83, 0x3b, 0x3f, 0x60, 0x9e, 0xe, 0x76, 0x4a);
	mi.position = 0xC0000000;
	mi.flags = CMIF_UNICODE;
	mi.hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_LOGO));
	mi.name.w = LPGENW("Show my &IP addresses");
	mi.pszService = MS_PROXYSWITCH_SHOWMYIPADDRS;
	HGENMENU hMenu = Menu_AddMainMenuItem(&mi);
	hSvcShowMyIP = CreateServiceFunction(mi.pszService, ShowMyIPAddrs);

	int opt_showMyIP = g_plugin.getByte("ShowMyIP", 100);
	if (opt_showMyIP != 100) {
		Menu_SetVisible(hMenu, opt_showMyIP != 0);
		g_plugin.delSetting("ShowMyIP");
	}

	hMenuRoot = mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Proxy settings && interfaces"), 0xC0000000);
	Menu_ConfigureItem(hMenuRoot, MCI_OPT_UID, "A9684E9E-E621-4962-986F-576897928D27");

	SET_UID(mi, 0xf93289a9, 0x3bad, 0x424b, 0xb2, 0x72, 0x14, 0xa7, 0x45, 0xa5, 0x8, 0x9c);
	mi.position = 1;
	mi.name.w = LPGENW("Disable proxy");
	mi.pszService = MS_PROXYSWITCH_PROXYDISABLE;
	Menu_AddMainMenuItem(&mi);
	hSvcProxyDisable = CreateServiceFunction(mi.pszService, ProxyDisable);

	SET_UID(mi, 0xf93289a9, 0x3bad, 0x424b, 0xb2, 0x72, 0x14, 0xa7, 0x45, 0xa5, 0x8, 0x9D);
	mi.position = 2;
	mi.name.w = LPGENW("Enable proxy");
	mi.pszService = MS_PROXYSWITCH_PROXYENABLE;
	Menu_AddMainMenuItem(&mi);
	hSvcProxyEnable = CreateServiceFunction(mi.pszService, ProxyEnable);

	int opt_showProxyIP = g_plugin.getByte("ShowProxyIP", 100);
	if (opt_showProxyIP != 100) {
		Menu_SetVisible(mi.root, opt_showProxyIP != 0);
		g_plugin.delSetting("ShowProxyIP");
	}

	UpdateInterfacesMenu();
	return 0;
}

int CMPlugin::Unload()
{
	if (hEventRebound)
		CloseHandle(hEventRebound);

	mir_cslock lck(csNIF_List);
	g_arNIF.destroy();
	return 0;
}
