/*

'Language Pack Manager'-Plugin for
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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
