#pragma once

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_awaymsg.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_utils.h>

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

class COptions
{
	enum
	{
		cssOnline = 0x00000001,
		cssOffline = 0x00000002,
		cssInvisible = 0x00000004,
		cssShortAway = 0x00000008,
		cssLongAway = 0x00000010,
		cssLightDND = 0x00000020,
		cssHeavyDND = 0x00000040,
		cssFreeChat = 0x00000080,
		cssIdle = 0x00000400,
		cssAll = 0x80000000
	};

	static unsigned long Status2Flag(int status)
	{
		switch (status) {
		case ID_STATUS_ONLINE: return cssOnline;
		case ID_STATUS_OFFLINE: return cssOffline;
		case ID_STATUS_INVISIBLE: return cssInvisible;
		case ID_STATUS_AWAY: return cssShortAway;
		case ID_STATUS_NA: return cssLongAway;
		case ID_STATUS_OCCUPIED: return cssLightDND;
		case ID_STATUS_DND: return cssHeavyDND;
		case ID_STATUS_FREECHAT: return cssFreeChat;
		case ID_STATUS_IDLE: return cssIdle;
		}
		return 0;
	}

	struct PrevStatus
	{
		PrevStatus(const char *_proto, int _status) :
			szProto(mir_strdup(_proto)),
			iStatus(_status)
		{
		}

		ptrA szProto;
		int iStatus;
	};

	OBJLIST<PrevStatus> m_aProtocol2Status;
	static int CompareStatuses(const PrevStatus *p1, const PrevStatus *p2)
	{
		return mir_strcmp(p1->szProto, p2->szProto);
	}

public:
	COptions() :
		m_aProtocol2Status(3, CompareStatuses)
	{
	}

	bool IsProtocolExcluded(const char* pszProtocol)const
	{
		uint32_t dwSettings = db_get_dw(NULL, pszProtocol, "ChangeSkypeStatus_Exclusions", 0);
		return ((dwSettings&cssAll) ? true : false);
	}

	bool IsProtocolStatusExcluded(const char* pszProtocol, int nStatus)const
	{
		uint32_t dwSettings = db_get_dw(NULL, pszProtocol, "ChangeSkypeStatus_Exclusions", 0);
		return ((dwSettings&Status2Flag(nStatus)) ? true : false);
	}

	void ExcludeProtocol(const char* pszProtocol, bool bExclude)
	{
		uint32_t dwSettings = db_get_dw(NULL, pszProtocol, "ChangeSkypeStatus_Exclusions", 0);
		if (bExclude)
			dwSettings |= cssAll;
		else
			dwSettings &= ~cssAll;

		db_set_dw(NULL, pszProtocol, "ChangeSkypeStatus_Exclusions", dwSettings);
	}

	void ExcludeProtocolStatus(const char* pszProtocol, int nStatus, bool bExclude)
	{
		uint32_t dwSettings = db_get_dw(NULL, pszProtocol, "ChangeSkypeStatus_Exclusions", 0);
		if (bExclude)
			dwSettings |= Status2Flag(nStatus);
		else
			dwSettings &= ~Status2Flag(nStatus);

		db_set_dw(NULL, pszProtocol, "ChangeSkypeStatus_Exclusions", dwSettings);
	}

	bool GetSyncStatusMsgFlag() const
	{
		return g_plugin.getBool("SyncStatusMsg");
	}

	bool GetSyncStatusStateFlag() const
	{
		return g_plugin.getBool("SyncStatusState");
	}

	void SetSyncStatusMsgFlag(bool b)
	{
		g_plugin.setByte("SyncStatusMsg", b);
	}

	void SetSyncStatusStateFlag(bool b)
	{
		g_plugin.setByte("SyncStatusState", b);
	}

	bool GetPreviousStatus(const char* pszProtocol, int& nStatus)const
	{
		int i = m_aProtocol2Status.getIndex((PrevStatus*)&pszProtocol);
		if (i != -1) {
			nStatus = m_aProtocol2Status[i].iStatus;
			return true;
		}

		return false;
	}

	void SetPreviousStatus(const char* pszProtocol, int nStatus)
	{
		int i = m_aProtocol2Status.getIndex((PrevStatus*)&pszProtocol);
		if (i != -1)
			m_aProtocol2Status[i].iStatus = nStatus;
		else
			m_aProtocol2Status.insert(new PrevStatus(pszProtocol, nStatus));
	}
};

extern COptions g_Options;

/////////////////////////////////////////////////////////////////////////////////////////

struct CMirandaStatus2SkypeStatus
{
	int m_nMirandaStatus;
	LPCSTR m_pszSkypeStatus;
	LPCTSTR m_ptszStatusName;
};

extern const CMirandaStatus2SkypeStatus g_aStatusCode[MAX_STATUS_COUNT];
