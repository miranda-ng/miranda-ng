/*

Import plugin for Miranda IM

Copyright (C) 2001,2002,2003,2004 Martin Öberg, Richard Hughes, Roland Rabien & Tristan Van de Vreede

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



#ifndef ICQSERVER_H
#define ICQSERVER_H

#include <windows.h>

// ======================
// == GLOBAL FUNCTIONS ==
// ======================

// =====================
// == LOCAL FUNCTIONS ==
// =====================

// Main function
static void ICQserverImport();

// GUI callbacks
INT_PTR CALLBACK FinishedPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);


// ======================
// == GLOBAL VARIABLES ==
// ======================

extern int      cICQAccounts;
extern char  ** szICQModuleName;
extern TCHAR ** tszICQAccountName;
extern int      iICQAccount;

// =====================
// == LOCAL VARIABLES ==
// =====================

// =============
// == DEFINES ==
// =============

#endif