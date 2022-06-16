/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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
*/

#ifndef _UI_PSP_BASE_INCLUDE_
#define _UI_PSP_BASE_INCLUDE_

/////////////////////////////////////////////////////////////////////////////////////////
// Base page dialog class

class PSPBaseDlg : public CUserInfoPageDlg
{
	friend class CPsTreeItem;

protected:
	CCtrlList *m_ctrlList;

	PSPBaseDlg(int idDialog);

	virtual void OnIconsChanged() {}
	bool OnInitDialog() override;
	bool OnApply() override;
	bool OnRefresh() override;
	void OnReset() override;
	void OnDestroy() override;

	INT_PTR DlgProc(UINT uMsg, WPARAM, LPARAM) override;

	void UpdateCountryIcon(CCtrlCombo &pCombo);

	HFONT GetBoldFont() const;
	MCONTACT GetContact() const;
	const char* GetBaseProto() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

void InitGeneralDlg(WPARAM wParam, USERINFOPAGE &uip);
void InitContactDlg(WPARAM wParam, USERINFOPAGE &uip);
void InitOriginDlg(WPARAM wParam, USERINFOPAGE &uip);
void InitCompanyDlg(WPARAM wParam, USERINFOPAGE &uip);
void InitAnniversaryDlg(WPARAM wParam, USERINFOPAGE &uip);
void InitOriginDlg(WPARAM wParam, USERINFOPAGE &uip, bool bReadOnly);
void InitProfileDlg(WPARAM wParam, USERINFOPAGE &uip);

#endif /* _UI_PSP_BASE_INCLUDE_ */
