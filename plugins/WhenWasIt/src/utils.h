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

#ifndef M_WWI_UTILS_H
#define M_WWI_UTILS_H

#include <stdarg.h>

#define LOG_FILE "wwi.log"

#define ANCHOR_LEFT     0x000001
#define ANCHOR_RIGHT		0x000002
#define ANCHOR_TOP      0x000004
#define ANCHOR_BOTTOM   0x000008
#define ANCHOR_ALL      ANCHOR_LEFT | ANCHOR_RIGHT | ANCHOR_TOP | ANCHOR_BOTTOM

int LogInit();
int Log(char *format, ...);
int Info(char *title, char *format, ...);

wchar_t* GetContactID(MCONTACT hContact);
wchar_t* GetContactID(MCONTACT hContact, char *szProto);
MCONTACT GetContactFromID(wchar_t *szID, char *szProto);
MCONTACT GetContactFromID(wchar_t *szID, wchar_t *szProto);

void CreateToolTip(HWND target, const wchar_t *tooltip, LPARAM width);

void CheckConvert(void);

#endif