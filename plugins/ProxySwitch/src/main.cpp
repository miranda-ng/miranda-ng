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
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

HGENMENU hEnableDisablePopupMenu = 0;

NETWORK_INTERFACE_LIST NIF_List;
mir_cs csNIF_List;
ACTIVE_CONNECTION_LIST Connection_List;
mir_cs csConnection_List;
HANDLE hEventRebound = NULL;

wchar_t opt_useProxy[MAX_IPLIST_LENGTH];
wchar_t opt_noProxy[MAX_IPLIST_LENGTH];
wchar_t opt_hideIntf[MAX_IPLIST_LENGTH];
UINT opt_defaultColors;
UINT opt_popups;
UINT opt_showProxyState;
UINT opt_miranda;
UINT opt_ie;
UINT opt_firefox;
UINT opt_showMyIP;
UINT opt_showProxyIP;
UINT opt_alwayReconnect;
UINT opt_startup;
UINT opt_not_restarted;
COLORREF opt_bgColor;
COLORREF opt_txtColor;

UINT opt_popupPluginInstalled;

static HANDLE hEventConnect = NULL;
static HANDLE hEventDisconnect = NULL;
static HANDLE hSvcCopyClip0 = NULL;
static HANDLE hSvcCopyClip1 = NULL;
static HANDLE hSvcCopyClip2 = NULL;
static HANDLE hSvcCopyClip3 = NULL;
static HANDLE hSvcCopyClip4 = NULL;
static HANDLE hSvcCopyClip5 = NULL;
static HANDLE hSvcPopupSwitch = NULL;
static HANDLE hSvcProxyDisable = NULL;
static HANDLE hSvcProxyEnable = NULL;
static HANDLE hSvcShowMyIP = NULL;

/* ################################################################################ */

static INT_PTR ShowMyIPAddrs(WPARAM, LPARAM)
{
	PopupMyIPAddrs(NULL);
	return 0;
}

void PopupMyIPAddrs(wchar_t *msg)
{
	POPUPDATAW ppd;
	NETWORK_INTERFACE_LIST list;

	ZeroMemory(&ppd, sizeof(ppd));

	if (Create_NIF_List_Ex(&list) >= 0) {

		wcsncpy_s(ppd.lpwzText, Print_NIF_List(list, msg), _TRUNCATE);

		if (opt_popupPluginInstalled) {
			LoadSettings();
			ppd.lchIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_PROXY));
			wcsncpy_s(ppd.lpwzContactName, TranslateT("Current IP address"), _TRUNCATE);
			ppd.colorBack = opt_defaultColors ? 0 : opt_bgColor;
			ppd.colorText = opt_defaultColors ? 0 : opt_txtColor;
			CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
		}
		else {
			MessageBox(NULL, ppd.lpwzText, _A2T(MODULENAME), MB_OK | MB_ICONINFORMATION);
		}

		Free_NIF_List(&list);
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

void CopyIP2Clipboard(UCHAR idx)
{
	mir_cslock lck(csNIF_List);
	if (NIF_List.item[idx].IPcount == 0) {
		return;
	}
	if (!OpenClipboard(NULL)) {
		return;
	}
	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, (HANDLE)NIF_List.item[idx].IPstr);
	CloseClipboard();
}

static INT_PTR CopyIP2Clipboard0(WPARAM, LPARAM)
{
	CopyIP2Clipboard(0);
	return 0;
}

static INT_PTR CopyIP2Clipboard1(WPARAM, LPARAM)
{
	CopyIP2Clipboard(1);
	return 0;
}

static INT_PTR CopyIP2Clipboard2(WPARAM, LPARAM)
{
	CopyIP2Clipboard(2);
	return 0;
}

static INT_PTR CopyIP2Clipboard3(WPARAM, LPARAM)
{
	CopyIP2Clipboard(3);
	return 0;
}

static INT_PTR CopyIP2Clipboard4(WPARAM, LPARAM)
{
	CopyIP2Clipboard(4);
	return 0;
}

static INT_PTR CopyIP2Clipboard5(WPARAM, LPARAM)
{
	CopyIP2Clipboard(5);
	return 0;
}

void UpdateInterfacesMenu(void)
{
	UCHAR idx;
	CMenuItem mi(g_plugin);
	char svc[60];

	if (!opt_showProxyIP && !opt_not_restarted)
		return;

	mir_cslock lck(csNIF_List);
	for (idx = 0; idx < NIF_List.count; idx++) {
		if (NIF_List.item[idx].MenuItem) {
			// set new name and flags
			//mi.name.w = Print_NIF(&(NIF_List.item[idx]));
			//if (NIF_List.item[idx].IPcount == 0) mi.flags |= CMIF_GRAYED;
			//mi.flags |= CMIM_FLAGS | CMIM_NAME;
			// update menu item
			Menu_ModifyItem(NIF_List.item[idx].MenuItem, Print_NIF(&(NIF_List.item[idx])), INVALID_HANDLE_VALUE, CMIF_GRAYED);
			//CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)NIF_List.item[idx].MenuItem, (LPARAM)&mi);
		}
		else {
			// add a new menu item
			sprintf(svc, "%s%d", MS_PROXYSWITCH_COPYIP2CLIP, idx);
			mi.position = 0xC00000;
			mi.flags = CMIF_UNICODE;
			mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Proxy Settings && Interfaces"), 0xC0000000);
			Menu_ConfigureItem(mi.root, MCI_OPT_UID, "68AB766F-09F1-4C4C-9AE1-4135617741C9");

			SET_UID(mi, 0x8295e40d, 0xa262, 0x434b, 0xa4, 0xb3, 0x57, 0x6b, 0xe0, 0xfc, 0x8f, 0x68);
			mi.name.w = Print_NIF(&(NIF_List.item[idx]));
			mi.pszService = svc;
			//mi.pszPopupName = Translate("Proxy Settings && Interfaces");
			//mi.popupPosition = 0xC0000000;
			NIF_List.item[idx].MenuItem = Menu_AddMainMenuItem(&mi);
			// menu cannot be grayed when creating, so we have to do it after that
			if (NIF_List.item[idx].IPcount == 0) {
				//ZeroMemory(&mi, sizeof(mi));
				//mi.cbSize = sizeof(mi);
				//mi.flags |= CMIF_GRAYED;
				Menu_ModifyItem(NIF_List.item[idx].MenuItem, Print_NIF(&(NIF_List.item[idx])), INVALID_HANDLE_VALUE, CMIF_GRAYED);
				//CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)NIF_List.item[idx].MenuItem, (LPARAM)&mi);
			}
			// create and register service for this menu item
			switch (idx) {
			case 0:
				hSvcCopyClip0 = CreateServiceFunction(svc, CopyIP2Clipboard0);
				break;

			case 1: 
				hSvcCopyClip1 = CreateServiceFunction(svc, CopyIP2Clipboard1);
				break;

			case 2: 
				hSvcCopyClip2 = CreateServiceFunction(svc, CopyIP2Clipboard2);
				break;

			case 3:
				hSvcCopyClip3 = CreateServiceFunction(svc, CopyIP2Clipboard3);
				break;

			case 4:
				hSvcCopyClip4 = CreateServiceFunction(svc, CopyIP2Clipboard4);
				break;

			case 5:
				hSvcCopyClip5 = CreateServiceFunction(svc, CopyIP2Clipboard5);
				break;
			}
		}
	}
}

/* ################################################################################ */

void UpdatePopupMenu(BOOL State)
{
	CMenuItem mi(g_plugin);

	if (!hEnableDisablePopupMenu)
		return;

	//ZeroMemory(&mi, sizeof(mi));
	//mi.cbSize = sizeof(mi);

	// popup is now disabled
	if (State == FALSE) {
		mi.name.w = LPGENW("Enable &IP change notification");
		mi.hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_NOTIF_0));

		// popup is now enabled
	}
	else {
		mi.name.w = LPGENW("Disable &IP change notification");
		mi.hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_NOTIF_1));
	}
	//mi.flags = CMIM_ICON | CMIM_NAME;

	// update menu item
	Menu_ModifyItem(hEnableDisablePopupMenu, mi.name.w);
	//CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hEnableDisablePopupMenu, (LPARAM)&mi);
}

static INT_PTR PopupSwitch(WPARAM, LPARAM)
{
	opt_popups = !opt_popups;
	UpdatePopupMenu(opt_popups);
	SaveSettings();
	return 0;
}

/* ################################################################################ */

int CMPlugin::Load()
{
	char proxy = -1;
	IP_RANGE_LIST range;

	opt_startup = FALSE;
	opt_not_restarted = FALSE;

	LoadSettings();

	ZeroMemory(&Connection_List, sizeof(Connection_List));
	Create_NIF_List_Ex(&NIF_List);

	if (opt_ie || opt_miranda || opt_firefox) {
		Create_Range_List(&range, opt_useProxy, TRUE);
		if (Match_Range_List(range, NIF_List))
			proxy = 1;
		Free_Range_List(&range);
		if (proxy == -1) {
			Create_Range_List(&range, opt_noProxy, FALSE);
			if (Match_Range_List(range, NIF_List))
				proxy = 0;
			Free_Range_List(&range);
		}
		if (proxy == -1) {
			Create_Range_List(&range, opt_useProxy, FALSE);
			if (Match_Range_List(range, NIF_List)) 
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
	CMenuItem mi(g_plugin);

	opt_popupPluginInstalled = ServiceExists(MS_POPUP_ADDPOPUP);


	hEventRebound = CreateEvent(NULL, TRUE, FALSE, NULL);
	mir_forkthread(IP_WatchDog, 0);

	if (opt_showMyIP) {
		hSvcShowMyIP = CreateServiceFunction(MS_PROXYSWITCH_SHOWMYIPADDRS, ShowMyIPAddrs);
		//ZeroMemory(&mi, sizeof(mi));
		//mi.cbSize = sizeof(mi);
		SET_UID(mi, 0x53b0835b, 0x7162, 0x4272, 0x83, 0x3b, 0x3f, 0x60, 0x9e, 0xe, 0x76, 0x4a);
		mi.position = 0xC0000000;
		mi.flags = CMIF_UNICODE;
		mi.hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_LOGO));
		mi.name.w = LPGENW("Show my &IP Addresses");
		mi.pszService = MS_PROXYSWITCH_SHOWMYIPADDRS;
		Menu_AddMainMenuItem(&mi);
	}

	if (opt_showProxyIP) {

		hSvcProxyDisable = CreateServiceFunction(MS_PROXYSWITCH_PROXYDISABLE, ProxyDisable);
		//ZeroMemory(&mi, sizeof(mi));
		//mi.cbSize = sizeof(mi);
		SET_UID(mi, 0xf93289a9, 0x3bad, 0x424b, 0xb2, 0x72, 0x14, 0xa7, 0x45, 0xa5, 0x8, 0x9c);
		mi.position = 1;
		mi.name.w = LPGENW("Disable Proxy");
		mi.pszService = MS_PROXYSWITCH_PROXYDISABLE;
		mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Proxy Settings && Interfaces"), 0xC0000000);
		Menu_ConfigureItem(mi.root, MCI_OPT_UID, "A9684E9E-E621-4962-986F-576897928D27");
		//mi.pszPopupName = Translate("Proxy Settings && Interfaces");
		//mi.popupPosition = 0xC0000000;
		Menu_AddMainMenuItem(&mi);

		hSvcProxyEnable = CreateServiceFunction(MS_PROXYSWITCH_PROXYENABLE, ProxyEnable);
		//ZeroMemory(&mi, sizeof(mi));
		//mi.cbSize = sizeof(mi);
		mi.position = 1;
		mi.name.w = LPGENW("Enable Proxy");
		mi.pszService = MS_PROXYSWITCH_PROXYENABLE;
		mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Proxy Settings && Interfaces"), 0xC0000000);
		Menu_ConfigureItem(mi.root, MCI_OPT_UID, "B37E5BBE-19CF-4C78-AE53-A0DB11656C36");
		//mi.pszPopupName = Translate("Proxy Settings && Interfaces");
		//mi.popupPosition = 0xC0000000;
		Menu_AddMainMenuItem(&mi);

		UpdateInterfacesMenu();
	}

	if (opt_popupPluginInstalled) {
		hSvcPopupSwitch = CreateServiceFunction(MS_PROXYSWITCH_POPUPSWITCH, PopupSwitch);
		//ZeroMemory(&mi, sizeof(mi));
		//mi.cbSize = sizeof(mi);
		mi.name.w = LPGENW("IP Change Notification");
		mi.hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_LOGO));
		mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("PopUps"), 0xC0000000);
		Menu_ConfigureItem(mi.root, MCI_OPT_UID, "185AC334-E90E-46C6-83A2-D4E36CB257D9");
		//mi.pszPopupName = Translate("PopUps");
		mi.pszService = MS_PROXYSWITCH_POPUPSWITCH;
		hEnableDisablePopupMenu = Menu_AddMainMenuItem(&mi);

		UpdatePopupMenu(opt_popups);
	}

	return 0;
}

int CMPlugin::Unload()
{
	if (hEventRebound)
		CloseHandle(hEventRebound);
	mir_cslock lck(csNIF_List);
	Free_NIF_List(&NIF_List);
	return 0;
}
