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

#ifndef _UINFOEX_CTRLANNIVEDIT_H_
#define _UINFOEX_CTRLANNIVEDIT_H_

#include "ctrl_base.h"

#define MAX_DESC	50

class CAnnivEditCtrl : public CBaseCtrl
{
	HWND	_hwndDlg;	 // owning dialog box
	HWND	_hwndDate;	// date picker
	HWND	_hBtnMenu;	// anniversary dropdown button
	HWND	_hBtnEdit;	// edit anniversary button
	HWND	_hBtnAdd;	 // add anniversary button
	HWND	_hBtnDel;	 // delete anniversary button

	BOOLEAN			 _ReminderEnabled;

	MAnnivDate**	_pDates;
	WORD					_numDates;
	WORD					_curDate;

	BOOLEAN ItemValid(WORD wIndex) const;
	BOOLEAN CurrentItemValid() const;

	INT_PTR DBGetBirthDay(HANDLE hContact, LPCSTR pszProto);
	INT_PTR DBWriteBirthDay(HANDLE hContact);

	INT_PTR DBGetAnniversaries(HANDLE hContact);
	INT_PTR DBWriteAnniversaries(HANDLE hContact);

	CAnnivEditCtrl(HWND hDlg, WORD idCtrl, LPCSTR pszSetting);
	~CAnnivEditCtrl();

public:

	MAnnivDate* Current() { return CurrentItemValid() ? _pDates[_curDate] : NULL; };
	WORD				CurrentIndex() const { return _curDate; };
	WORD				NumDates() const { return _numDates; };
	BOOLEAN		 ReminderEnabled() const { return _ReminderEnabled; };

	MAnnivDate* FindDateById(const WORD wId);

	VOID				EnableCurrentItem();
	VOID				EnableReminderCtrl(BOOLEAN bEnabled);

	INT_PTR		 SetCurSel(WORD wIndex);

	INT_PTR		 AddDate(MAnnivDate &mda);
	INT_PTR		 DeleteDate(WORD wIndex);

	VOID				SetZodiacAndAge(MAnnivDate *mt);

	// notification handlers
	VOID OnMenuPopup();
	VOID OnDateChanged(LPNMDATETIMECHANGE lpChange);
	VOID OnRemindEditChanged();
	VOID OnReminderChecked();

	/**
	 * CBaseCtrl interface:
	 **/
	static FORCEINLINE CAnnivEditCtrl* GetObj(HWND hCtrl) 
		{ return (CAnnivEditCtrl*) GetUserData(hCtrl); }
	static FORCEINLINE CAnnivEditCtrl* GetObj(HWND hDlg, WORD idCtrl)
		{ return GetObj(GetDlgItem(hDlg, idCtrl)); }

	static CBaseCtrl* CreateObj(HWND hDlg, WORD idCtrl, LPCSTR pszSetting);

	virtual VOID	Release();
	virtual BOOL	OnInfoChanged(HANDLE hContact, LPCSTR pszProto);
	virtual VOID	OnApply(HANDLE hContact, LPCSTR pszProto);
};

#endif /* _UINFOEX_CTRLANNIVEDIT_H_ */