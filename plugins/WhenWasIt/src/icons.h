/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_WWI_ICONS_H
#define M_WWI_ICONS_H

extern HANDLE hCheckMenu;
extern HANDLE hListMenu;
extern HANDLE hAddBirthdayContact;
extern HANDLE hRefreshUserDetails;

extern HANDLE hImportBirthdays;
extern HANDLE hExportBirthdays;

extern HANDLE hDTB[];
extern HANDLE hDTBMore;

extern HANDLE hWWIExtraIcons;

int AddIcons();

HANDLE GetDTBIconHandle(int dtb);

__forceinline HICON GetDTBIcon(int dtb)
{
	return Skin_GetIconByHandle(GetDTBIconHandle(dtb));
}

#endif //M_WWI_ICONS_H