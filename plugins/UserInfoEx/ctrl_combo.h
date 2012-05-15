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

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/ctrl_combo.h $
Revision       : $Revision: 187 $
Last change on : $Date: 2010-09-08 16:05:54 +0400 (Ср, 08 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/

#ifndef _UI_CTRL_COMBO_INCLUDE_
#define _UI_CTRL_COMBO_INCLUDE_

#include "ctrl_base.h"

/**
 *
 **/
class CCombo : public CBaseCtrl
{
	INT					_curSel;
	LPIDSTRLIST _pList;
	INT					_nList;
	BYTE				_bDataType;
	
	/**
	 * Private constructure is to force to use static member 'Create' 
	 * as the default way of attaching a new object to the window control.
	 *
	 * @param	 none
	 *
	 * @return	nothing
	 **/
	CCombo(HWND hDlg, WORD idCtrl, LPCSTR pszSetting, BYTE bDBDataType, LPIDSTRLIST pList, INT nListCount);

	INT Find(INT nIndex) const;
	INT Find(LPTSTR ptszItemLabel) const;

	INT_PTR AddItem(LPCTSTR pszText, LPARAM lParam);

public:

	/**
	 *
	 *
	 **/
	static FORCEINLINE CCombo* GetObj(HWND hCtrl) 
		{ return (CCombo*) GetUserData(hCtrl); }
	static FORCEINLINE CCombo* GetObj(HWND hDlg, WORD idCtrl)
		{ return GetObj(GetDlgItem(hDlg, idCtrl)); }

	static CBaseCtrl* CreateObj(HWND hDlg, WORD idCtrl, LPCSTR pszSetting, BYTE bDBDataType, LPIDSTRLIST pList, INT nListCount);

	virtual VOID	Release();
	virtual BOOL	OnInfoChanged(HANDLE hContact, LPCSTR pszProto);
	virtual VOID	OnApply(HANDLE hContact, LPCSTR pszProto);
	virtual VOID	OnChangedByUser(WORD wChangedMsg);
};

#endif /* _UI_CTRL_COMBO_INCLUDE_ */