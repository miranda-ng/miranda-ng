{
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
}

{$IFNDEF M_HELP}
{$DEFINE M_HELP}

const

{
 Help Plugin 0.2.1.2
 All services in here except MS_HELP_SHOWLANGDIALOG should be thread-safe,
 you can call them from any thread
}

{ interface id }
  MIID_HELP: TGUID = '{302660C5-1BF6-4054-A79F-77B1965D6F48}';

{ Enable/disable the help context menu for a specific control.   v0.2.0.0+
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
 wParam : (HWND)hwndCtl
 lParam : flags (see below)
Returns 0 on success or nonzero on failure
}
  MS_HELP_SETCONTEXTSTATE = 'Help/SetContextState';
  HCSF_CONTEXTMENU = $01; // show help context menu for this control
  HCSF_AUTOTIP     = $02; // show automatic help tip on hover for this control
                          // only works for non-editable

{ Show a help tooltip for a specific control or dialog.   v0.2.0.0+
You can call this if you would like to show help at a specific time.
 wParam : (HWND)hwndCtl
 lParam : 0
Returns 0 on success or nonzero on failure.
The service fails when the help tooltip cannot be instantiated.
}
  MS_HELP_SHOWHELP = 'Help/ShowHelp';

{ Show the download language dialog.   v0.2.1.0+
 wParam : 0
 lParam : 0
The dialog can't have a parent due to it's asynchronous nature.
If the language window is already opened it will be
brought to front instead (returns success).
Returns 0 on success, nonzero otherwise.
}
  MS_HELP_SHOWLANGDIALOG = 'Help/ShowLangDialog';

{$ENDIF}
