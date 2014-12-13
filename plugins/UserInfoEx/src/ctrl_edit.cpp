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

/**
 * This function creates a CEditCtrl object. 
 *
 * @param	hDlg			- HWND of the owning propertysheet page
 * @param	idCtrl			- the ID of the dialog control to associate with this class's instance
 * @param	pszSetting		- the database setting to be handled by this class
 * @param	dbType			- the expected data type of the setting
 *
 * @return	This static method returns the pointer of the created object.
 **/
CBaseCtrl* CEditCtrl::CreateObj(HWND hDlg, WORD idCtrl, LPCSTR pszSetting, BYTE dbType)
{
	CEditCtrl *ctrl = NULL;

	ctrl = new CEditCtrl(hDlg, idCtrl, USERINFO, pszSetting);
	if (ctrl) {
		ctrl->_dbType = dbType;
	}
	return (ctrl);
}

/**
 * This function creates a CEditCtrl object. 
 *
 * @param	hDlg			- HWND of the owning propertysheet page
 * @param	idCtrl			- the ID of the dialog control to associate with this class's instance
 * @param	pszModule		- the database module to be handled by this class
 * @param	pszSetting		- the database setting to be handled by this class
 * @param	dbType			- the expected data type of the setting
 *
 * @return	This static method returns the pointer of the created object.
 **/
CBaseCtrl* CEditCtrl::CreateObj(HWND hDlg, WORD idCtrl, LPCSTR pszModule, LPCSTR pszSetting, BYTE dbType)
{
	CEditCtrl *ctrl = NULL;

	ctrl = new CEditCtrl(hDlg, idCtrl, pszModule, pszSetting);
	if (ctrl) {
		ctrl->_dbType = dbType;
	}
	return (ctrl);
}

/**
 *
 *
 **/
CEditCtrl::CEditCtrl(HWND hDlg, WORD idCtrl, LPCSTR pszModule, LPCSTR pszSetting)
	: CBaseCtrl(hDlg, idCtrl, pszModule, pszSetting)
{
	SendDlgItemMessage(hDlg, idCtrl, EM_LIMITTEXT, 0x7fFFffFF, 0L);
}

/**
 * This method deletes the class object
 * and all allocated memory of its members.
 *
 * @param	none
 *
 * @return	nothing
 **/
void CEditCtrl::Release()
{
	delete this;
}

/*
 * 
 *
 */
void CEditCtrl::OnReset()
{
}


/**
 * This method controls the changed bit for the control.
 *
 * @param	hCtrl			- HWND of the combobox
 * @param	hContact		- HANDLE of the contact whose timezone to select
 * @param	pszProto		- the contact's protocol
 *
 * @return	nothing
 **/
BOOL CEditCtrl::OnInfoChanged(MCONTACT hContact, LPCSTR pszProto)
{
	if (!_Flags.B.hasChanged)
	{
		DBVARIANT dbv;
		TCHAR szText[64];

		_Flags.B.hasCustom = _Flags.B.hasProto = _Flags.B.hasMeta = 0;
		_Flags.W |= DB::Setting::GetTStringCtrl(hContact, _pszModule, _pszModule, pszProto, _pszSetting, &dbv);
	
		EnableWindow(_hwnd, 
			!hContact || _Flags.B.hasCustom || !db_get_b(NULL, MODNAME, SET_PROPSHEET_PCBIREADONLY, 0));	

		MIR_FREE(_pszValue);
		switch (dbv.type) 
		{
		case DBVT_BYTE:
			_itot_s(dbv.bVal, szText, SIZEOF(szText), 10);
			SetWindowText(_hwnd, szText);
			_pszValue = mir_tstrdup(szText);
			break;

		case DBVT_WORD:
			_itot_s(dbv.wVal, szText, SIZEOF(szText), 10);
			SetWindowText(_hwnd, szText);
			_pszValue = mir_tstrdup(szText);
			break;

		case DBVT_DWORD:
			_itot_s(dbv.dVal, szText, SIZEOF(szText), 10);
			SetWindowText(_hwnd, szText);
			_pszValue = mir_tstrdup(szText);
			break;

		case DBVT_TCHAR:
			if (dbv.ptszVal)
			{
				SetWindowText(_hwnd, dbv.ptszVal);
				_pszValue = dbv.ptszVal;
				break;
			}
		
		default:
			SetWindowText(_hwnd, _T(""));
			db_free(&dbv);
			break;
		}
		_Flags.B.hasChanged = 0;
	}
	return _Flags.B.hasChanged;
}

/**
 * This method writes the control's information to database
 *
 * @param	hContact		- HANDLE of the contact whose timezone to select
 * @param	pszProto		- the contact's protocol
 *
 * @return	nothing
 **/
void CEditCtrl::OnApply(MCONTACT hContact, LPCSTR pszProto)
{
	if (_Flags.B.hasChanged)
	{
		const char* pszModule = hContact ? _pszModule : pszProto;

		if (_Flags.B.hasCustom || !hContact)
		{
			DWORD cch = GetWindowTextLength(_hwnd);

			if (cch > 0)
			{
				LPTSTR val = (LPTSTR)mir_alloc((cch + 1) * sizeof(TCHAR));

				if (GetWindowText(_hwnd, val, cch + 1) > 0) 
				{
					DBVARIANT dbv;

					dbv.type = _dbType;
					switch (_dbType) 
					{
					case DBVT_BYTE:
						dbv.bVal = (BYTE)_tcstol(val, NULL, 10);
						break;

					case DBVT_WORD:
						dbv.wVal = (WORD)_tcstol(val, NULL, 10);
						break;

					case DBVT_DWORD:
						dbv.dVal = (DWORD)_tcstol(val, NULL, 10);
						break;

					case DBVT_TCHAR:
						dbv.ptszVal = val;
						break;

					default:
						dbv.type = DBVT_DELETED;

					}
					if (dbv.type != DBVT_DELETED)
					{
						if (!db_set(hContact, pszModule, _pszSetting, &dbv))
						{
							if (!hContact)
							{
								_Flags.B.hasCustom = 0;
								_Flags.B.hasProto = 1;
							}
							_Flags.B.hasChanged = 0;
							
							// save new value
							MIR_FREE(_pszValue);
							_pszValue = val;
							val = NULL;
						}
					}
				}
				MIR_FREE(val);
			}
		}
		if (_Flags.B.hasChanged)
		{
			db_unset(hContact, pszModule, _pszSetting);

			_Flags.B.hasChanged = 0;

			OnInfoChanged(hContact, pszProto);
		}			
		InvalidateRect(_hwnd, NULL, TRUE);
	}
}

/**
 * The user changed information stored in the control.
 *
 * @return	nothing
 **/
void CEditCtrl::OnChangedByUser(WORD wChangedMsg)
{
	if ((wChangedMsg == EN_UPDATE) || (wChangedMsg == EN_CHANGE))
	{
		const int	cch = GetWindowTextLength(_hwnd);

		_Flags.B.hasChanged = mir_tstrlen(_pszValue) != cch;
		_Flags.B.hasCustom = (cch > 0);

		if (!_Flags.B.hasChanged && _Flags.B.hasCustom) {
			BYTE		need_free = 0;
			LPTSTR		szText;
		
			__try 
			{
				szText = (LPTSTR)alloca((cch + 1) * sizeof(TCHAR));
			}
			__except(EXCEPTION_EXECUTE_HANDLER) 
			{
				szText = (LPTSTR)mir_alloc((cch + 1) * sizeof(TCHAR));
				need_free = 1;
			}

			if (szText != NULL) {
				GetWindowText(_hwnd, szText, cch + 1);
				_Flags.B.hasChanged = mir_tstrcmp(_pszValue, szText);
				if (need_free)
					MIR_FREE(szText);
			} else {
				_Flags.B.hasChanged = 0;
			}
		}
		InvalidateRect(_hwnd, NULL, TRUE);

		if (_Flags.B.hasChanged)
			SendMessage(GetParent(GetParent(_hwnd)), PSM_CHANGED, 0, 0);
	}
}

/**
 * Opens the url given in a editbox in the users default browser
 **/
void CEditCtrl::OpenUrl()
{
	int lenUrl = 1 + Edit_GetTextLength(_hwnd);
	LPSTR szUrl;
	BYTE need_free = 0;

	__try 
	{
		szUrl = (LPSTR)alloca((8 + lenUrl) * sizeof(CHAR));
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		szUrl = (LPSTR)mir_alloc((8 + lenUrl) * sizeof(CHAR));
		need_free = 1;
	}
	if (szUrl && (GetWindowTextA(_hwnd, szUrl, lenUrl) > 0))
	{
		CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)szUrl);
	}
	if (need_free)
	{
		MIR_FREE(szUrl);
	}
}

LRESULT CEditCtrl::LinkNotificationHandler(ENLINK* lnk)
{
	switch (lnk->msg)
	{
	case WM_SETCURSOR:
		{
			SetCursor(LoadCursor(NULL, IDC_HAND));
			SetWindowLongPtr(GetParent(_hwnd), DWLP_MSGRESULT, TRUE);
		}
		return TRUE;

	case WM_LBUTTONUP: 
		{
			if (lnk)
			{
				TEXTRANGE tr;
				BYTE need_free = 0;

				// do not call function if user selected some chars of the url string
				SendMessage(_hwnd, EM_EXGETSEL, NULL, (LPARAM) &tr.chrg);
				if (tr.chrg.cpMax == tr.chrg.cpMin)
				{
					// retrieve the url string
					tr.chrg = lnk->chrg;

					__try 
					{
						tr.lpstrText = (LPTSTR)alloca((tr.chrg.cpMax - tr.chrg.cpMin + 8) * sizeof(TCHAR));
					}
					__except(EXCEPTION_EXECUTE_HANDLER) 
					{
						tr.lpstrText = (LPTSTR)mir_alloc((tr.chrg.cpMax - tr.chrg.cpMin + 8) * sizeof(TCHAR));
						need_free = 1;
					}
					if (tr.lpstrText && (SendMessage(_hwnd, EM_GETTEXTRANGE, NULL, (LPARAM)&tr) > 0))
					{
						if (_tcschr(tr.lpstrText, '@') != NULL && _tcschr(tr.lpstrText, ':') == NULL && _tcschr(tr.lpstrText, '/') == NULL) 
						{
							 MoveMemory(tr.lpstrText + (7*sizeof(TCHAR)), tr.lpstrText, (tr.chrg.cpMax - tr.chrg.cpMin + 1)*sizeof(TCHAR));
							 memcpy(tr.lpstrText, _T("mailto:"), (7*sizeof(TCHAR)));
						}

						LPSTR pszUrl = mir_t2a(tr.lpstrText);
						if (pszUrl)
						{
							CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)pszUrl);
							mir_free(pszUrl);
						}
					}
					if (need_free)
					{
						MIR_FREE(tr.lpstrText);
					}
				}
			}
		}
	}
	return FALSE;
}