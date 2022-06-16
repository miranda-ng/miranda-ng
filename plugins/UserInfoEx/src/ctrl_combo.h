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

#ifndef _UI_CTRL_COMBO_INCLUDE_
#define _UI_CTRL_COMBO_INCLUDE_

/**
 *
 **/
class CCombo : public CBaseCtrl
{
	int					_curSel;
	LPIDSTRLIST _pList;
	int					_nList;
	uint8_t				_bDataType;
	
	/**
	 * Private constructure is to force to use static member 'Create' 
	 * as the default way of attaching a new object to the window control.
	 *
	 * @param	 none
	 *
	 * @return	nothing
	 **/
	CCombo(HWND hDlg, uint16_t idCtrl, LPCSTR pszSetting, uint8_t bDBDataType, LPIDSTRLIST pList, int nListCount);

	int Find(int nIndex) const;
	int Find(LPTSTR ptszItemLabel) const;

	INT_PTR AddItem(LPCTSTR pszText, LPARAM lParam);

public:

	/**
	 *
	 *
	 **/
	static __forceinline CCombo* GetObj(HWND hCtrl) 
		{ return (CCombo*) GetUserData(hCtrl); }
	static __forceinline CCombo* GetObj(HWND hDlg, uint16_t idCtrl)
		{ return GetObj(GetDlgItem(hDlg, idCtrl)); }

	static CBaseCtrl* CreateObj(HWND hDlg, uint16_t idCtrl, LPCSTR pszSetting, uint8_t bDBDataType, LPIDSTRLIST pList, int nListCount);

	virtual void	Release();
	virtual BOOL	OnInfoChanged(MCONTACT hContact, LPCSTR pszProto);
	virtual void	OnApply(MCONTACT hContact, LPCSTR pszProto);
	virtual void	OnChangedByUser(uint16_t wChangedMsg);
};

#endif /* _UI_CTRL_COMBO_INCLUDE_ */