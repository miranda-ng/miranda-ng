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

#define MAX_DESC	50

class CAnnivEditCtrl : public CBaseCtrl
{
	HWND	_hwndDlg;	 // owning dialog box
	HWND	_hwndDate;	// date picker
	HWND	_hBtnMenu;	// anniversary dropdown button
	HWND	_hBtnEdit;	// edit anniversary button
	HWND	_hBtnAdd;	 // add anniversary button
	HWND	_hBtnDel;	 // delete anniversary button

	uint8_t	_ReminderEnabled;

	MAnnivDate** _pDates;
	uint16_t	_numDates;
	uint16_t	_curDate;

	uint8_t ItemValid(uint16_t wIndex) const;
	uint8_t CurrentItemValid() const;

	INT_PTR DBGetBirthDay(MCONTACT hContact, LPCSTR pszProto);
	INT_PTR DBWriteBirthDay(MCONTACT hContact);

	INT_PTR DBGetAnniversaries(MCONTACT hContact);
	INT_PTR DBWriteAnniversaries(MCONTACT hContact);

	CAnnivEditCtrl(HWND hDlg, uint16_t idCtrl, LPCSTR pszSetting);
	~CAnnivEditCtrl();

public:

	MAnnivDate* Current() { return CurrentItemValid() ? _pDates[_curDate] : nullptr; };
	uint16_t				CurrentIndex() const { return _curDate; };
	uint16_t				NumDates() const { return _numDates; };
	uint8_t		 ReminderEnabled() const { return _ReminderEnabled; };

	MAnnivDate* FindDateById(const uint16_t wId);

	void				EnableCurrentItem();
	void				EnableReminderCtrl(uint8_t bEnabled);

	INT_PTR		 SetCurSel(uint16_t wIndex);

	INT_PTR		 AddDate(MAnnivDate &mda);
	INT_PTR		 DeleteDate(uint16_t wIndex);

	void				SetZodiacAndAge(MAnnivDate *mt);

	// notification handlers
	void OnMenuPopup();
	void OnDateChanged(LPNMDATETIMECHANGE lpChange);
	void OnRemindEditChanged();
	void OnReminderChecked();

	/**
	 * CBaseCtrl interface:
	 **/
	static __forceinline CAnnivEditCtrl* GetObj(HWND hCtrl) 
		{ return (CAnnivEditCtrl*) GetUserData(hCtrl); }
	static __forceinline CAnnivEditCtrl* GetObj(HWND hDlg, uint16_t idCtrl)
		{ return GetObj(GetDlgItem(hDlg, idCtrl)); }

	static CBaseCtrl* CreateObj(HWND hDlg, uint16_t idCtrl, LPCSTR pszSetting);

	virtual void	Release();
	virtual BOOL	OnInfoChanged(MCONTACT hContact, LPCSTR pszProto);
	virtual void	OnApply(MCONTACT hContact, LPCSTR pszProto);
};

#endif /* _UINFOEX_CTRLANNIVEDIT_H_ */