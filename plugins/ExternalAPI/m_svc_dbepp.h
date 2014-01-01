/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2003-11 Bio, Jonathan Gordon

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

#ifndef M_DBEDITOR
#define M_DBEDITOR

/*
Open user tree in DBE++
wParam=(HANDLE)hContact
lParam=0
always returns 0
*/
#define MS_DBEDIT_MENUCOMMAND "DBEditorpp/MenuCommand"

/*
Import settings\contacts from file
wParam=(HANDLE)hContact
lParam=(char*)FilePath
always returns 0
*/
#define MS_DBEDIT_IMPORT "DBEditorpp/Import"

#endif