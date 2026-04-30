#include "stdafx.h"

static CMStringW GetDbText(MCONTACT hContact, const char *pszSetting, const wchar_t *pwszDefault = L"")
{
	ptrW value(db_get_wsa(hContact, DB_MODULE_GLUCOSE, pszSetting));
	return value ? CMStringW(value) : CMStringW(pwszDefault);
}

static CMStringW GetSensorRemaining(MCONTACT hContact)
{
	uint32_t activation = db_get_dw(hContact, DB_MODULE_GLUCOSE, "SensorActivationTime", 0);
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
		CMStringW patient = GetDbText(m_hContact, "PatientName");
		if (patient.IsEmpty()) {
			char *szProto = Proto_GetBaseAccountName(m_hContact);
			if (szProto)
				patient = db_get_wsm(m_hContact, szProto, "Nick", L"");
		}
		if (patient.IsEmpty())
			patient = TranslateT("LibreView");

		CMStringW value = GetDbText(m_hContact, "Value");
		CMStringW trend = GetDbText(m_hContact, "TrendSymbol");
		CMStringW unit = GetDbText(m_hContact, "Unit");
		CMStringW current;
		if (!value.IsEmpty())
			current.Format(L"%s%s %s", value.c_str(), trend.c_str(), unit.c_str());

		CMStringW targetLow = GetDbText(m_hContact, "TargetLow");
		CMStringW targetHigh = GetDbText(m_hContact, "TargetHigh");
		if (!targetLow.IsEmpty())
			targetLow.Append(L" mmol/L");
		if (!targetHigh.IsEmpty())
			targetHigh.Append(L" mmol/L");

		SetDlgItemText(m_hwnd, IDC_PATIENT, patient);
		SetDlgItemText(m_hwnd, IDC_CURRENT, current);
		SetDlgItemText(m_hwnd, IDC_SENSOR_ACTIVATION, GetDbText(m_hContact, "SensorActivation"));
		SetDlgItemText(m_hwnd, IDC_SENSOR_REMAINING, GetSensorRemaining(m_hContact));
		SetDlgItemText(m_hwnd, IDC_TARGET_LOW, targetLow);
		SetDlgItemText(m_hwnd, IDC_TARGET_HIGH, targetHigh);
		SetDlgItemText(m_hwnd, IDC_LAST_UPDATE, GetDbText(m_hContact, "Timestamp"));
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
