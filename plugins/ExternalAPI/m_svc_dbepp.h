/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright © 2003-2011 Bio, Jonathan Gordon

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
Register single module as 'known'
wParam=(char*)Module name
lParam=0
always returns 0
Note: must be used after or in ME_SYSTEM_MODULESLOADED
*/
#define MS_DBEDIT_REGISTERSINGLEMODULE "DBEditorpp/RegisterSingleModule"

/*
Register modules as 'known'
wParam=(char**)array with module names
lParam=(int)count of module names
always returns 0
Note: must be used after or in ME_SYSTEM_MODULESLOADED
Example:
	char * mods[3] = {"Module1", "Module2", "Module3"};
	CallService(MS_DBEDIT_REGISTERMODULE, (WPARAM)mods, (LPARAM)3);
*/
#define MS_DBEDIT_REGISTERMODULE "DBEditorpp/RegisterModule"

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