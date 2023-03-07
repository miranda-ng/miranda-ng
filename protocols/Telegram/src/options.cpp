/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

class COptionsDlg : public CTelegramDlgBase
{
	CCtrlCheck chkHideChats, chkUsePopups;
	CCtrlCombo cmbCountry;
	CCtrlEdit edtGroup, edtPhone, edtDeviceName;
	ptrW m_wszOldGroup;

public:
	COptionsDlg(CTelegramProto *ppro, int iDlgID, bool bFullDlg) :
		CTelegramDlgBase(ppro, iDlgID),
		cmbCountry(this, IDC_COUNTRY),
		chkUsePopups(this, IDC_POPUPS),
		chkHideChats(this, IDC_HIDECHATS),
		edtPhone(this, IDC_PHONE),
		edtGroup(this, IDC_DEFGROUP),
		edtDeviceName(this, IDC_DEVICE_NAME),
		m_wszOldGroup(mir_wstrdup(ppro->m_wszDefaultGroup))
	{
		CreateLink(edtPhone, ppro->m_szOwnPhone);
		CreateLink(edtGroup, ppro->m_wszDefaultGroup);
		CreateLink(edtDeviceName, ppro->m_wszDeviceName);
		CreateLink(chkHideChats, ppro->m_bHideGroupchats);

		if (bFullDlg)
			CreateLink(chkUsePopups, ppro->m_bUsePopups);

		cmbCountry.OnChange = Callback(this, &COptionsDlg::onChange_Country);
	}

	bool OnInitDialog() override
	{
		int iCount;
		CountryListEntry *pList;
		CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&iCount, (LPARAM)&pList);

		for (int i = 0; i < iCount; i++) {
			unsigned countryCode = pList[i].id;
			int idx = cmbCountry.AddString(TranslateW(_A2T(pList[i].szName).get()), countryCode);
			if (countryCode == m_proto->m_iCountry)
				cmbCountry.SetCurSel(idx);
		}
		
		onChange_Country(0);
		return true;
	}

	bool OnApply() override
	{
		int iCountry = cmbCountry.GetCurData();
		if (iCountry == 9999 || !mir_wstrlen(m_proto->m_szOwnPhone)) {
			SetFocus(edtPhone.GetHwnd());
			return false;
		}

		m_proto->m_iCountry = iCountry;

		if (mir_wstrcmp(m_proto->m_wszDefaultGroup, m_wszOldGroup))
			Clist_GroupCreate(0, m_proto->m_wszDefaultGroup);
		return true;
	}

	void onChange_Country(CCtrlCombo *)
	{
		CMStringA buf;
		switch (int iCode = cmbCountry.GetCurData()) {
		case 9999:
		case -1:
			buf = "---";
			break;
		default:
			buf.Format("+%d", iCode);
		}
		SetDlgItemTextA(m_hwnd, IDC_CODE, buf);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Advanced options

class CAdvOptionsDlg : public CTelegramDlgBase
{
	CCtrlEdit edtDiff1, edtDiff2;
	CCtrlSpin spin1, spin2;
	CCtrlCombo cmbStatus1, cmbStatus2;

public:
	CAdvOptionsDlg(CTelegramProto *ppro) :
		CTelegramDlgBase(ppro, IDD_OPTIONS_ADV),
		spin1(this, IDC_SPIN1, 32000),
		spin2(this, IDC_SPIN2, 32000),
		edtDiff1(this, IDC_DIFF1),
		edtDiff2(this, IDC_DIFF2),
		cmbStatus1(this, IDC_STATUS1),
		cmbStatus2(this, IDC_STATUS2)
	{
		edtDiff1.OnChange = Callback(this, &CAdvOptionsDlg::onChange_Timeout1);
		edtDiff2.OnChange = Callback(this, &CAdvOptionsDlg::onChange_Timeout2);

		spin1.OnChange = Callback(this, &CAdvOptionsDlg::onChange_Spin1);
		spin2.OnChange = Callback(this, &CAdvOptionsDlg::onChange_Spin2);

		CreateLink(spin1, ppro->m_iTimeDiff1);
		CreateLink(spin2, ppro->m_iTimeDiff2);
	}

	bool OnInitDialog() override
	{
		for (uint32_t iStatus = ID_STATUS_OFFLINE; iStatus <= ID_STATUS_MAX; iStatus++) {
			int idx = cmbStatus1.AddString(Clist_GetStatusModeDescription(iStatus, 0), iStatus);
			if (iStatus == m_proto->m_iStatus1)
				cmbStatus1.SetCurSel(idx);

			idx = cmbStatus2.AddString(Clist_GetStatusModeDescription(iStatus, 0), iStatus);
			if (iStatus == m_proto->m_iStatus2)
				cmbStatus2.SetCurSel(idx);
		}

		return true;
	}

	bool OnApply() override
	{
		m_proto->m_iStatus1 = cmbStatus1.GetCurData();
		m_proto->m_iStatus2 = cmbStatus2.GetCurData();
		return true;
	}

	void onChange_Value1(int val)
	{
		bool bEnabled = val != 0;
		spin2.Enable(bEnabled);
		edtDiff2.Enable(bEnabled);
		cmbStatus1.Enable(bEnabled);
		cmbStatus2.Enable(bEnabled && spin2.GetPosition() != 0);
	}

	void onChange_Timeout1(CCtrlEdit *)
	{
		onChange_Value1(edtDiff1.GetInt());
	}

	void onChange_Spin1(CCtrlEdit *)
	{
		onChange_Value1(spin1.GetPosition());
	}

	void onChange_Timeout2(CCtrlEdit *)
	{
		cmbStatus2.Enable(edtDiff1.GetInt() != 0 && edtDiff2.GetInt() != 0);
	}

	void onChange_Spin2(CCtrlEdit *)
	{
		cmbStatus2.Enable(spin1.GetPosition() != 0 && spin2.GetPosition() != 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

MWindow CTelegramProto::OnCreateAccMgrUI(MWindow hwndParent)
{
	auto *pDlg = new COptionsDlg(this, IDD_ACCMGRUI, false);
	pDlg->SetParent(hwndParent);
	pDlg->Create();
	return pDlg->GetHwnd();
}

int CTelegramProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");

	odp.position = 1;
	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new COptionsDlg(this, IDD_OPTIONS, true);
	g_plugin.addOptions(wParam, &odp);

	odp.position = 2;
	odp.szTab.w = LPGENW("Advanced");
	odp.pDialog = new CAdvOptionsDlg(this);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
