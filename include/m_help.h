/*

Miranda IM Help Plugin
Copyright (C) 2002 Richard Hughes, 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Help-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_HELP_H__
#define M_HELP_H__

/* Enable/disable the help context menu for a specific control.   v0.2.0.0+
Note: You normally do not need to call this, read below.
You can can use this to deactivate the appearance of the help context menu
being shown when the user right clicks on an control.
You can use this service to disable the context menu.

You do *not* need to use this service when you would like to show
a context menu by yourself, just handle WM_CONTEXTMENU correctly instead.
You need to return TRUE in your DlgProc or 0 in your WndProc, indicating 'message handled'.

The context menu is disabled by default on the following controls (looks silly on multi-component controls):
ListView, TreeView, Statusbar, Toolbar, CLC
AutoTips are disabled by default for controls stating DLGC_WANTALLKEYS or DLGC_HASSETSEL at 
WM_GETDLGCODE (autotips are annoying on edits).
 wParam=(WPARAM)(HWND)hwndCtl
 lParam=(LPARAM)flags (see below)
Returns 0 on success or nonzero on failure
*/
#define MS_HELP_SETCONTEXTSTATE  "Help/SetContextState"
#define HCSF_CONTEXTMENU  0x01  // show help context menu for this control
#define HCSF_AUTOTIP      0x02  // show automatic help tip on hover for this control
                                // only works for non-editable
#if !defined(HELP_NOHELPERFUNCTIONS)
__inline static int Help_SetContextState(HWND hwndCtl,DWORD flags) {
	if(!ServiceExists(MS_HELP_SETCONTEXTSTATE)) return -1;
	return CallService(MS_HELP_SETCONTEXTSTATE,(WPARAM)hwndCtl,flags);
}
#endif

/* Show a help tooltip for a specific control or dialog.   v0.2.0.0+
You can call this if you would like to show help at a specific time.
 wParam=(WPARAM)(HWND)hwndCtl
 lParam=0 (unused)
Returns 0 on success or nonzero on failure.
The service fails when the help tooltip cannot be instantiated.
*/
#define MS_HELP_SHOWHELP  "Help/ShowHelp"

/* Show the download language dialog.   v0.2.1.0+
 wParam=lParam=0
The dialog can't have a parent due to it's asynchronous nature.
If the language window is already opened it will be
brought to front instead (returns success).
Returns 0 on success, nonzero otherwise.
*/
#define MS_HELP_SHOWLANGDIALOG  "Help/ShowLangDialog"


#ifndef HELP_NOSETTINGS
#define SETTING_AUTOTIPSENABLED_DEFAULT    0
#define SETTING_AUTOTIPDELAY_DEFAULT       4000
#define SETTING_ENABLEHELPUPDATES_DEFAULT  1
#endif

#endif // M_HELP_H__
