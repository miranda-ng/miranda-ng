#include "stdafx.h"

void CSteamProto::OnModulesLoaded()
{
	HookProtoEvent(ME_IDLE_CHANGED, &CSteamProto::OnIdleChanged);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CSteamProto::OnPreCreateMessage);

	// Register custom db event
	DBEVENTTYPEDESCR dbEventType = {};
	dbEventType.module = m_szModuleName;
	dbEventType.eventType = EVENTTYPE_STEAM_CHATSTATES;
	dbEventType.descr = "Chat state notifications";
	DbEvent_RegisterType(&dbEventType);
}

int CSteamProto::OnIdleChanged(WPARAM, LPARAM lParam)
{
	bool idle = (lParam & IDF_ISIDLE) != 0;
	bool privacy = (lParam & IDF_PRIVACY) != 0;

	// Respect user choice about (not) notifying idle to protocols
	if (privacy) {
		// Reset it to 0 if there is some time already
		if (m_idleTS) {
			m_idleTS = 0;
			delSetting("IdleTS");
		}

		return 0;
	}

	// We don't want to reset idle time when we're already in idle state
	if (idle && m_idleTS > 0)
		return 0;

	if (idle) {
		// User started being idle
		MIRANDA_IDLE_INFO mii;
		Idle_GetInfo(mii);

		// Compute time when user really became idle
		m_idleTS = now() - mii.idleTime * 60;
		setDword("IdleTS", m_idleTS);
	}
	else {
		// User stopped being idle
		m_idleTS = 0;
		delSetting("IdleTS");
	}

	return 0;
}
