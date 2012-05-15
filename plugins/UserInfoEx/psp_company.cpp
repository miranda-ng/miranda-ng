/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

===============================================================================

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/psp_company.cpp $
Revision       : $Revision: 187 $
Last change on : $Date: 2010-09-08 16:05:54 +0400 (Ср, 08 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/

#include "commonheaders.h"
#include "ctrl_combo.h"
#include "ctrl_edit.h"
#include "psp_base.h"

/**
 * Dialog procedure for the company contact information propertysheetpage
 *
 * @param hDlg		- handle to the dialog window
 * @param uMsg		- the message to handle
 * @param wParam	- parameter
 * @param lParam	- parameter
 *
 * @return	different values
 **/
INT_PTR CALLBACK PSPProcCompany(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
	case WM_INITDIALOG:
		{
			CCtrlList *pCtrlList = CCtrlList::CreateObj(hDlg);
			if (pCtrlList)
			{
				LPIDSTRLIST pList;
				UINT nList;
				HFONT hBoldFont;
				PSGetBoldFont(hDlg, hBoldFont);
				SendDlgItemMessage(hDlg, IDC_PAGETITLE, WM_SETFONT, (WPARAM)hBoldFont, 0);

				TranslateDialogDefault(hDlg);

				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_COMPANY,					SET_CONTACT_COMPANY,			DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_DEPARTMENT,				SET_CONTACT_COMPANY_DEPARTMENT,	DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_OFFICE,					SET_CONTACT_COMPANY_OFFICE,		DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_POSITION,					SET_CONTACT_COMPANY_POSITION,	DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_SUPERIOR,					SET_CONTACT_COMPANY_SUPERIOR,	DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_ASSISTENT,				SET_CONTACT_COMPANY_ASSISTENT,	DBVT_TCHAR));

				GetOccupationList(&nList, &pList);
				pCtrlList->insert(   CCombo::CreateObj(hDlg, EDIT_OCCUPATION,				SET_CONTACT_COMPANY_OCCUPATION,	DBVT_WORD,	pList, nList));
			}
		}
	}
	return PSPBaseProc(hDlg, uMsg, wParam, lParam);
}			
