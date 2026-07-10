#pragma once

#include <windows.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_fontservice.h>
#include <m_history.h>
#include <m_ignore.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protoint.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_timezones.h>
#include <m_userinfo.h>
#include <m_utils.h>

#include "resource.h"

#define MODULENAME "LibreView"
#define DEFAULT_API_URL "https://api.libreview.ru"
#define DEFAULT_API_VERSION "4.16.0"
#define SENSOR_LIFETIME (14 * 24 * 60 * 60)

static inline bool IsSensorExpired(uint32_t activation)
{
	return activation > 0 && time(0) >= (activation + SENSOR_LIFETIME);
}

class CLibreViewProto;

extern HNETLIBUSER hNetlibUser;
extern UINT_PTR hTimer;

void Check_ThreadFunc(void *);
void UpdateContactDisplay(MCONTACT);
void RestartTimer();
int UserInfoInit(WPARAM, LPARAM);
uint32_t ParseLibreTimestamp(const CMStringW &timestamp);
void RefreshGraphWindow();
static inline CMStringW ConvertGlucoseForDisplay(const CMStringW &originalValue, bool bApiMgdl, bool bUseMgdl, int offset)

{
	if (originalValue.IsEmpty())
		return CMStringW();
	
	double value = _wtof(originalValue.c_str());
	
	// Apply offset BEFORE unit conversion (in the same units as API data)
	value += (double)offset;
	
	if (bApiMgdl && bUseMgdl) {
		// API mg/dL -> Display mg/dL (no conversion, offset applied above)
		CMStringW result(FORMAT, L"%.0f", value);
		return result;
	}
	else if (bApiMgdl && !bUseMgdl) {
		// API mg/dL -> Display mmol/L (convert)
		CMStringW result(FORMAT, L"%.1f", value / 18.0);
		if (result.Right(2) == L".0")
			result.Truncate(result.GetLength() - 2);
		return result;
	}
	else if (!bApiMgdl && bUseMgdl) {
		// API mmol/L -> Display mg/dL (convert)
		CMStringW result(FORMAT, L"%.0f", value * 18.0);
		return result;
	}
	else {
		// API mmol/L -> Display mmol/L (no conversion, offset applied above)
		CMStringW result(FORMAT, L"%.1f", value);
		if (result.Right(2) == L".0")
			result.Truncate(result.GetLength() - 2);
		return result;
	}
}

class CLibreViewProto : public PROTO<CLibreViewProto>
{
	mir_cs csLock;
	bool bChecking = false;

public:
	CLibreViewProto(const char*, const wchar_t*);
	~CLibreViewProto();

	CMOption<uint32_t> UpdateInterval;
	CMOption<uint32_t> DisplayUnits;
	CMOption<bool> WriteHistory;
	CMOption<int> Offset;

	MCONTACT m_hContact;
	CMStringA szToken, szAccountHash, szPatientId, szApiUrl, szMinVersion;
	time_t tsLastUpdate = 0;

	bool Login();
	bool FetchConnections();
	bool FetchGlucose();
	void ClearAuth();

	void CheckAccount();
	void EnsureAccount();
	MCONTACT EnsureAccountContact();

	INT_PTR __cdecl Update(WPARAM, LPARAM);
	int __cdecl OptInit(WPARAM, LPARAM);

	INT_PTR GetCaps(int type, MCONTACT hContact = 0) override;
	int SetStatus(int iNewStatus) override;
	void OnModulesLoaded() override;
	void OnShutdown() override;
};

typedef CProtoDlgBase<CLibreViewProto> CLibreViewDlgBase;

struct CMPlugin : public ACCPROTOPLUGIN<CLibreViewProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

extern CMPlugin g_plugin;
