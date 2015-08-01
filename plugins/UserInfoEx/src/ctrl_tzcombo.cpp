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
*/

#include "stdafx.h"

static INT_PTR EnumNamesProc(CTimeZone *pTimeZone, int index, LPARAM lParam)
{
	if (pTimeZone && pTimeZone->ptszDisplay)
	{
		int added = ComboBox_AddString((HWND)lParam, pTimeZone->ptszDisplay);
		if (SUCCEEDED(added)) 
		{
			if (FAILED(ComboBox_SetItemData((HWND)lParam, added, pTimeZone))) 
			{
				ComboBox_DeleteString((HWND)lParam, added);
			}
		}
	}
	return 0;
}

/**
 * This functions fills a combobox given by @hCtrl with
 * all items of the global timezone manager
 *
 * @param	hDlg			- HWND of the owning propertysheet page
 * @param	idCtrl			- the ID of the control to associate with this class's instance
 * @param	pszSetting		- the database setting to be handled by this class
 *
 * @return	CTzCombo*
 **/
CBaseCtrl* CTzCombo::CreateObj(HWND hDlg, WORD idCtrl, LPCSTR pszSetting)
{
	HWND hCtrl = GetDlgItem(hDlg, idCtrl);

	CTzCombo *ctrl = new CTzCombo(hDlg, idCtrl, pszSetting);
	if (ctrl) {
		//set the adress of our timezone handle as itemdata
		//caller can obtain the handle htz to extract all relevant information
		ctrl->_curSel = 0;
		TimeZone_PrepareList(NULL, NULL, hCtrl, TZF_PLF_CB);
	}
	return (ctrl);
}

/**
 *
 *
 **/
CTzCombo::CTzCombo() : CBaseCtrl()
{
	_curSel = CB_ERR;
}

/**
 *
 *
 **/
CTzCombo::CTzCombo(HWND hDlg, WORD idCtrl, LPCSTR pszSetting)
	: CBaseCtrl(hDlg, idCtrl, pszSetting)
{
	_curSel = CB_ERR;
}

/**
 * This method does a binary search in the sorted
 * ComboBox for the item index. (old UIEX method)
 *
 * @param	pTimeZone		- CTimeZone compobox item data.
 *
 * @retval	CB_ERR			- item not found
 * @retval	0...n			- index of the combobox item
 **/
int CTzCombo::Find(CTimeZone *pTimeZone) const
{
	int nItemIndex;
	int nItemCount = ComboBox_GetCount(_hwnd);

	for (nItemIndex = 0; nItemIndex < nItemCount; nItemIndex++)
	{
		if (pTimeZone == (CTimeZone *)ComboBox_GetItemData(_hwnd, nItemIndex))
			return nItemIndex;
	}
	return CB_ERR;
}

/**
 * This method does a binary search in the sorted
 * ComboBox for the item index. (new core tz interface)
 *
 * @param	pTimeZone		- LPTIME_ZONE_INFORMATION to find.
 *
 * @retval	CB_ERR			- item not found
 * @retval	0...n			- index of the combobox item
 **/
int CTzCombo::Find(LPTIME_ZONE_INFORMATION pTimeZone) const
{
	int nItemIndex;
	int nItemCount = ComboBox_GetCount(_hwnd);

	for (nItemIndex = 0; nItemIndex < nItemCount; nItemIndex++) {
		if (pTimeZone == TimeZone_GetInfo((HANDLE)ComboBox_GetItemData(_hwnd, nItemIndex)))
			return nItemIndex;
	}
	return CB_ERR;
}

/**
 * This functions removes the user data from a combobox.
 *
 * @param	hCtrl	-	HWND of the combobox
 *
 * @return	nothing
 **/
void CTzCombo::Release()
{
	delete this;
}

/**
 * This functions selects the combobox item which matches the contact's timezone.
 *
 * @param	hCtrl		- HWND of the combobox
 * @param	hContact	- HANDLE of the contact whose timezone to select
 * @param	pszProto	- the contact's protocol (not used by new core tz interface)
 *
 * @return	_Flags.B.hasChanged member
 **/
BOOL CTzCombo::OnInfoChanged(MCONTACT hContact, LPCSTR pszProto)
{
	if (!_Flags.B.hasChanged) {
		LPTIME_ZONE_INFORMATION pTimeZone;
		pTimeZone = getTziByContact(hContact);
		ComboBox_SetCurSel(_hwnd, Find(pTimeZone));
		_curSel = ComboBox_GetCurSel(_hwnd);
		SendMessage(GetParent(_hwnd), WM_TIMER, 0, 0);
	}
	return _Flags.B.hasChanged;
}

/**
 * This method writes the combobox's item as the contact's timezone.
 *
 * @param		hContact	- HANDLE of the contact whose timezone to select
 * @param		pszProto	- the contact's protocol (not used by new core tz interface)
 *
 * @return	nothing
 **/
void CTzCombo::OnApply(MCONTACT hContact, LPCSTR pszProto)
{
	if (_Flags.B.hasChanged)
	{
		const char* pszModule = hContact ? USERINFO : pszProto;
		if (_Flags.B.hasCustom || !hContact) {
			//use new core tz interface
			TimeZone_StoreListResult(hContact, NULL, _hwnd, TZF_PLF_CB);
			if (!hContact) {
				_Flags.B.hasCustom = 0;
				_Flags.B.hasProto = 1;
			}
			_Flags.B.hasChanged = 0;
		}

		if (_Flags.B.hasChanged)
		{
			db_unset(hContact, USERINFO, SET_CONTACT_TIMEZONENAME);
			db_unset(hContact, USERINFO, SET_CONTACT_TIMEZONEINDEX);
			db_unset(hContact, pszModule, SET_CONTACT_TIMEZONE);

			_Flags.B.hasChanged = 0;
			OnInfoChanged(hContact, pszProto);
		}
		InvalidateRect(_hwnd, NULL, TRUE);
	}
}

/**
 * The user changed combobox selection, so mark it changed.
 *
 * @return	nothing
 **/
void CTzCombo::OnChangedByUser(WORD wChangedMsg)
{
	if (wChangedMsg == CBN_SELCHANGE) {
		int c = ComboBox_GetCurSel(_hwnd);

		if (_curSel != c) {
			if (!_Flags.B.hasChanged) {
				_Flags.B.hasChanged = 1;
				_Flags.B.hasCustom = 1;
				SendMessage(GetParent(GetParent(_hwnd)), PSM_CHANGED, 0, 0);
			}
			_curSel = c;
			SendMessage(GetParent(_hwnd), WM_TIMER, 0, 0);
		}
	}
}

/**
 * This method fills @szTime with the current time
 * according to the combobox's selected timezone.
 *
 * @param		szTime		- string to fill with the current time
 * @param		cchTime		- number of characters the string can take
 *
 * @return	nothing
 **/
void CTzCombo::GetTime(LPTSTR szTime, int cchTime)
{
	TimeZone_PrintDateTime((HANDLE)ComboBox_GetItemData(_hwnd, _curSel), _T("t"), szTime, cchTime, 0);
}
