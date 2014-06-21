#include "common.h"

int CSteamProto::OnModulesLoaded(WPARAM, LPARAM)
{
	HookEventObj(ME_OPT_INITIALISE, OnOptionsInit, this);

	TCHAR name[128];
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s connection"), m_tszUserName);

	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSteamProto::PrebuildContactMenu);

	return 0;
}

int CSteamProto::OnPreShutdown(WPARAM, LPARAM)
{
	//SetStatus(ID_STATUS_OFFLINE);

	Netlib_CloseHandle(this->m_hNetlibUser);
	this->m_hNetlibUser = NULL;

	return 0;
}

INT_PTR CSteamProto::OnAccountManagerInit(WPARAM wParam, LPARAM lParam)
{
	return (int)CreateDialogParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_ACCMGR),
		(HWND)lParam,
		CSteamProto::MainOptionsProc,
		(LPARAM)this);
}

int CSteamProto::OnOptionsInit(void *obj, WPARAM wParam, LPARAM lParam)
{
	CSteamProto *instance = (CSteamProto*)obj;

	char *title = mir_t2a(instance->m_tszUserName);

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInstance;
	odp.pszTitle = title;
	odp.dwInitParam = LPARAM(obj);
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszGroup = LPGEN("Network");

	odp.pszTab = LPGEN("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc = MainOptionsProc;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Blocked contacts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_BLOCK_LIST);
	odp.pfnDlgProc = BlockListOptionsProc;
	Options_AddPage(wParam, &odp);

	mir_free(title);

	return 0;
}