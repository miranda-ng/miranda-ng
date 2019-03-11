/*
    Helper functions for Miranda-IM (www.miranda-im.org)
    Copyright 2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __GEN_HELPERS_H
#define __GEN_HELPERS_H

TCHAR* Hlp_GetProtocolName(const char *proto);
TCHAR* Hlp_GetDlgItemText(HWND hwndDlg, int nIDDlgItem);
TCHAR* Hlp_GetWindowText(HWND hwndDlg);

#define MAX_DEBUG	1024

#define __LOGLEVEL_DEBUG	10
#define __LOGLEVEL_INFO		9

#ifndef LOGLEVEL
 #ifdef _DEBUG
  #define LOGLEVEL	__LOGLEVEL_DEBUG
 #else
  #define LOGLEVEL	__LOGLEVEL_INFO
 #endif
#endif

#if LOGLEVEL >= __LOGLEVEL_DEBUG
#define log_debug Netlib_Logf
#else
static __inline int log_debug(int, const char*, ...) { return 0; }
#endif

#if LOGLEVEL >= __LOGLEVEL_INFO
#define log_info Netlib_Logf
#else
static __inline int log_info(int, const char*, ...) { return 0; }
#endif

#endif
