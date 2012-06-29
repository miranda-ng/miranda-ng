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
#include "selectuserdlg.h"
#include "selectuserdlg.h"

extern LPTSTR g_strPluginName;
extern CMessagesHandler * g_pMessHandler;


CSelectUserDlg::CSelectUserDlg(void) : m_bAllOk(false)
{
}

CSelectUserDlg::~CSelectUserDlg(void)
{
}

BOOL CSelectUserDlg::PreTranslateMessage(MSG* pMsg)
{
	return IsDialogMessage(pMsg);
}

LRESULT CSelectUserDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{	
BEGIN_PROTECT_AND_LOG_CODE
	TranslateDialogDefault(m_hWnd);
	m_listUsers = GetDlgItem(IDC_LIST_USERS);	
	RebuildCL();
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

void CSelectUserDlg::RebuildCL()
{
BEGIN_PROTECT_AND_LOG_CODE
	HANDLE hContact= reinterpret_cast<HANDLE> (CallService(MS_DB_CONTACT_FINDFIRST, 0, 0));
	TCHAR* szContactName = NULL;
	TCHAR* szProto = NULL;
	DWORD wId = 0;
	while (hContact != NULL)
	{
		szContactName = g_pMessHandler->GetContactName(hContact);		
		szProto = (TCHAR*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (szProto)
		{
			size_t nPos = -1;
			if (szContactName)
			{
				nPos = m_listUsers.AddString(szContactName);
			}

			/*DBVARIANT db = {0};
			DBGetContactSetting(hContact, szProto, "uin", &db);
			DBGetContactSetting(hContact, szProto, "UIN", &db);*/
			//m_listUsers.SetItemData(nPos, )
		}

		hContact = reinterpret_cast<HANDLE> (CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM) hContact, 0));
	}
END_PROTECT_AND_LOG_CODE
}

LRESULT CSelectUserDlg::OnBtnOKClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	INT nsel = m_listUsers.GetCurSel();	
	m_bAllOk = (nsel != -1);
	if (m_bAllOk == true)
	{
		m_listUsers.GetText(nsel, m_szRetVal);
		EndDialog(0);
	}
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT CSelectUserDlg::OnBtnCancelClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	m_bAllOk = false;
	EndDialog(0);
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT CSelectUserDlg::OnLbnDblclkListUsers(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	return OnBtnOKClicked(wNotifyCode, wID, hWndCtl, bHandled);
}
