/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

COptionsDlg::COptionsDlg(int dlgId) :
	CSuper(dlgId),
	m_settings(this, IDC_SETTINGS),
	spin(this, IDC_SPIN, 1024),
	msg1(this, IDC_SETTINGMSG),
	msg2(this, IDC_SETTINGMSG2),
	edtNumber(this, IDC_SETTINGNUMBER),
	chkAdvanced(this, IDC_USEADVANCED),
	chkShowDelivery(this, IDC_SHOWDELIVERYMSGS)
{
	CreateLink(chkAdvanced, g_plugin.bUseAdvanced);
	CreateLink(chkShowDelivery, g_plugin.bShowDelivery);

	m_settings.OnSelChange = Callback(this, &COptionsDlg::onSelChange_Settings);
}

bool COptionsDlg::OnInitDialog()
{
	CSuper::OnInitDialog();

	m_settings.AddString(TranslateT("Send If My Status Is..."));
	m_settings.AddString(TranslateT("Send If They Change Status to..."));
	m_settings.AddString(L"----------------------------");
	m_settings.AddString(TranslateT("Reuse Pounce"));
	m_settings.AddString(TranslateT("Give Up delay"));
	m_settings.AddString(TranslateT("Confirmation Window"));
	m_settings.SetCurSel(g_plugin.getByte(hContact, "LastSetting"));
	onSelChange_Settings(0);
	return true;
}

bool COptionsDlg::OnApply()
{
	CSuper::OnApply();

	saveLastSetting();
	return true;
}

void COptionsDlg::OnDestroy()
{
	if (SendIfMy)
		DestroyWindow(SendIfMy);
	if (SendWhenThey)
		DestroyWindow(SendWhenThey);
}

void COptionsDlg::onSelChange_Settings(CCtrlListBox*)
{
	if (m_bInitialized)
		saveLastSetting();

	int item = m_settings.GetCurSel();
	switch (item) {
	case 0: // Send If My Status Is...
		showAll(false);
		if (m_bInitialized)
			statusModes(true);
		break;

	case 1: // Send If They Change status to
		showAll(false);
		if (m_bInitialized)
			statusModes(false);
		break;

	case 3: // Reuse Pounce
		showAll(true);
		msg1.SetText(TranslateT("Reuse this message? (0 to use it once)"));
		msg2.SetText(TranslateT("Times"));
		edtNumber.SetInt(g_plugin.getByte(hContact, "Reuse"));
		break;

	case 4: // Give Up delay
		showAll(true);
		msg1.SetText(TranslateT("Give up after... (0 to not give up)"));
		msg2.SetText(TranslateT("Days"));
		edtNumber.SetInt(g_plugin.getByte(hContact, "GiveUpDays"));
		break;

	case 5:	// confirm window
		showAll(true);
		msg1.SetText(TranslateT("Show confirmation window? (0 to not Show)"));
		msg2.SetText(TranslateT("Seconds to wait before sending"));
		edtNumber.SetInt(g_plugin.getWord(hContact, "ConfirmTimeout"));
		break;
	}
	g_plugin.setByte(hContact, "LastSetting", (uint8_t)item);
	NotifyChange();
}

void COptionsDlg::saveLastSetting()
{
	switch (g_plugin.getByte(hContact, "LastSetting", 2)) {
	case 3: // Reuse Pounce
		g_plugin.setByte(hContact, "Reuse", (uint8_t)edtNumber.GetInt());
		break;
	case 4: // Give Up delay
		g_plugin.setByte(hContact, "GiveUpDays", (uint8_t)edtNumber.GetInt());
		g_plugin.setDword(hContact, "GiveUpDate", (uint32_t)edtNumber.GetInt() * SECONDSINADAY);
		break;
	case 5:	// confirm window
		g_plugin.setWord(hContact, "ConfirmTimeout", (uint16_t)edtNumber.GetInt());
		break;
	}
}

void COptionsDlg::showAll(bool bShow)
{
	msg1.Show(bShow);
	msg2.Show(bShow);
	spin.Show(bShow);
	edtNumber.Show(bShow);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Status modes dialog wrapper

class CStatusModesDlg : public CDlgBase
{
	COptionsDlg *pDlg;
	bool isMe;

	CCtrlCheck chk1, chk2, chk3, chk4, chk5, chk6, chk7, chk8;

public:
	CStatusModesDlg(COptionsDlg *_1, bool _2) :
		CDlgBase(g_plugin, IDD_STATUSMODES),
		pDlg(_1),
		isMe(_2),
		chk1(this, IDC_CHECK1),
		chk2(this, IDC_CHECK2),
		chk3(this, IDC_CHECK3),
		chk4(this, IDC_CHECK4),
		chk5(this, IDC_CHECK5),
		chk6(this, IDC_CHECK6),
		chk7(this, IDC_CHECK7), 
		chk8(this, IDC_CHECK8)
	{
		SetParent(pDlg->GetHwnd());
	}

	bool OnInitDialog() override
	{
		int statusFlag;

		if (isMe) {
			pDlg->SendIfMy = m_hwnd;
			statusFlag = g_plugin.getWord(pDlg->hContact, "SendIfMyStatusIsFLAG", 0);
			SetCaption(TranslateT("Send If My Status Is"));
			chk1.SetText(TranslateT("Any"));
			chk2.SetText(TranslateT("Online"));
			chk3.SetText(TranslateT("Away"));
			chk4.SetText(TranslateT("Not available"));
			chk5.SetText(TranslateT("Occupied"));
			chk6.SetText(TranslateT("Do not disturb"));
			chk7.SetText(TranslateT("Free for chat"));
			chk8.SetText(TranslateT("Invisible"));
		}
		else {
			pDlg->SendWhenThey = m_hwnd;
			statusFlag = g_plugin.getWord(pDlg->hContact, "SendIfTheirStatusIsFLAG", 0);
			SetCaption(TranslateT("Send If Their Status changes"));
			chk1.SetText(TranslateT("From Offline"));
			chk2.SetText(TranslateT("To Online"));
			chk3.SetText(TranslateT("To Away"));
			chk4.SetText(TranslateT("To Not available"));
			chk5.SetText(TranslateT("To Occupied"));
			chk6.SetText(TranslateT("To Do not disturb"));
			chk7.SetText(TranslateT("To Free for chat"));
			chk8.SetText(TranslateT("To Invisible"));
		}

		chk1.SetState((statusFlag & ANY) != 0);
		chk2.SetState((statusFlag & ONLINE) != 0);
		chk3.SetState((statusFlag & AWAY) != 0);
		chk4.SetState((statusFlag & NA) != 0);
		chk5.SetState((statusFlag & OCCUPIED) != 0);
		chk6.SetState((statusFlag & DND) != 0);
		chk7.SetState((statusFlag & FFC) != 0);
		chk8.SetState((statusFlag & INVISIBLE) != 0);
		return true;
	}

	bool OnApply() override
	{
		int flag = chk1.GetState()
			| (chk2.GetState() << 1)
			| (chk3.GetState() << 2)
			| (chk4.GetState() << 3)
			| (chk5.GetState() << 4)
			| (chk6.GetState() << 5)
			| (chk7.GetState() << 6)
			| (chk8.GetState() << 7);

		if (isMe)
			g_plugin.setWord(pDlg->hContact, "SendIfMyStatusIsFLAG", flag);
		else
			g_plugin.setWord(pDlg->hContact, "SendIfTheirStatusIsFLAG", flag);
		return true;
	}

	void OnDestroy() override
	{
		if (isMe)
			pDlg->SendIfMy = nullptr;
		else
			pDlg->SendWhenThey = nullptr;
	}
};

void COptionsDlg::statusModes(bool isMe)
{
	if (isMe) {
		if (SendIfMy)
			SetForegroundWindow(SendIfMy);
		else
			(new CStatusModesDlg(this, true))->Create();
	}
	else {
		if (SendWhenThey)
			SetForegroundWindow(SendWhenThey);
		else
			(new CStatusModesDlg(this, false))->Create();
	}
}
