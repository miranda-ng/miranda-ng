{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2006 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
}

{$IFNDEF M_ERSATZ}
{$DEFINE M_ERSATZ}

const
// Returns the status message for a status
// wParam=(word) 0 for current status or a status
// lParam=0
// Returns status msg or NULL if there is none.  The protocol have to handle only the current 
// status. Handling messages for other statuses is optional.
// Remember to mir_free the return value
  PS_GETMYAWAYMSG = '/GetMyAwayMsg';

// Created if ersatz is installed
// wParam=0
// lParam=0
// returns always 1
  MS_ERSATZ_ENABLED = 'ERSATZ/Enabled';

{$ENDIF}
