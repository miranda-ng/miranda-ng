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
#include "settingsdlgholder.h"

/// #warning 
/// dont make Translate each time
/// instead once fill list of translated strings
/// and use them

extern LPTSTR g_strPluginName;
extern CMessagesHandler * g_pMessHandler;
CSettingsDlgHolder * CSettingsDlgHolder::m_pThis = NULL;

CSettingsDlgHolder::CSettingsDlgHolder(void) : 
IMSingeltone<CSettingsDlgHolder>(this),
m_bTabSelected(false),
m_bDestroying(false)
{
}

CSettingsDlgHolder::~CSettingsDlgHolder(void)
{
BEGIN_PROTECT_AND_LOG_CODE
	BOOL b;
	OnDestroy(NULL, NULL, NULL, b);
END_PROTECT_AND_LOG_CODE
}

BOOL CSettingsDlgHolder::PreTranslateMessage(MSG* pMsg)
{
	return IsDialogMessage(pMsg);
}

LRESULT CSettingsDlgHolder::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{	
BEGIN_PROTECT_AND_LOG_CODE
	TranslateDialogDefault(m_hWnd);

	m_tabCtrl = GetDlgItem(IDC_TABSHOLDER);
	m_statusMsg = GetDlgItem(IDC_STATUSMSG);

	TCITEM ti = {0};
	ti.mask = TCIF_TEXT;
	ti.pszText = TranslateTS(TEXT("General"));
	ti.cchTextMax = _tcslen(ti.pszText);
	m_tabCtrl.InsertItem(0, &ti);
	/*ti.pszText = Translate("About");
	ti.cchTextMax = _tcslen(ti.pszText);
	m_tabCtrl.InsertItem(1, &ti);*/

	m_generalOpt.Create(m_tabCtrl, NULL);
	m_generalOpt.m_hwndHolder = this->m_hWnd;
	m_tabCtrl.SetCurSel(0);

	/// #warning 
	/// copy paste from OnNotify(), fix later
	/*m_statusMsg.SetWindowText(Translate("General options, provides basic functionality."));*/
	m_statusMsg.SetWindowText(TranslateTS(TEXT("")));
	m_generalOpt.SetWindowPos(NULL, 7, 23, NULL, NULL, SWP_NOSIZE);
	m_generalOpt.ShowWindow(SW_SHOW);	

END_PROTECT_AND_LOG_CODE
	return FALSE;
}

void CSettingsDlgHolder::OnNotifyParents(LPNMHDR lpnmhdr, int nCode)
{
BEGIN_PROTECT_AND_LOG_CODE
	lpnmhdr->code = nCode;
	m_generalOpt.SendMessage(WM_NOTIFY, 0, reinterpret_cast<LPARAM>(lpnmhdr));
END_PROTECT_AND_LOG_CODE
}

LRESULT CSettingsDlgHolder::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	LPNMHDR lpnmhdr = NULL;
	lpnmhdr = (LPNMHDR)lParam;
	if (lpnmhdr == NULL)
	{		
		return FALSE;
	}

	if (m_bTabSelected == false)
	{
		m_bTabSelected = true;
		return FALSE;
	}

	switch (lpnmhdr->code)
	{
	case PSN_APPLY:
		OnNotifyParents(lpnmhdr, PSN_APPLY);
		break;
	}

	int nSel = -1;
	if (lpnmhdr->hwndFrom == m_tabCtrl)
	{
		nSel = m_tabCtrl.GetCurSel();
		/// let's make a switch by dialogs...
		if (nSel == 0)
		{
			m_statusMsg.SetWindowText(TranslateTS(TEXT("General options, provides basic functionality.")));
			m_generalOpt.ShowWindow(SW_SHOW);
			m_generalOpt.SetWindowPos(NULL, 7, 23, NULL, NULL, SWP_NOSIZE);
		}
		else
			m_generalOpt.ShowWindow(SW_HIDE);
	}

	m_bTabSelected = false;

END_PROTECT_AND_LOG_CODE
	return FALSE;
}

BOOL CALLBACK CSettingsDlgHolder::FakeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
BEGIN_PROTECT_AND_LOG_CODE
	CSettingsDlgHolder *pHandler = CSettingsDlgHolder::GetObject();	

	if (pHandler == NULL)
		return FALSE;

	if (pHandler->m_bDestroying == true)
		return FALSE;

	if (pHandler->m_hWnd == NULL)
		pHandler->Attach(hwndDlg);
		
	LRESULT lr = 0;
	DWORD dwid = 0;
	BOOL bRetVal = pHandler->ProcessWindowMessage(hwndDlg, msg, wParam, lParam, lr, dwid);
	return bRetVal;
	END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT CSettingsDlgHolder::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	m_bDestroying = true;

	if (m_tabCtrl.IsWindow())
	{
		m_tabCtrl.DeleteAllItems();
		m_tabCtrl.DestroyWindow();
	}
	if (m_statusMsg.IsWindow())
		m_statusMsg.DestroyWindow();

	if (m_generalOpt.IsWindow())
	{
		/*try 
		{*/
			//OSVERSIONINFO ver = {0};
			//ver.dwOSVersionInfoSize = sizeof(ver);

			//if (GetVersionEx(&ver) == TRUE)
			//{
			//	if (ver.dwMajorVersion > 4)
			//	{
					NMHDR h = {0};
					h.code = PSN_APPLY;
					OnNotifyParents(&h, PSN_APPLY);
			//	}
			//}
			m_generalOpt.DestroyWindow();
		//}
		//catch (...)
		//{/// win9x will die here
		//}		
	}
	m_hWnd = NULL;
	
END_PROTECT_AND_LOG_CODE
	return FALSE;
}

LRESULT CSettingsDlgHolder::OnSettingsChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
BEGIN_PROTECT_AND_LOG_CODE
	return SendMessage(GetParent(), PSM_CHANGED, 0, 0);
END_PROTECT_AND_LOG_CODE
	return FALSE;
}