/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
 *
 *      This file is part of SAR.
 *
 *  SAR is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SAR is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SAR.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "optionsdlg.h"
#include "addruledlg.h"

/// need ptr to get/set info
extern CMessagesHandler *g_pMessHandler;
extern CCrushLog CRUSHLOGOBJ;
extern LPTSTR g_strPluginName;

//COptionsDlg g_optionsDlg;

BOOL CALLBACK COptionsDlg::DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
BEGIN_PROTECT_AND_LOG_CODE

	/*if (g_optionsDlg.m_bDestroying)
		return FALSE;

	if (g_optionsDlg.m_hWnd == NULL)
		g_optionsDlg.Attach(hwndDlg);

	LRESULT lre		= {0};
	DWORD	dwId	= {0};
	return g_optionsDlg.ProcessWindowMessage(hwndDlg, msg, wParam, lParam, lre, dwId);*/

END_PROTECT_AND_LOG_CODE
	return FALSE;
}

COptionsDlg::COptionsDlg(void) : m_bShown(false), m_szMessage(NULL)
{
BEGIN_PROTECT_AND_LOG_CODE
///g_optionsDlg.m_bDestroying = false;
m_bDestroying = false;
END_PROTECT_AND_LOG_CODE
}

COptionsDlg::~COptionsDlg(void)
{
BEGIN_PROTECT_AND_LOG_CODE
END_PROTECT_AND_LOG_CODE
}

BOOL COptionsDlg::PreTranslateMessage(MSG* pMsg)
{
BEGIN_PROTECT_AND_LOG_CODE
	return ::IsDialogMessage(m_hWnd, pMsg);
END_PROTECT_AND_LOG_CODE
}

/// process OnInitDialog
LRESULT COptionsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	TranslateDialogDefault(m_hWnd);

	if (!g_pMessHandler)
		return FALSE;

	m_listRules = GetDlgItem(IDC_LIST_RULES);
	m_editReplayDelay = GetDlgItem(IDC_EDIT_REPLAYDELAY);
//	m_editHeaderMessage = GetDlgItem(IDC_EDIT_HEADER);
	m_editMessageText = GetDlgItem(IDC_EDIT_MESSAGE);
	m_btnAdd = GetDlgItem(IDC_BTN_ADDRULE);
	m_btnDel = GetDlgItem(IDC_BTN_DELRULE);
	m_btnChange = GetDlgItem(IDC_BTN_EDITRULE);
	m_wndCheckDOnSOn = GetDlgItem(IDC_CHECK_DISABLEWMON);
	m_wndCheckModeTypes = GetDlgItem(IDC_CH_ENABLE_WSCH);
	m_cbModeTypes = GetDlgItem(IDC_CB_ENWSC);
	m_chbShowAurWhmc = GetDlgItem(IDC_CH_SHOWCG);
	m_cbDisWMBTypes = GetDlgItem(IDC_CB_DWSB);
	m_chDisWMB = GetDlgItem(IDC_CH_DISWHSB);
	m_chSaveAURSToHist = GetDlgItem(IDC_CH_SAVE_AURS);

	m_cbModeTypes.AddString(TranslateTS(TEXT("DND/NA/Away/Occupied modes")));
	m_cbModeTypes.AddString(TranslateTS(TEXT("DND mode")));
	m_cbModeTypes.AddString(TranslateTS(TEXT("NA mode")));
	m_cbModeTypes.AddString(TranslateTS(TEXT("Away mode")));
	m_cbModeTypes.AddString(TranslateTS(TEXT("Occupied mode")));

	m_cbDisWMBTypes.AddString(TranslateTS(TEXT("Online/Free for chat modes")));
	m_cbDisWMBTypes.AddString(TranslateTS(TEXT("Online mode")));
	m_cbDisWMBTypes.AddString(TranslateTS(TEXT("Free for chat mode")));	

	COMMON_RULE_ITEM & commRules = g_pMessHandler->getSettings().getStorage().getCommonRule();
	REPLYER_SETTINGS & settings = g_pMessHandler->getSettings().getSettings();	

	if (settings.ModeValue > m_cbModeTypes.GetCount())
		m_cbModeTypes.SetCurSel(0);
	else
		m_cbModeTypes.SetCurSel(settings.ModeValue);
	if (settings.ModeDisValue > m_cbDisWMBTypes.GetCount())
		m_cbDisWMBTypes.SetCurSel(0);
	else
		m_cbDisWMBTypes.SetCurSel(settings.ModeDisValue);

	m_cbDisWMBTypes.EnableWindow(settings.bDisableWhenModeIsSet);

	if (settings.bDisableWhenModeIsSet)
	{
		m_chDisWMB.SendMessage(BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		m_chDisWMB.SendMessage(BM_SETCHECK, BST_UNCHECKED, 0);
	}

	if (settings.bShowAURDlgWhenModeChanges)
	{
		m_chbShowAurWhmc.SendMessage(BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		m_chbShowAurWhmc.SendMessage(BST_UNCHECKED, BST_CHECKED, 0);
	}

	if (settings.bEnableWhenModeIsSet)
	{
		m_wndCheckModeTypes.SendMessage(BM_SETCHECK, BST_CHECKED, 0);		
	}
	else
	{
		m_wndCheckModeTypes.SendMessage(BST_UNCHECKED, BST_CHECKED, 0);
	}

	if (settings.bSaveToHistory)
	{
		m_chSaveAURSToHist.SendMessage(BM_SETCHECK, BST_CHECKED, 0);		
	}
	else
	{
		m_chSaveAURSToHist.SendMessage(BST_UNCHECKED, BST_CHECKED, 0);
	}

	m_cbModeTypes.EnableWindow(settings.bEnableWhenModeIsSet);

	int nLength = settings.ReplayDelay;
	if (nLength == 0)
		nLength = SETTINGS_DEF_COMMON_VALREPDELAY;

	if (settings.bDisableWhenMirandaIsOn)
		m_wndCheckDOnSOn.SendMessage(BM_SETCHECK, BST_CHECKED, 0);
	else
		m_wndCheckDOnSOn.SendMessage(BM_SETCHECK, BST_UNCHECKED, 0);

	TCHAR strNumber[MAX_PATH] = {0};
	m_editReplayDelay.SetWindowText(_itot(nLength, strNumber, 10));
	m_nReplayDelay = nLength;

	TCHAR* str = commRules.Message;
	m_editMessageText.SetWindowText(str);	
		
	if (m_szMessage)
	{
		delete m_szMessage;
		m_szMessage = NULL;
	}

	m_szMessage = new TCHAR[SETTINGS_MESSAGE_MAXVALENGTH];

	if (!m_szMessage)
		return FALSE;

	memset(m_szMessage, 0, SETTINGS_MESSAGE_MAXVALENGTH * sizeof(TCHAR));

	m_editMessageText.GetWindowText(m_szMessage, SETTINGS_MESSAGE_MAXVALENGTH);

	{ /// filling listbox with rule names...
		RulesHash::iterator it;
		RulesHash & hash = g_pMessHandler->getSettings().getStorage().getHashTable();
		int nPos = 0;

		bool bAdded = false;
		for (it = hash.begin(); it != hash.end(); it++)
		{
			nPos = m_listRules.AddString(it->second.RuleName);
			if (nPos != -1)
			{
				m_listRules.SetItemData(nPos, it->first);
				if (!bAdded)
					bAdded = true;
			}
		}
		if (bAdded)
		{
			OnSetSelInRulesList(0);			
		}
	}
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

/// process WM_DESTROY
LRESULT COptionsDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	m_bDestroying = true;
	///g_optionsDlg.m_bDestroying = true;
	///::DestroyWindow(m_hWnd);
	///g_optionsDlg.m_hWnd = NULL;
	//m_hWnd = NULL;
	m_szMessage = NULL;	
	// m_bDestroying = false;
	///g_optionsDlg.m_bDestroying = false;
	return FALSE;
END_PROTECT_AND_LOG_CODE
}

void COptionsDlg::OnNotifyAboutChanges(void)
{
BEGIN_PROTECT_AND_LOG_CODE
	SendMessage(m_hwndHolder, PSM_CHANGED, 0, 0);
END_PROTECT_AND_LOG_CODE
}

/// handler of typing in replay delay edit box
LRESULT COptionsDlg::OnEditReply(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{	
BEGIN_PROTECT_AND_LOG_CODE
	if (m_bShown)
	{
		m_bShown = false;
		return FALSE;
	}
	TCHAR tch[MAX_PATH] = {0};
	GetDlgItemText (IDC_EDIT_REPLAYDELAY, tch, sizeof(tch));
	int nValue = _tstoi(tch);

	if (nValue != m_nReplayDelay)
	{
		m_nReplayDelay = nValue;
		OnNotifyAboutChanges();
	}

END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	switch (((LPNMHDR)lParam)->code)
	{
		case PSN_APPLY:
		{	/// apply our changes...
			REPLYER_SETTINGS sett;
			COMMON_RULE_ITEM commrule;
			
			commrule.Message = m_szMessage;

			sett.bEnabled = g_pMessHandler->getSettings().getSettings().bEnabled;
			sett.ReplayDelay = m_nReplayDelay;

			LRESULT lres = m_wndCheckDOnSOn.SendMessage(BM_GETCHECK, 0, 0);
			sett.bDisableWhenMirandaIsOn = (lres == BST_CHECKED);
			lres = m_wndCheckModeTypes.SendMessage(BM_GETCHECK, 0, 0);
			sett.bEnableWhenModeIsSet = (lres == BST_CHECKED);
			lres = m_chbShowAurWhmc.SendMessage(BM_GETCHECK, 0, 0);
			sett.bShowAURDlgWhenModeChanges = (lres == BST_CHECKED);
			lres = m_chSaveAURSToHist.SendMessage(BM_GETCHECK, 0, 0);
			sett.bSaveToHistory = (lres == BST_CHECKED);
			sett.ModeValue = m_cbModeTypes.GetCurSel();
			lres = m_chDisWMB.SendMessage(BM_GETCHECK, 0, 0);
			sett.bDisableWhenModeIsSet = (lres == BST_CHECKED);
			sett.ModeDisValue = m_cbDisWMBTypes.GetCurSel();

			CSettingsHandler & handler = g_pMessHandler->getSettings();
			handler.setSettings(sett, &commrule);
			//handler.getStorage().Flush();	/// flush all unflushed rules into hdd
		}
	}
	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnEditHeader(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnEditMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	if (!m_szMessage)
		return FALSE;

	int nLength = m_editMessageText.GetWindowTextLength();

	if (nLength > SETTINGS_MESSAGE_MAXVALENGTH)	
	{
		MessageBox(TranslateTS(TEXT("too big size")), g_strPluginName, MB_OK);
		m_editMessageText.SetWindowText(m_szMessage);

		return FALSE;
	}
	nLength++;
	LPTSTR str2 = new TCHAR[nLength];

	if (!str2)
		return FALSE;
	
	memset(str2, 0, nLength * sizeof(TCHAR));
	m_editMessageText.GetWindowText(str2, nLength);

	if (_tcscmp(m_szMessage, str2) != 0)
	{
		_tcscpy(m_szMessage, str2);
		OnNotifyAboutChanges();
	}

	delete str2;
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

void COptionsDlg::OnSetSelInRulesList(int nIndex)
{
BEGIN_PROTECT_AND_LOG_CODE
	m_listRules.SetCurSel(nIndex);
	BOOL b;
	OnListBoxSelChanged(0, 0,  0, b);
END_PROTECT_AND_LOG_CODE
}

LRESULT COptionsDlg::OnBtnAddRuleClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	CAddRuleDlg dlg;
	dlg.DoModal(m_hWnd, 0);
	if (dlg.m_bAddedOk)
	{
		int npos = m_listRules.AddString(dlg.m_item.RuleName);
		if (npos != -1)
		{
			m_listRules.SetItemData(npos, dlg.m_dwCRC32);
			OnSetSelInRulesList(npos);
			OnNotifyAboutChanges();
		}
	}	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnBtnEditRuleClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	int nIndex = m_listRules.GetCurSel();
	if (nIndex >= 0)
	{
		DWORD dwCrc32 = static_cast<DWORD>(m_listRules.GetItemData(nIndex));
		if (dwCrc32)
		{
			CAddRuleDlg dlgEdit;						
			RulesHash::iterator it;
			RulesHash & hash = g_pMessHandler->getSettings().getStorage().getHashTable();
			it = hash.find(dwCrc32);
			if (it != hash.end())
			{
				dlgEdit.m_item = it->second;
				dlgEdit.m_dwCRC32 = dwCrc32;
				dlgEdit.DoModal();
				if (dlgEdit.m_bAddedOk)
				{/// the rule is edited...
					m_listRules.DeleteString(nIndex);
					nIndex = m_listRules.AddString(dlgEdit.m_item.RuleName);
					m_listRules.SetItemData(nIndex, dlgEdit.m_dwCRC32);
					OnSetSelInRulesList(nIndex);
					OnNotifyAboutChanges();					
				}
			}			
		}
	}	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnBtnDeleteRuleClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	int nIndex = m_listRules.GetCurSel();
	if (nIndex >= 0)
	{
		DWORD dwptr = static_cast<DWORD>(m_listRules.GetItemData(nIndex));
		if (dwptr)/// we have ptr
		{
			if (MessageBox(TranslateTS(TEXT("Do you really want delete selected rule?")), g_strPluginName, MB_YESNO) == IDNO)
				return FALSE;

			CRulesStorage & hash = g_pMessHandler->getSettings().getStorage();

			if (hash.RemReplyAction(dwptr))
			{/// we found item in storage list...
				int nCount = m_listRules.DeleteString(nIndex);
				if (nCount != LB_ERR)
				{	/// set sel to prev one
					if (nIndex == 0 && nCount)
						OnSetSelInRulesList(0);
					else
					if ( (nIndex - 1) <= nCount)
					{
						OnSetSelInRulesList(nIndex - 1);
					}
				}

				OnNotifyAboutChanges();
			}
		}
	}

	BOOL b;
	OnListBoxSelChanged(0, 0, 0, b);
	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnListBoxSelChanged(WORD wParam, WORD w2Param, HWND hwnd, BOOL & bHandled)
{
BEGIN_PROTECT_AND_LOG_CODE
	int nCount = m_listRules.GetCurSel();
	//m_btnChange.EnableWindow((nCount >= 0));
	BOOL bEnable = (nCount >= 0);
	m_btnDel.EnableWindow(bEnable);
	m_btnChange.EnableWindow(bEnable);
	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnBnClickedCheck(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	OnNotifyAboutChanges();

	return FALSE;
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnBnCheckEnableWhenModeIsOn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	LRESULT lres = m_wndCheckModeTypes.SendMessage(BM_GETCHECK, 0, 0);
	m_cbModeTypes.EnableWindow(lres == BST_CHECKED);	
	OnNotifyAboutChanges();	
END_PROTECT_AND_LOG_CODE	
	return FALSE;
}

LRESULT COptionsDlg::OnCbnSelchangeCbEnwsc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	OnNotifyAboutChanges();
	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnCheckSaveToHistory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	OnNotifyAboutChanges();
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnBnCheckDisableWhenModeIsOn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	LRESULT lres = m_chDisWMB.SendMessage(BM_GETCHECK, 0, 0);
	m_cbDisWMBTypes.EnableWindow(lres == BST_CHECKED);
	OnNotifyAboutChanges();
	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnCbnSelchangeCbDisnwsc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	OnNotifyAboutChanges();
	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

/// options are changed in another place...
/// so just update them all...
LRESULT COptionsDlg::OnRefreshOptions(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	MessageBox (TranslateTS(TEXT("Options are changed in another dialog")), g_strPluginName, MB_OK);
	///OnNotifyAboutChanges();
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT COptionsDlg::OnLbnDblclkListRules(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
BEGIN_PROTECT_AND_LOG_CODE
	return OnBtnEditRuleClick(wNotifyCode, wID, hWndCtl, bHandled);
END_PROTECT_AND_LOG_CODE
}