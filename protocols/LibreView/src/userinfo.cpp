#include "stdafx.h"


static CMStringW GetDbText(MCONTACT hContact, const char *pszSetting, const wchar_t *pwszDefault = L"")
{
	CLibreViewProto *ppro = g_plugin.getInstance(hContact);
	if (!ppro)
		return CMStringW(pwszDefault);
	CMStringW value = ppro->getMStringW(hContact, pszSetting);
	return value.IsEmpty() ? CMStringW(pwszDefault) : value;
}

static const wchar_t* GetLocalizedUnitByKey(const wchar_t *pwszUnit)
{
	if (!mir_wstrcmpi(pwszUnit, L"mg/dL"))
		return TranslateT("mg/dL");
	if (!mir_wstrcmpi(pwszUnit, L"mmol/L"))
		return TranslateT("mmol/L");
	return pwszUnit;
}

static CMStringW GetSensorRemaining(MCONTACT hContact)
{
	CLibreViewProto *ppro = g_plugin.getInstance(hContact);
	uint32_t activation = ppro ? ppro->getDword(hContact, "SensorActivationTime", 0) : 0;
	if (activation == 0)
		return TranslateT("N/A");

	const time_t expires = activation + SENSOR_LIFETIME;
	time_t now = time(0);
	if (expires <= now)
		return TranslateT("Expired");

	uint32_t warmupMinutes = ppro ? ppro->getDword(hContact, "SensorWarmupMinutes", 0) : 0;
	if (IsSensorWarmingUp(activation, warmupMinutes)) {
		time_t warmupEnd = activation + warmupMinutes * 60;
		int minutesLeft = int((warmupEnd - now) / 60) + 1;
		if (minutesLeft < 1)
			minutesLeft = 1;
		CMStringW result;
		result.Format(TranslateT("Warming up (%d min left)"), minutesLeft);
		return result;
	}

	time_t remaining = expires - now;
	int days = int(remaining / (24 * 60 * 60));
	int hours = int((remaining % (24 * 60 * 60)) / (60 * 60));
	int minutes = int((remaining % (60 * 60)) / 60);

	CMStringW result;
	if (days > 0)
		result.Format(TranslateT("%d days %d hours"), days, hours);
	else if (hours > 0)
		result.Format(TranslateT("%d hours %d minutes"), hours, minutes);
	else
		result.Format(TranslateT("%d minutes"), minutes);
	return result;
}

class CLibreViewUserInfoDlg : public CUserInfoPageDlg
{
public:
	CLibreViewUserInfoDlg() :
		CUserInfoPageDlg(g_plugin, IDD_USERINFO)
	{}

	bool OnInitDialog() override
	{
		OnRefresh();
		return true;
	}

	bool OnRefresh() override
	{
		// Build patient name from FirstName + LastName
		CMStringW firstName = GetDbText(m_hContact, "FirstName");
		CMStringW lastName = GetDbText(m_hContact, "LastName");
		CMStringW patient(firstName);
		if (!lastName.IsEmpty()) {
			if (!patient.IsEmpty())
				patient.AppendChar(' ');
			patient.Append(lastName);
		}
		if (patient.IsEmpty())
			patient = TranslateT("LibreView");

		SetDlgItemText(m_hwnd, IDC_PATIENT, patient);

		CMStringW originalValue = GetDbText(m_hContact, "Value");
		CLibreViewProto *ppro = g_plugin.getInstance(m_hContact);
		const bool bUseMgdl = ppro && ppro->DisplayUnits == 1;

		// Get API units from database
		const int apiUnits = ppro ? ppro->getDword(m_hContact, "GlucoseUnits", 0) : 0;
		const bool bApiMgdl = apiUnits == 1;
		const int offset = ppro ? ppro->Offset : 0;

		// Convert original API value to display units
		CMStringW displayValue = ConvertGlucoseForDisplay(originalValue, bApiMgdl, bUseMgdl, offset);

		const wchar_t *pwszUnit = GetLocalizedUnitByKey(bUseMgdl ? L"mg/dL" : L"mmol/L");
		CMStringW lastValue;
		if (!displayValue.IsEmpty()) {
			lastValue.Format(L"%s %s", displayValue.c_str(), pwszUnit);
			int trendValue = ppro ? ppro->getDword(m_hContact, "TrendArrow", 0) : 0;
			CMStringW trendArrow(TrendToArrow(trendValue));
			if (!trendArrow.IsEmpty())
				lastValue.AppendFormat(L" (%s)", trendArrow.c_str());
		}

		SetDlgItemText(m_hwnd, IDC_LAST_GLUCOSE, lastValue);
		uint32_t activationTime = ppro ? ppro->getDword(m_hContact, "SensorActivationTime", 0) : 0;
		if (activationTime) {
			wchar_t buf[64];
			TimeZone_PrintTimeStamp(nullptr, activationTime, L"d t", buf, _countof(buf), 0);
			SetDlgItemText(m_hwnd, IDC_SENSOR_ACTIVATION, buf);
		}
		else {
			SetDlgItemText(m_hwnd, IDC_SENSOR_ACTIVATION, L"");
		}
		SetDlgItemText(m_hwnd, IDC_SENSOR_REMAINING, GetSensorRemaining(m_hContact));
		// Parse and format last update timestamp
		CMStringW rawTimestamp = GetDbText(m_hContact, "Timestamp");
		if (!rawTimestamp.IsEmpty()) {
			uint32_t timestampUnix = ParseLibreTimestamp(rawTimestamp);
			if (timestampUnix) {
				wchar_t timestampBuf[64];
				TimeZone_PrintTimeStamp(nullptr, timestampUnix, L"d t", timestampBuf, _countof(timestampBuf), 0);
				SetDlgItemText(m_hwnd, IDC_LAST_UPDATE, timestampBuf);
			}
			else {
				SetDlgItemText(m_hwnd, IDC_LAST_UPDATE, rawTimestamp);
			}
		}
		else {
			SetDlgItemText(m_hwnd, IDC_LAST_UPDATE, L"");
		}
		return false;
	}
};

int UserInfoInit(WPARAM wParam, LPARAM hContact)
{
	if (g_plugin.getInstance((MCONTACT)hContact) == nullptr)
		return 0;

	USERINFOPAGE uip = {};
	uip.szTitle.a = LPGEN("LibreView");
	uip.position = 100000000;
	uip.flags = ODPF_BOLDGROUPS;
	uip.pDialog = new CLibreViewUserInfoDlg();
	g_plugin.addUserInfo(wParam, &uip);
	return 0;
}
