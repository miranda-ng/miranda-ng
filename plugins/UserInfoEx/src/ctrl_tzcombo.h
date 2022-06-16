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

#ifndef _UI_CTRL_TZ_COMBO_INCLUDE_
#define _UI_CTRL_TZ_COMBO_INCLUDE_

/**
 *
 **/
class CTzCombo : public CBaseCtrl
{
	int _curSel;											//selectet combo index
	
	CTzCombo();
	CTzCombo(HWND hDlg, uint16_t idCtrl, LPCSTR pszSetting);

	int Find(LPTIME_ZONE_INFORMATION pTimeZone) const;		//new core tz interface

public:

	static __forceinline CTzCombo* GetObj(HWND hCtrl) 
		{ return (CTzCombo*) GetUserData(hCtrl); }
	static __forceinline CTzCombo* GetObj(HWND hDlg, uint16_t idCtrl)
		{ return GetObj(GetDlgItem(hDlg, idCtrl)); }

	static CBaseCtrl* CreateObj(HWND hDlg, uint16_t idCtrl, LPCSTR pszSetting);

	virtual void	Release();
//	virtual void	OnReset() {};
	virtual BOOL	OnInfoChanged(MCONTACT hContact, LPCSTR pszProto);
	virtual void	OnApply(MCONTACT hContact, LPCSTR pszProto);
	virtual void	OnChangedByUser(uint16_t wChangedMsg);

	void			GetTime(LPTSTR szTime, int cchTime);
};

#endif /* _UI_CTRL_TZ_COMBO_INCLUDE_ */