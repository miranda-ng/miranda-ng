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
#include "../resource.h"
#include "addruledlg.h"
#include "addruledlg.h"
#include "selectuserdlg.h"

extern LPTSTR g_strPluginName;
extern CMessagesHandler *g_pMessHandler;
extern CCrushLog CRUSHLOGOBJ;

CAddRuleDlg::CAddRuleDlg(void) : m_dwCRC32(NULL), m_baur2thisMode(false), m_bEditing(false)
{
BEGIN_PROTECT_AND_LOG_CODE
	ZeroMemory(&m_item, sizeof(m_item));
END_PROTECT_AND_LOG_CODE
}

CAddRuleDlg::~CAddRuleDlg(void)
{
BEGIN_PROTECT_AND_LOG_CODE
END_PROTECT_AND_LOG_CODE
}

LRESULT CAddRuleDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE	
	TranslateDialogDefault(m_hWnd);
	m_editRuleName = GetDlgItem(IDC_ED_RULENAME);
	m_editContactName = GetDlgItem(IDC_ED_RULECNAME);
	m_editReplyText = GetDlgItem(IDC_ED_RULEREPLY);
	//m_editReplyAction = GetDlgItem(IDC_ED_RULEACTION);
	m_btnSelUser = GetDlgItem(IDC_BTN_SELUSER);

	CenterWindow();
	
	if (m_dwCRC32)
	{
		SetWindowText(TranslateTS(TEXT("Edit rule")));
		m_editRuleName.SetWindowText(m_item.RuleName);
		m_editContactName.SetWindowText(m_item.ContactName);
		m_editReplyText.SetWindowText(m_item.ReplyText);
		//m_editReplyAction.SetWindowText(m_item.ReplyAction);
		m_bEditing = true;
	}
	else if (m_baur2thisMode)
	{
		m_editRuleName.SetWindowText(m_item.RuleName);
		m_editContactName.SetWindowText(m_item.ContactName);
		m_editReplyText.SetWindowText(m_item.ReplyText);
		//m_editReplyAction.SetWindowText(m_item.ReplyAction);

		m_btnSelUser.EnableWindow(FALSE);
		m_editContactName.EnableWindow(FALSE);
		m_editReplyText.SetSel(0, _tcslen(m_item.ReplyText), FALSE);
		m_editReplyText.SetFocus();
	}
	else
	{
		m_editRuleName.SetWindowText(SETTINGS_DEF_RULE_NAME);
		m_editReplyText.SetWindowText(SETTINGS_DEF_MESSAGE_RULE);
	}

END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT CAddRuleDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	return FALSE;
END_PROTECT_AND_LOG_CODE
}

BOOL CAddRuleDlg::PreTranslateMessage(MSG* pMsg)
{
BEGIN_PROTECT_AND_LOG_CODE
	return FALSE;
END_PROTECT_AND_LOG_CODE
}

LRESULT CAddRuleDlg::OnBtnOKClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
#define ON_ERROR_1BYTE(x) if (x == 1)	{								\
					MessageBox(TranslateTS(TEXT("Please, specify longer contact name")), g_strPluginName, MB_OK);	\
					return FALSE;		}								

#define ON_ERROR(x) MessageBox(TranslateTS(x), g_strPluginName, MB_OK);	\
					return FALSE;

	int nLength = m_editRuleName.GetWindowTextLength();
	if (nLength)
	{
		nLength++;
		m_item.RuleName = new TCHAR[nLength];

		if (m_item.RuleName != NULL)
		{
			memset(m_item.RuleName, 0, nLength * sizeof(TCHAR));
			m_editRuleName.GetWindowText(m_item.RuleName, nLength);
		}
	}
	else
	{
		ON_ERROR(TEXT("Please, specify Rule Name"))
	}
	
	nLength = m_editContactName.GetWindowTextLength();
	if (nLength)
	{
		ON_ERROR_1BYTE(nLength)
		nLength++;
		m_item.ContactName = new TCHAR[nLength];

		if (m_item.ContactName != NULL)
		{
			memset(m_item.ContactName, 0, nLength * sizeof(TCHAR));
			m_editContactName.GetWindowText(m_item.ContactName, nLength);
		}
	}
	else
	{
		ON_ERROR(TEXT("Please, specify Contact Name"))
	}

	nLength = m_editReplyText.GetWindowTextLength();
	if (nLength)
	{
		if (nLength > SETTINGS_MESSAGE_MAXVALENGTH)
		{
			MessageBox(TranslateTS(TEXT("header is too big")), g_strPluginName, MB_OK);
			return FALSE;
		}

		nLength++;
		m_item.ReplyText = new TCHAR[nLength];

		if (m_item.ContactName != NULL)
		{
			memset(m_item.ReplyText, 0, nLength * sizeof(TCHAR));
			m_editReplyText.GetWindowText(m_item.ReplyText, nLength);
		}		
	}
	else
	{
		ON_ERROR(TEXT("Please, specify Reply Text"))
	}	
	
	{
		if (m_bEditing)
		{/// delete prev item from storage...
			g_pMessHandler->getSettings().getStorage().RemReplyAction(m_dwCRC32);
		}

		bool b;
		DWORD dwCrc = g_pMessHandler->getSettings().AddReplyAction(m_item, b);
		if (b)
		{
			MessageBox(TranslateTS(TEXT("Rule with the same contact name already exists")), g_strPluginName, MB_OK);
			return FALSE;
		}
		
		m_bAddedOk = ((b == false) && dwCrc);
		m_dwCRC32 = dwCrc;
	}

	EndDialog(0);	
#undef ON_ERROR
#undef ON_ERROR_1BYTE
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT CAddRuleDlg::OnBtnCancelClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	m_bAddedOk = false;
	EndDialog(0);	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}
LRESULT CAddRuleDlg::OnBnClickedBtnSeluser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	CSelectUserDlg dlg;
	dlg.DoModal(m_hWnd);
	if (dlg.m_bAllOk == true)
	{
		m_editContactName.SetWindowText(dlg.m_szRetVal);
		m_editReplyText.SetFocus();
	}
	else
	{
		m_editContactName.SetFocus();
	}
END_PROTECT_AND_LOG_CODE
	return 0;
}
