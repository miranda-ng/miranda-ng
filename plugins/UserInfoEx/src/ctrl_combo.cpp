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

#include "stdafx.h"

/**
 * This static method creates an object for the CCombo class and returns its pointer.
 * If combobox is filled with items from pList.
 *
 * @param		hDlg			- HWND of the owning propertysheet page
 * @param		idCtrl			- the ID of the control to associate with this class's instance
 * @param		pszSetting		- the database setting to be handled by this class
 * @param		bDBDataType		- datatype of of the associated database setting (uint16_t, uint32_t, ...)
 * @param		pList			- pointer to a LPIDSTRLIST list, which holds the items to insert into the combobox.
 * @param		nListCount		- number of items in the list
 *
 * @return	pointer of the newly created CCombo object.
 **/
CBaseCtrl* CCombo::CreateObj(HWND hDlg, uint16_t idCtrl, LPCSTR pszSetting, uint8_t bDBDataType, LPIDSTRLIST pList, int nListCount)
{
	return new CCombo(hDlg, idCtrl, pszSetting, bDBDataType, pList, nListCount);
}

/**
 * This is the constructor. If combobox is filled with items from pList.
 *
 * @param		hDlg			- HWND of the owning propertysheet page
 * @param		idCtrl			- the ID of the control to associate with this class's instance
 * @param		pszSetting		- the database setting to be handled by this class
 * @param		bDBDataType		- datatype of of the associated database setting (uint16_t, uint32_t, ...)
 * @param		pList			- pointer to a LPIDSTRLIST list, which holds the items to insert into the combobox.
 * @param		nListCount		- number of items in the list
 *
 * @return	nothing
 **/
CCombo::CCombo(HWND hDlg, uint16_t idCtrl, LPCSTR pszSetting, uint8_t bDBDataType, LPIDSTRLIST pList, int nListCount)
	: CBaseCtrl(hDlg, idCtrl, pszSetting)
{
	_curSel = CB_ERR;
	_pList = pList;
	_nList = nListCount;
	_bDataType = bDBDataType;

	// fill in data
	if (_pList && (_nList > 0)) {
		for (int i = 0; i < _nList; i++) {
			AddItem(_pList[i].ptszTranslated, (LPARAM)&_pList[i]);
		}
	}
}

/**
 * This method searches for an item which matches the given nID.
 *
 * @param		nID				- the id of the desired object.
 *
 * @retval	CB_ERR	- item not found
 * @retval	0...n		- index of the combobox item
 **/
int CCombo::Find(int nID) const
{
	int i;
	LPIDSTRLIST pd;

	for (i = ComboBox_GetCount(_hwnd) - 1; i >= 0; i--) {
		pd = (LPIDSTRLIST)ComboBox_GetItemData(_hwnd, i);
		if (PtrIsValid(pd) && (pd->nID == nID)) {
			break;
		}
	}
	return i;
}

/**
 * This method searches for an item which matches the given label.
 *
 * @param		ptszItemLabel	- The translated label of the desired item.
 *
 * @retval	CB_ERR	- item not found
 * @retval	0...n		- index of the combobox item
 **/
int CCombo::Find(LPTSTR ptszItemLabel) const
{
	return ComboBox_FindStringExact(_hwnd, 0, ptszItemLabel);
}

/**
 * Adds a string and associated item data to a combobox.
 *
 * @param	 pszText			- the text to add to the combobox
 * @param	 lParam				- item data to accociate with the new item
 *
 * @return	zero-based index to new item or CB_ERR on failure
 **/
INT_PTR CCombo::AddItem(LPCTSTR pszText, LPARAM lParam)
{
	INT_PTR added = ComboBox_AddString(_hwnd, pszText);
	if (SUCCEEDED(added)) {
		if (PtrIsValid(lParam) && FAILED(ComboBox_SetItemData(_hwnd, added, lParam))) {
			ComboBox_DeleteString(_hwnd, added);
			added = CB_ERR;
		}
	}
	return added;
}

/**
 * This functions removes the user data from a combobox.
 *
 * @param		hCtrl			- HWND of the combobox
 *
 * @return	nothing
 **/
void CCombo::Release()
{
	delete this;
}

/**
 * This method selects the combobox item which matches the contact's setting, associated with.
 *
 * @param		hContact	- HANDLE of the contact
 * @param		pszProto	- the contact's protocol
 *
 * @return	nothing
 **/
BOOL CCombo::OnInfoChanged(MCONTACT hContact, LPCSTR pszProto)
{
	if (!_Flags.B.hasChanged && _pList != nullptr) {
		DBVARIANT dbv;
		int iVal = CB_ERR;

		_Flags.B.hasCustom = _Flags.B.hasProto = _Flags.B.hasMeta = false;
		_Flags.W |= DB::Setting::GetWStringCtrl(hContact, USERINFO, USERINFO, pszProto, _pszSetting, &dbv);
		EnableWindow(_hwnd, !hContact || _Flags.B.hasCustom || !g_plugin.bReadOnly);

		if (_Flags.B.hasCustom || _Flags.B.hasProto || _Flags.B.hasMeta) {
			switch (dbv.type) {
			case DBVT_BYTE: iVal = Find((int)dbv.bVal); break;
			case DBVT_WORD: iVal = Find((int)dbv.wVal); break;
			case DBVT_DWORD: iVal = Find((int)dbv.dVal); break;
			case DBVT_WCHAR:
				iVal = Find(TranslateW(dbv.pwszVal));
				if (iVal == CB_ERR) {
					// other
					iVal = Find(_pList[_nList - 1].nID);
				}
			}
		}
		if (iVal == CB_ERR) {
			// unspecified
			iVal = Find(_pList[0].nID);
		}
		db_free(&dbv);
		ComboBox_SetCurSel(_hwnd, iVal);
		_curSel = ComboBox_GetCurSel(_hwnd);
		SendMessage(GetParent(_hwnd), WM_COMMAND, MAKEWPARAM((uint16_t)this->_idCtrl, (uint16_t)CBN_SELCHANGE), (LPARAM)_hwnd);
	}
	return _Flags.B.hasChanged;
}

/**
 * This method writes the combobox's item
 *
 * @param		hContact		- HANDLE of the contact
 * @param		pszProto		- the contact's protocol
 *
 * @return	nothing
 **/
void CCombo::OnApply(MCONTACT hContact, LPCSTR pszProto)
{
	if (_Flags.B.hasChanged) {
		LPCSTR pszModule = hContact ? USERINFO : pszProto;

		if ((_Flags.B.hasCustom || !hContact) && (_curSel != CB_ERR)) {
			LPIDSTRLIST pd;

			pd = (LPIDSTRLIST)SendMessage(_hwnd, CB_GETITEMDATA, _curSel, 0);
			if (pd != nullptr) {
				switch (_bDataType) {
				case DBVT_BYTE:
					db_set_b(hContact, pszModule, _pszSetting, pd->nID);
					break;
				case DBVT_WORD:
					db_set_w(hContact, pszModule, _pszSetting, pd->nID);
					break;
				case DBVT_DWORD:
					db_set_dw(hContact, pszModule, _pszSetting, pd->nID);
					break;
				case DBVT_ASCIIZ:
				case DBVT_WCHAR:
					db_set_s(hContact, pszModule, _pszSetting, (LPSTR)pd->pszText);
				}
				if (!hContact) {
					_Flags.B.hasCustom = false;
					_Flags.B.hasProto = true;
				}
				_Flags.B.hasChanged = false;
			}
		}
		if (_Flags.B.hasChanged) {
			db_unset(hContact, pszModule, _pszSetting);
			_Flags.B.hasChanged = false;
			OnInfoChanged(hContact, pszProto);
		}
		InvalidateRect(_hwnd, nullptr, TRUE);
	}
}

/**
 * The user changed combobox selection, so mark it changed.
 *
 **/
void CCombo::OnChangedByUser(uint16_t wChangedMsg)
{
	if (wChangedMsg == CBN_SELCHANGE) {
		int c = ComboBox_GetCurSel(_hwnd);

		if (_curSel != c) {
			if (!_Flags.B.hasChanged) {
				_Flags.B.hasChanged = _Flags.B.hasCustom = true;
				SendMessage(GetParent(GetParent(_hwnd)), PSM_CHANGED, 0, 0);
			}
			_curSel = c;
		}
	}
}

