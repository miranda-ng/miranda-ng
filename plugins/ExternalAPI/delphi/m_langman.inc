{
'Language Pack Manager'-Plugin for
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2005-2007 H. Herkenrath

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
}

{$IFNDEF M_LANGMAN}
{$DEFINE M_LANGMAN}

const

{
 Language Pack Manager v1.0.2.1
 If you have any question or extension whishes please let me know:
 hrathh at users.sourceforge.net
}

{ interface id }
  MIID_LANGMAN: TGUID = '{D80370D5-4B1E-46a8-ACA4-1FAAD79B7D1E}';

{ Show the download language dialog.   v1.0.1.2+
 wParam : 0
 lParam : 0
The dialog can't have a parent due to it's asynchronous nature.
If the language window is already opened it will be
brought to front instead (returns success).
Returns 0 on success, nonzero otherwise.
}
  MS_LANGMAN_SHOWLANGDIALOG = 'LangMan/ShowLangDialog';

{$ENDIF} // M_LANGMAN
