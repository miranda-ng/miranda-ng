/*

'Language Pack Manager'-Plugin for
Miranda IM: the free IM client for Microsoft* Windows*

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (LangMan-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_LANGMAN_H__
#define M_LANGMAN_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
 #pragma once
#endif

/*
 Language Pack Manager v1.0.2.2
 If you have any question or extension whishes please let me know:
 hrathh at users.sourceforge.net
*/

/* interface id */
#if !defined(MIID_LANGMAN)
 #define MIID_LANGMAN  {0xd80370d5,0x4b1e,0x46a8,{0xac,0xa4,0x1f,0xaa,0xd7,0x9b,0x7d,0x1e}}
#endif

/* Show the download language dialog.   v1.0.2.0+
 wParam=lParam=0
The dialog can't have a parent due to it's asynchronous nature.
If the language window is already opened it will be
brought to front instead (returns success).
Returns 0 on success, nonzero otherwise.
*/
#define MS_LANGMAN_SHOWLANGDIALOG  "LangMan/ShowLangDialog"


#ifndef LANGMAN_NOSETTINGS
#define SETTING_ENABLEAUTOUPDATES_DEFAULT  1
#endif

#endif // M_LANGMAN_H

