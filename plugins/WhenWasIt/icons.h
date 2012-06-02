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

#include "commonheaders.h"

//extern HICON hiDlg;
extern HICON hiMainMenu;
extern HICON hiCheckMenu;
extern HICON hiListMenu;
extern HICON hiAddBirthdayContact;
extern HICON hiRefreshUserDetails;

extern HICON hiImportBirthdays;
extern HICON hiExportBirthdays;

extern HICON hiDTB[];
extern HICON hiDTBMore;

extern HANDLE hClistImages[];

extern HANDLE hWWIExtraIcons;

int LoadIcons();
int AddIcons();
int GetIcons();

int RebuildAdvIconList();

HICON GetDTBIcon(int dtb);
HICON GetDABIcon(int dtb);
HANDLE GetClistIcon(int dtb);

#endif //M_WWI_ICONS_H