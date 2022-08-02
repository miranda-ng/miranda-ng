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

#ifndef _UINFOEX_ICONS_H_INCLUDED_
#define _UINFOEX_ICONS_H_INCLUDED_ 1

// sections
#define SECT_COMMON           LPGEN("Extended user info")
#define SECT_DLG              LPGEN("Extended user info") "/" LPGEN("Dialogs")
#define SECT_BUTTONS          LPGEN("Extended user info") "/" LPGEN("Buttons")
#define SECT_TREE             LPGEN("Extended user info") "/" LPGEN("Treeview")
#define SECT_REMIND           LPGEN("Extended user info") "/" LPGEN("Reminder")

#define ICONINDEX(id)	max((min((id), IDI_LASTICON)) - IDI_FIRST_ICON, 0)

struct ICONCTRL
{
	int   iIcon;
	UINT  Message;
	uint16_t  idCtrl;
};

LPTSTR	IcoLib_GetDefaultIconFileName();
void	IcoLib_SetCtrlIcons(HWND hDlg, const ICONCTRL* pCtrl, uint8_t numCtrls);

void	IcoLib_LoadModule();

#endif /* _UINFOEX_ICONS_H_INCLUDED_ */
