#include "common.h"

int CSteamProto::OnModulesLoaded(WPARAM, LPARAM)
{
	HookEventObj(ME_OPT_INITIALISE, OnOptionsInit, this);
	HookProtoEvent(ME_IDLE_CHANGED, &CSteamProto::OnIdleChanged);

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
	return (INT_PTR)(CSteamOptionsMain::CreateAccountManagerPage(this, (HWND)lParam))->GetHwnd();
}

int CSteamProto::OnOptionsInit(void *obj, WPARAM wParam, LPARAM lParam)
{
	CSteamProto *instance = (CSteamProto*)obj;

	char *title = mir_t2a(instance->m_tszUserName);

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInstance;
	odp.pszTitle = title;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszGroup = LPGEN("Network");

	odp.pszTab = LPGEN("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc = CSteamOptionsMain::DynamicDlgProc;
	odp.dwInitParam = (LPARAM)&instance->SteamMainOptionsParam;
	instance->SteamMainOptionsParam.create = CSteamOptionsMain::CreateOptionsPage;
	instance->SteamMainOptionsParam.param = instance;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Blocked contacts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_BLOCK_LIST);
	odp.pfnDlgProc = CSteamOptionsMain::DynamicDlgProc;
	odp.dwInitParam = (LPARAM)&instance->SteamBlockListOptionsParam;
	instance->SteamBlockListOptionsParam.create = CSteamOptionsBlockList::CreateOptionsPage;
	instance->SteamBlockListOptionsParam.param = instance;
	Options_AddPage(wParam, &odp);

	mir_free(title);

	return 0;
}

int CSteamProto::OnIdleChanged(WPARAM wParam, LPARAM lParam)
{
	bool idle = (lParam & IDF_ISIDLE) != 0;
	bool privacy = (lParam & IDF_PRIVACY) != 0;

	// Respect user choice about (not) notifying idle to protocols
	if (privacy)
	{
		// Reset it to 0 if there is some time already
		if (m_idleTS)
		{
			m_idleTS = 0;
			delSetting("IdleTS");
		}

		return 0;
	}

	// We don't want to reset idle time when we're already in idle state
	if (idle && m_idleTS > 0)
		return 0;

	if (idle)
	{
		// User started being idle
		MIRANDA_IDLE_INFO mii = { sizeof(mii) };
		CallService(MS_IDLE_GETIDLEINFO, 0, (LPARAM)&mii);
		
		// Compute time when user really became idle
		m_idleTS = time(0) - mii.idleTime * 60;
		setDword("IdleTS", m_idleTS);
	}
	else
	{
		// User stopped being idle
		m_idleTS = 0;
		delSetting("IdleTS");
	}

	return 0;
}