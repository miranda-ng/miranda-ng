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

#include <commctrl.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>

#include "../RuleItem.h"

class CAddRuleDlg : public CDialogImpl<CAddRuleDlg>, public CMessageFilter
{
public:
	CAddRuleDlg(void);
	virtual ~CAddRuleDlg(void);
public:
	enum {IDD = IDD_ADDRULE_DLG};
	BEGIN_MSG_MAP(COptionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		COMMAND_ID_HANDLER(IDOK, OnBtnOKClicked)
		COMMAND_ID_HANDLER(IDCANCEL, OnBtnCancelClicked)
		COMMAND_HANDLER(IDC_BTN_SELUSER, BN_CLICKED, OnBnClickedBtnSeluser)
	END_MSG_MAP()	
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBtnOKClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBtnCancelClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnSeluser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
protected:
	CEdit m_editRuleName;
	CEdit m_editContactName;
	CEdit m_editReplyText;
	//CEdit m_editReplyAction;
	CButton m_btnSelUser;
public:
	RULE_ITEM m_item;
	bool m_bAddedOk;
	DWORD m_dwCRC32;
private:
	bool m_bEditing;
public:
	bool m_baur2thisMode;
};
