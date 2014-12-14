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

#include "commonheaders.h"

CBaseCtrl* CAnnivEditCtrl::CreateObj(HWND hDlg, WORD idCtrl, LPCSTR pszSetting)
{
	CAnnivEditCtrl *ctrl = NULL;

	ctrl = new CAnnivEditCtrl(hDlg, idCtrl, pszSetting);
	if (ctrl)
	{
	}
	return (ctrl);
}

CAnnivEditCtrl::CAnnivEditCtrl(HWND hDlg, WORD idCtrl, LPCSTR pszSetting)
	: CBaseCtrl(hDlg, idCtrl, pszSetting)
{
	_hwndDlg = hDlg;
	_hBtnAdd = GetDlgItem(hDlg, BTN_ADD);
	_hBtnDel = GetDlgItem(hDlg, BTN_DELETE);
	_hBtnEdit = GetDlgItem(hDlg, BTN_EDIT);
	_hBtnMenu = GetDlgItem(hDlg, BTN_MENU);
	_hwndDate = GetDlgItem(hDlg, EDIT_ANNIVERSARY_DATE);
	_ReminderEnabled = db_get_b(NULL, MODNAME, SET_REMIND_ENABLED, DEFVAL_REMIND_ENABLED);

	_pDates = NULL;
	_curDate = 0;
	_numDates = 0;
	
	// set button tooltips
	SendMessage(_hBtnAdd, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Add a new anniversary"), MBBF_TCHAR);
	SendMessage(_hBtnDel, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Delete an existing anniversary"), MBBF_TCHAR);

	// limit textinput
	SendDlgItemMessage(_hwndDlg, EDIT_REMIND, EM_LIMITTEXT, 2, 0);
	SendDlgItemMessage(_hwndDlg, SPIN_REMIND, UDM_SETRANGE32, 0, 50);

	// birthday is shown as an item in any case
	{
		MAnnivDate mdb;

		mdb.Id(ANID_BIRTHDAY);
		mdb.Description(TranslateT("Birthday"));
		AddDate(mdb);
	}
}

CAnnivEditCtrl::~CAnnivEditCtrl()
{
	WORD i;

	if (_pDates != NULL) 
	{
		for (i = 0; i < _numDates; i++) 
		{
			delete _pDates[i];
		}
		mir_free(_pDates);
		_pDates = NULL;
	}
}

void CAnnivEditCtrl::Release()
{
	delete this;
}


/**
 * name:	CAnnivEditCtrl
 * class:	ItemValid
 * desc:	tests whether the item pointed to by the wIndex is valid or not
 * param:	wIndex	- index to desired item
 * return:	TRUE if item is valid, FALSE otherwise
 **/
BYTE CAnnivEditCtrl::ItemValid(WORD wIndex) const
{
	return (_pDates != NULL && wIndex < _numDates && _pDates[wIndex] != NULL);
}

/**
 * name:	CAnnivEditCtrl
 * class:	CurrentItemValid
 * desc:	checks, whether currently selected item is valid
 * param:	none
 * return:	TRUE if item is valid, FALSE otherwise
 **/
BYTE CAnnivEditCtrl::CurrentItemValid() const
{
	return ItemValid(_curDate);
}

/**
 * name:	CAnnivEditCtrl
 * class:	EnableReminderCtrl
 * desc:	enables or disables reminder controls
 * param:	none
 * return:	TRUE if item is valid, FALSE otherwise
 **/
void CAnnivEditCtrl::EnableReminderCtrl(BYTE bEnabled)
{
	bEnabled &= _ReminderEnabled != REMIND_OFF;
	EnableWindow(GetDlgItem(_hwndDlg, RADIO_REMIND1), bEnabled);
	EnableWindow(GetDlgItem(_hwndDlg, RADIO_REMIND2), bEnabled);
	EnableWindow(GetDlgItem(_hwndDlg, RADIO_REMIND3), bEnabled);
	EnableWindow(GetDlgItem(_hwndDlg, EDIT_REMIND), bEnabled);
	EnableWindow(GetDlgItem(_hwndDlg, SPIN_REMIND), bEnabled);
	EnableWindow(GetDlgItem(_hwndDlg, TXT_REMIND), bEnabled);
}

/**
 * name:	EnableCurrentItem
 * class:	CAnnivEditCtrl
 * desc:	make control readonly if required
 * param:	none
 * return:	nothing
 **/
void CAnnivEditCtrl::EnableCurrentItem()
{
	MAnnivDate *pCurrent = Current();

	if (pCurrent) {
		MCONTACT hContact;
	
		PSGetContact(_hwndDlg, hContact);

		const BYTE bEnabled
			= !hContact ||
				(pCurrent->Flags() & CTRLF_HASCUSTOM) || 
				!(pCurrent->Flags() & (CTRLF_HASPROTO|CTRLF_HASMETA)) ||
				!db_get_b(NULL, MODNAME, SET_PROPSHEET_PCBIREADONLY, 0);

		EnableWindow(_hBtnEdit, bEnabled);
		EnableWindow(_hBtnDel, bEnabled);
		EnableWindow(_hwndDate, bEnabled);
	}
}

/**
 * name:	FindDateById
 * class:	CAnnivEditCtrl
 * desc:	returns an iterator to an item with the given id
 * param:	wId		- id the returned item must have
 * return:	if an date with the wId was found - iterator to item,
 *			NULL otherwise
 **/
MAnnivDate* CAnnivEditCtrl::FindDateById(const WORD wId)
{
	WORD i;

	if (_pDates != NULL) {
		for (i = 0; i < _numDates; i++) {
			if (_pDates[i]->Id() < ANID_NONE && _pDates[i]->Id() == wId) {
				return _pDates[i];
			}
		}
	}
	return NULL;
}

/**
 * name:	AddDate
 * class:	CAnnivEditCtrl
 * desc:	Add a new item to the array of dates
 * param:	mda		- the date to add
 * return:	0 on success, -1 on failure, 1 if the item to change was edited before and the new item was not set
 **/
INT_PTR CAnnivEditCtrl::AddDate(MAnnivDate &mda)
{
	MAnnivDate *pmda, **pmd;

	// if a date with wID exists, replace it
	if ((pmda = FindDateById(mda.Id())) != NULL) {
		BYTE bChanged = pmda->IsChanged(),
			bRemindChanged = pmda->IsReminderChanged();

		if (!bChanged) {
			pmda->Set(mda);
			pmda->Module(mda.Module());
			pmda->Description(mda.Description());
			pmda->Flags(mda.Flags());
		}
		if (!bRemindChanged) {
			pmda->RemindOption(mda.RemindOption());
			pmda->RemindOffset(mda.RemindOffset());
		}
		return bChanged || bRemindChanged;
	}
	if (mda.Id() == ANID_NONE)
		mda.Id(_numDates - 1);

	if (pmd = (MAnnivDate **)mir_realloc(_pDates, (_numDates + 1) * sizeof(pmda))) {
		_pDates = pmd;
		if (_pDates[_numDates] = new MAnnivDate(mda)) {
			_numDates++;
			return 0;
		}
	}
	return 1;
}

/**
 * name:	DeleteDate
 * class:	CAnnivEditCtrl
 * desc:	Delete the item on the position identified by wIndex
 * param:	pDateCtrl	- pointer to the date control's data structure
 *			wIndex		- index of the item to delete
 * return:	0 on success 1 otherwise
 **/
INT_PTR CAnnivEditCtrl::DeleteDate(WORD wIndex)
{
	if (!ItemValid(wIndex)) return 1;
	
	// only delete values, but not the item
	if (_pDates[wIndex]->Id() == ANID_BIRTHDAY) {
		MCONTACT hContact;
		LPCSTR pszProto;

		PSGetContact(_hwndDlg, hContact);
		PSGetBaseProto(_hwndDlg, pszProto);

		// protocol value exists?
		if (_pDates[wIndex]->DBGetDate(hContact, pszProto, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR)) {
			_pDates[wIndex]->SetFlags(MAnnivDate::MADF_HASPROTO);
		}
		else {
			_pDates[wIndex]->ZeroDate();
		}

		_pDates[wIndex]->RemindOption(BST_INDETERMINATE);
		_pDates[wIndex]->RemindOffset((BYTE)-1);

		_pDates[wIndex]->RemoveFlags(MAnnivDate::MADF_HASCUSTOM);
		_pDates[wIndex]->SetFlags(MAnnivDate::MADF_CHANGED|MAnnivDate::MADF_REMINDER_CHANGED);
	}
	else {
		delete _pDates[wIndex];
		_numDates--;
		if (wIndex < _numDates)
			memmove(_pDates + wIndex, _pDates + wIndex + 1, (_numDates - wIndex) * sizeof(*_pDates));
		memset((_pDates + _numDates), 0, sizeof(*_pDates)); // XXX: check me: sizeof(*_pDates) -> (sizeof(*_pDates) - _numDates)
		if (_curDate >= _numDates)
			_curDate = _numDates - 1;
	}
	SendMessage(GetParent(_hwndDlg), PSM_CHANGED, NULL, NULL);
	SetCurSel(_curDate);
	return 0;
}

/**
 * name:	DateCtrl_DBGetBirthDay
 * desc:
 * param:
 * return:	0 on success 1 otherwise
 **/
INT_PTR CAnnivEditCtrl::DBGetBirthDay(MCONTACT hContact, LPCSTR pszProto)
{
	MAnnivDate mdb;

	if (!mdb.DBGetBirthDate(hContact, (char *)pszProto)) {
		mdb.DBGetReminderOpts(hContact);
		return AddDate(mdb) > 0;
	}
	return 0;
}

/**
 * name:	DateCtrl_DBGetBirthDay
 * desc:
 * param:
 * return:	0 on success 1 otherwise
 **/
INT_PTR CAnnivEditCtrl::DBGetAnniversaries(MCONTACT hContact)
{
	MAnnivDate mda;

	WORD i;
	BYTE bChanged = FALSE;

	for (i = 0; i < ANID_LAST && !mda.DBGetAnniversaryDate(hContact, i); i++) {
		mda.DBGetReminderOpts(hContact);
		switch (AddDate(mda)) {
			case -1:
				return bChanged;
			case 1:
				bChanged |= 1;
				break;
		}
	}
	return bChanged;
}

/**
 * name:	DBWriteBirthDay
 * class:	CAnnivEditCtrl
 * desc:	writes the birthday for a contact to database
 * param:	hContact - the contact to write the anniversaries to
 * return:	0 on success 1 otherwise
 **/
INT_PTR CAnnivEditCtrl::DBWriteBirthDay(MCONTACT hContact)
{
	MAnnivDate *pmdb;

	if ((pmdb = FindDateById(ANID_BIRTHDAY)) == NULL)
		return 1;
		
	if (pmdb->IsChanged()) {
		// save birthday, to mBirthday module by default
		if (pmdb->Flags() & pmdb->MADF_HASCUSTOM)
			pmdb->DBWriteBirthDate(hContact);
		else
			pmdb->DBDeleteBirthDate(hContact);
	}

	if (pmdb->IsReminderChanged()) {
		pmdb->DBWriteReminderOpts(hContact);
	}
	pmdb->RemoveFlags(MAnnivDate::MADF_CHANGED|MAnnivDate::MADF_REMINDER_CHANGED);
	return 0;
}

/**
 * name:	DBWriteAnniversaries
 * class:	CAnnivEditCtrl
 * desc:	write all anniversaries to the database
 * param:	hContact - the contact to write the anniversaries to
 * return:	0 on success 1 otherwise
 **/
INT_PTR CAnnivEditCtrl::DBWriteAnniversaries(MCONTACT hContact)
{
	const LPCSTR szPrefix[] = { "Reminder", "Offset", "Desc", "Day", "Month", "Year", "Stamp", "Date" };
	CHAR szSet0[MAXSETTING];
	WORD i, ret, ofs, wIndex = 0;

	for (i = 0; i < _numDates; i++) {
		if (
			_pDates[i] != NULL &&
			!_pDates[i]->DBWriteAnniversaryDate(hContact, wIndex) &&
			!_pDates[i]->DBWriteReminderOpts(hContact)
	)
		{
			wIndex++;
		}
	}
	// delete reluctant items
	do {
		ofs = mir_snprintf(szSet0, SIZEOF(szSet0), "Anniv%d", wIndex);
		ret = 1;
		for (i = 0; i < SIZEOF(szPrefix); i++) {
			mir_strncpy(szSet0 + ofs, szPrefix[i], SIZEOF(szSet0) - ofs);
			ret &= db_unset(hContact, USERINFO, szSet0);
		}
	}
	while (wIndex++ <= ANID_LAST && !ret);
	return 0;
}

/**
 * name:	SetCurSel
 * class:	CAnnivEditCtrl
 * desc:	shows the item, identified by wIndex
 * param:	pDateCtrl	- pointer to the date control's data structure
 *			wIndex		- index of the item to delete
 * return:	0 on success 1 otherwise
 **/
INT_PTR CAnnivEditCtrl::SetCurSel(WORD wIndex)
{
	BYTE bEnabled = ItemValid(wIndex);

	EnableWindow(_hwndDate, bEnabled);
	EnableWindow(_hBtnEdit, bEnabled);
	EnableWindow(_hBtnDel, bEnabled && _pDates[wIndex]->IsValid());
	if (!bEnabled) {
		EnableReminderCtrl(FALSE);
		return 1;
	}
	_curDate = wIndex;

	// set date of date control
	if (_pDates[wIndex]->IsValid()) {
		SYSTEMTIME st = _pDates[wIndex]->SystemTime();
		DateTime_SetSystemtime(_hwndDate, GDT_VALID, &st);
		DateTime_SetFormat(_hwndDate, NULL);
	}
	else {
		TCHAR szText[MAX_DESC];
		mir_sntprintf(szText, SIZEOF(szText), _T("'%s'"), TranslateT("Unspecified"));
		DateTime_SetSystemtime(_hwndDate, GDT_NONE, NULL);
		DateTime_SetFormat(_hwndDate, szText);
	}
	// set edit button's caption
	SetWindowText(_hBtnEdit, _pDates[wIndex]->Description());

	// set reminder options
	CheckDlgButton(_hwndDlg, RADIO_REMIND1, _pDates[wIndex]->RemindOption() == BST_INDETERMINATE ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(_hwndDlg, RADIO_REMIND2, _pDates[wIndex]->RemindOption() == BST_CHECKED ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(_hwndDlg, RADIO_REMIND3, _pDates[wIndex]->RemindOption() == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);

	OnReminderChecked();
	EnableCurrentItem();
	return 0;
}

/**
 * name:	OnMenuPopup
 * class:	CAnnivEditCtrl
 * desc:	is called to show a popup menu for all anniversaries of a contact
 * param:	none
 * return:	nothing
 **/
void CAnnivEditCtrl::OnMenuPopup()
{
	POINT pt = { 0, 0 };
	RECT rc;
	MENUITEMINFO mii;
	HMENU hMenu;
	WORD i;

	if (hMenu = CreatePopupMenu()) {
		SetFocus(_hBtnMenu);

		memset(&mii, 0, sizeof(MENUITEMINFO));
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_ID|MIIM_STRING|MIIM_STATE;

		// insert the items
		for (i = 0; i < _numDates; i++) {
			mii.fState = _pDates[i]->IsValid() ? MFS_CHECKED : MFS_UNCHECKED;
			mii.dwTypeData = (LPTSTR)_pDates[i]->Description();
			mii.wID = WM_USER + i;
			if (!InsertMenuItem(hMenu, i, TRUE, &mii)) {
				DestroyMenu(hMenu);
				return;
			}
		}
		ClientToScreen(_hBtnMenu, &pt);
		GetClientRect(_hBtnMenu, &rc);
		i = TrackPopupMenuEx(hMenu, TPM_RIGHTALIGN|TPM_RETURNCMD, pt.x + rc.right, pt.y + rc.bottom, _hwndDlg, NULL);
		DestroyMenu(hMenu);
		SendMessage(_hBtnMenu, BM_SETCHECK, NULL, NULL);
		if (i >= WM_USER) SetCurSel(i - WM_USER);
	}
}

/**
 * name:	OnMenuPopup
 * class:	CAnnivEditCtrl
 * desc:	is called to show a popup menu for all anniversaries of a contact
 * param:	none
 * return:	nothing
 **/
void CAnnivEditCtrl::OnDateChanged(LPNMDATETIMECHANGE lpChange)
{
	MAnnivDate *pCurrent = Current();

	if (pCurrent && !pCurrent->IsEqual(lpChange->st))
	{
		HWND hPs = GetParent(_hwndDlg);

		// save the new date to the structure
		DateTime_SetFormat(_hwndDate, NULL);
		pCurrent->Set(lpChange->st, TRUE);
		pCurrent->SetFlags(MAnnivDate::MADF_CHANGED|MAnnivDate::MADF_HASCUSTOM);
		
		// notify parent of the change
		SendMessage(hPs, PSM_CHANGED, NULL, NULL);
		EnableWindow(_hBtnDel, TRUE);

		// update the age and zodiac controls on the general propertysheetpage
		if (pCurrent->Id() == ANID_BIRTHDAY) 
		{
			SetZodiacAndAge(pCurrent);
		}
	}
}

/**
 * name:	OnRemindEditChanged
 * class:	CAnnivEditCtrl
 * desc:	is called, if reminder edit control was changed
 * param:	none
 * return:	nothing
 **/
void CAnnivEditCtrl::OnRemindEditChanged()
{
	MAnnivDate	*pCurrent = Current();

	if (pCurrent) 
	{
		UINT iVal = GetDlgItemInt(_hwndDlg, EDIT_REMIND, NULL, FALSE);
		if (iVal != pCurrent->RemindOffset() && IsDlgButtonChecked(_hwndDlg, RADIO_REMIND2) == BST_CHECKED) 
		{
			SendMessage(GetParent(_hwndDlg), PSM_CHANGED, NULL, NULL);
			pCurrent->SetFlags(MAnnivDate::MADF_REMINDER_CHANGED);
			pCurrent->RemindOffset(iVal);
		}
	}
}

/**
 * name:	OnReminderChecked
 * class:	CAnnivEditCtrl
 * desc:	is called if reminder checkbox's state was changed
 * param:	none
 * return:	nothing
 **/
void CAnnivEditCtrl::OnReminderChecked()
{
	MCONTACT hContact;
	LPCSTR pszProto;
	int state;
	TCHAR buf[6];
	MAnnivDate *pCurrent = Current();

	PSGetContact(_hwndDlg, hContact);
	if (!hContact || !PSGetBaseProto(_hwndDlg, pszProto) || !pCurrent) 
	{
		EnableReminderCtrl(FALSE);
	}
	else
	{
		if (IsDlgButtonChecked(_hwndDlg, RADIO_REMIND1))
		{
			_itot(db_get_b(NULL, MODNAME, SET_REMIND_OFFSET, DEFVAL_REMIND_OFFSET), buf, 10);
			EnableWindow(GetDlgItem(_hwndDlg, EDIT_REMIND), FALSE);
			EnableWindow(GetDlgItem(_hwndDlg, SPIN_REMIND), FALSE);
			state = BST_INDETERMINATE;
		}
		else if (IsDlgButtonChecked(_hwndDlg, RADIO_REMIND2))
		{
			if (pCurrent->RemindOffset() == (BYTE)-1)
			{
				_itot(db_get_b(NULL, MODNAME, SET_REMIND_OFFSET, DEFVAL_REMIND_OFFSET), buf, 10);
			}
			else
			{
				_itot(pCurrent->RemindOffset(), buf, 10);
			}
			EnableWindow(GetDlgItem(_hwndDlg, EDIT_REMIND), _ReminderEnabled);
			EnableWindow(GetDlgItem(_hwndDlg, SPIN_REMIND), _ReminderEnabled);
			state = BST_CHECKED;
		}
		else
		{
			*buf = 0;
			EnableWindow(GetDlgItem(_hwndDlg, EDIT_REMIND), FALSE);
			EnableWindow(GetDlgItem(_hwndDlg, SPIN_REMIND), FALSE);
			state = BST_UNCHECKED;
		}
		if (pCurrent->RemindOption() != state) 
		{
			pCurrent->RemindOption(state);
			if (!PspIsLocked(_hwndDlg)) 
			{
				pCurrent->SetFlags(MAnnivDate::MADF_REMINDER_CHANGED);
				SendMessage(GetParent(_hwndDlg), PSM_CHANGED, NULL, NULL);
			}
		}
		SetDlgItemText(_hwndDlg, EDIT_REMIND, buf);
	}
}

void CAnnivEditCtrl::SetZodiacAndAge(MAnnivDate *mt)
{
	if (PtrIsValid(mt))
	{
		int age;
		MZodiac zod;

		zod = mt->Zodiac();
		if (zod.pszName != NULL)
		{
			ShowWindow(GetDlgItem(_hwndDlg, TEXT_ZODIAC), SW_SHOW);
			SetDlgItemText(_hwndDlg, TEXT_ZODIAC, TranslateTS(zod.pszName));
			SendDlgItemMessage(_hwndDlg, IDC_ZODIAC, STM_SETIMAGE, IMAGE_ICON, (LPARAM)zod.hIcon);
		}
		else
		{
			ShowWindow(GetDlgItem(_hwndDlg, IDC_ZODIAC), SW_HIDE);
			ShowWindow(GetDlgItem(_hwndDlg, TEXT_ZODIAC), SW_HIDE);
		}
		if ((age = mt->Age()) > 0)
		{
			SetDlgItemInt(_hwndDlg, EDIT_AGE, age , FALSE);
		}
	}
}

BOOL CAnnivEditCtrl::OnInfoChanged(MCONTACT hContact, LPCSTR pszProto)
{
	BOOL bChanged;
	bChanged = DBGetBirthDay(hContact, pszProto);
	bChanged |= DBGetAnniversaries(hContact);
	SetCurSel(0);
	SetZodiacAndAge(_pDates[0]);
	return bChanged;
}

void CAnnivEditCtrl::OnApply(MCONTACT hContact, LPCSTR pszProto)
{
	DBWriteBirthDay(hContact);
	DBWriteAnniversaries(hContact);
}
