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
	uint32_t activation = db_get_dw(hContact, MODULENAME, "SensorActivationTime", 0);
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

		CMStringW value = GetDbText(m_hContact, "Value");
		CMStringW unit(GetLocalizedUnitByKey(GetDbText(m_hContact, "Unit")));
		CMStringW current;
		if (!value.IsEmpty())
			current.Format(L"%s %s", value.c_str(), unit.c_str());

		CMStringW targetLow = GetDbText(m_hContact, "TargetLow");
		CMStringW targetHigh = GetDbText(m_hContact, "TargetHigh");
		if (!targetLow.IsEmpty())
			targetLow.AppendFormat(L" %s", unit.c_str());
		if (!targetHigh.IsEmpty())
			targetHigh.AppendFormat(L" %s", unit.c_str());

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
