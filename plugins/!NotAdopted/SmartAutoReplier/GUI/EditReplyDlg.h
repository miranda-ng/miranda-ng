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

#include "..\resource.h"

#include <commctrl.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>

class CEditReplyDlg : public CDialogImpl<CEditReplyDlg>, public CMessageFilter
{
public:
	CEditReplyDlg(void);
	~CEditReplyDlg(void);
public:
	enum {IDD = IDD_SET_AURMESSAGEDLG};	
	BEGIN_MSG_MAP(CEditReplyDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnBtnOKClicked)
		COMMAND_ID_HANDLER(IDCANCEL, OnBtnCancelClicked)
	END_MSG_MAP()
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBtnOKClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBtnCancelClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
public:
	REPLYER_SETTINGS m_settings;
	COMMON_RULE_ITEM m_commRule;
	bool	m_bAllOk;
protected:
	CEdit m_editReplyText;
	CEdit m_editReplyHeader;
};
