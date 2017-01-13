#include "stdafx.h"

int CSteamProto::OnModulesLoaded(WPARAM, LPARAM)
{
	HookProtoEvent(ME_OPT_INITIALISE, &CSteamProto::OnOptionsInit);
	HookProtoEvent(ME_IDLE_CHANGED, &CSteamProto::OnIdleChanged);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CSteamProto::OnPreCreateMessage);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSteamProto::PrebuildContactMenu);

	// Register custom db event
	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = m_szModuleName;
	dbEventType.eventType = EVENTTYPE_STEAM_CHATSTATES;
	dbEventType.descr = "Chat state notifications";
	DbEvent_RegisterType(&dbEventType);
	return 0;
}

INT_PTR CSteamProto::OnAccountManagerInit(WPARAM, LPARAM lParam)
{
	return (INT_PTR)(CSteamOptionsMain::CreateAccountManagerPage(this, (HWND)lParam))->GetHwnd();
}

int CSteamProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_hInstance;
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");

	odp.szTab.w = LPGENW("Account");
	odp.pDialog = CSteamOptionsMain::CreateOptionsPage(this);
	Options_AddPage(wParam, &odp);

	odp.szTab.w = LPGENW("Blocked contacts");
	odp.pDialog = CSteamOptionsBlockList::CreateOptionsPage(this);
	Options_AddPage(wParam, &odp);
	return 0;
}

int CSteamProto::OnIdleChanged(WPARAM, LPARAM lParam)
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
