{
Copyright (C) 2006 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
}

{$IFNDEF M_NICKHISTORY}
{$DEFINE M_NICKHISTORY}

const
  EVENTTYPE_NICKNAME_CHANGE = 9001;

{
Return TRUE is Nick History is enabled for this contact

wParam: hContact
lParam: ignored
}
  MS_NICKHISTORY_ENABLED = 'NickHistory/Enabled';

{
Enable Nick History for a contact

wParam: hContact
lParam: ignored
}
  MS_NICKHISTORY_ENABLE = 'NickHistory/Enable';

{
Disable Nick History for a contact

wParam: hContact
lParam: ignored
}
  MS_NICKHISTORY_DISABLE = 'NickHistory/Disable';

{$ENDIF}
