#include "stdafx.h"

static CMStringW ConvertGlucoseForDisplay(const CMStringW &originalValue, bool bApiMgdl, bool bUseMgdl)
{
	if (originalValue.IsEmpty())
		return CMStringW();
	
	double value = _wtof(originalValue.c_str());
	
	if (bApiMgdl && bUseMgdl) {
		// API mg/dL -> Display mg/dL (no conversion)
		CMStringW result(FORMAT, L"%.1f", value);
		if (result.Right(2) == L".0")
			result.Truncate(result.GetLength() - 2);
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
		CMStringW result(FORMAT, L"%.1f", value * 18.0);
		if (result.Right(2) == L".0")
			result.Truncate(result.GetLength() - 2);
		return result;
	}
	else {
		// API mmol/L -> Display mmol/L (no conversion)
		CMStringW result(FORMAT, L"%.1f", value);
		if (result.Right(2) == L".0")
			result.Truncate(result.GetLength() - 2);
		return result;
	}
}

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

	const time_t expires = activation + 14 * 24 * 60 * 60;
	time_t now = time(0);
	if (expires <= now)
		return TranslateT("Expired");

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

		CMStringW originalValue = GetDbText(m_hContact, "Value");
		CLibreViewProto *ppro = g_plugin.getInstance(m_hContact);
		const bool bUseMgdl = ppro && ppro->DisplayUnits == 1;
	
		// Get API units from database
		const int apiUnits = ppro ? ppro->getDword(m_hContact, "GlucoseUnits", 0) : 0;
		const bool bApiMgdl = apiUnits == 1;
	
		// Convert original API value to display units
		CMStringW displayValue = ConvertGlucoseForDisplay(originalValue, bApiMgdl, bUseMgdl);
	
		const wchar_t *pwszUnit = GetLocalizedUnitByKey(bUseMgdl ? L"mg/dL" : L"mmol/L");
		CMStringW current;
		if (!displayValue.IsEmpty())
			current.Format(L"%s %s", displayValue.c_str(), pwszUnit);

		SetDlgItemText(m_hwnd, IDC_PATIENT, patient);
		SetDlgItemText(m_hwnd, IDC_CURRENT, current);
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
			extern uint32_t ParseLibreTimestamp(const CMStringW &timestamp); // Forward declaration
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
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (g_plugin.getInstance(szProto) == nullptr)
		return 0;

	USERINFOPAGE uip = {};
	uip.szTitle.a = LPGEN("LibreView");
	uip.position = 100000000;
	uip.flags = ODPF_BOLDGROUPS;
	uip.pDialog = new CLibreViewUserInfoDlg();
	g_plugin.addUserInfo(wParam, &uip);
	return 0;
}
