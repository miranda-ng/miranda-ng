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

#pragma once

/// #warning: redeclaring....quick programming ;)
/// look at stdafx.h
#ifndef REFRESH_OPTS_MSG
	#define REFRESH_OPTS_MSG WM_USER + 666
#endif

#include <commctrl.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlctrls.h>

/// here is a wrapper on options dlg
class COptionsDlg : public CDialogImpl<COptionsDlg>, public CMessageFilter
{
public:	
	COptionsDlg(void);
	virtual ~COptionsDlg(void);
public:
	enum {IDD = IDD_OPTIONS};
	BEGIN_MSG_MAP(COptionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(REFRESH_OPTS_MSG, OnRefreshOptions)
		COMMAND_ID_HANDLER(IDC_BTN_ADDRULE, OnBtnAddRuleClick)
		COMMAND_ID_HANDLER(IDC_BTN_DELRULE, OnBtnDeleteRuleClick)
		COMMAND_ID_HANDLER(IDC_BTN_EDITRULE, OnBtnEditRuleClick)
		COMMAND_ID_HANDLER(IDC_EDIT_REPLAYDELAY, OnEditReply)
		COMMAND_ID_HANDLER(IDC_EDIT_HEADER, OnEditHeader)
		COMMAND_ID_HANDLER(IDC_EDIT_MESSAGE, OnEditMessage)
		COMMAND_CODE_HANDLER(LBN_SELCHANGE, OnListBoxSelChanged)
		COMMAND_HANDLER(IDC_LIST_RULES, LBN_DBLCLK, OnLbnDblclkListRules)
		COMMAND_HANDLER(IDC_CHECK_DISABLEWMON, BN_CLICKED, OnBnClickedCheck)
		COMMAND_HANDLER(IDC_CH_ENABLE_WSCH, BN_CLICKED, OnBnCheckEnableWhenModeIsOn)
		COMMAND_HANDLER(IDC_CH_SHOWCG, BN_CLICKED, OnBnCheckEnableWhenModeIsOn)
		COMMAND_HANDLER(IDC_CH_SAVE_AURS, BN_CLICKED, OnCheckSaveToHistory)
		COMMAND_HANDLER(IDC_CB_ENWSC, CBN_SELENDOK, OnCbnSelchangeCbEnwsc)
		COMMAND_HANDLER(IDC_CH_DISWHSB, BN_CLICKED, OnBnCheckDisableWhenModeIsOn)
		COMMAND_HANDLER(IDC_CB_DWSB, CBN_SELENDOK, OnCbnSelchangeCbDisnwsc)
	END_MSG_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	/// messages handlers prototypes
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefreshOptions(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	/// commands handlers prototypes
	LRESULT OnBtnAddRuleClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBtnDeleteRuleClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBtnEditRuleClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditReply(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditHeader(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnListBoxSelChanged(WORD wParam, WORD wParam2, HWND hwnd, BOOL & bHandled); 
	LRESULT OnBnClickedCheck(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnCheckEnableWhenModeIsOn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeCbEnwsc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnCheckDisableWhenModeIsOn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCheckSaveToHistory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeCbDisnwsc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLbnDblclkListRules(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
public:
	static BOOL CALLBACK DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	void OnNotifyAboutChanges(void);
	void OnSetSelInRulesList(int nIndex);
protected:
	CEdit m_editReplayDelay;		/// edit box where reply delay is specifyed
	CEdit m_editMessageText;		/// edit box where message text is specifyed
	CListBox m_listRules;			/// list box where all rules are specifyed
	CButton m_btnAdd;				/// button add rule
	CButton m_btnDel;				/// button delete rule
	CButton m_btnChange;			/// button change rule
	CWindow m_wndCheckDOnSOn;		/// checkbox that specifyes dis of plug. when mir is on
	CComboBox m_cbModeTypes;		/// cb mode types
	CWindow m_wndCheckModeTypes;	/// ch mode types
	CWindow m_chbShowAurWhmc;		/// ch show edit reply message
	CWindow m_chDisWMB;				/// ch disable when mode becomes...
	CWindow m_chSaveAURSToHist;		/// save aurs to history
	CComboBox m_cbDisWMBTypes;		/// cb disable when mode becomes...
private:	
	LPTSTR	m_szMessage;
	int		m_nReplayDelay;
	bool	m_bDestroying;
protected:
	bool m_bShown;
public:
	HWND m_hwndHolder;
};

/// here is static dlg proc for options dialog..
/// very bad but mirand can accept only this function
BOOL CALLBACK DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);