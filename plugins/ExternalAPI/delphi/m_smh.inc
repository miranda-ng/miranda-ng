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

{$IFNDEF M_SMH}
{$DEFINE M_SMH}

const
  EVENTTYPE_STATUSMESSAGE_CHANGE = 9002;

{
Return TRUE is Status Message History is enabled for this contact

wParam: hContact
lParam: ignored
}
  MS_SMH_ENABLED = 'SMH/Enabled';

{
Enable Status Message History for a contact

wParam: hContact
lParam: ignored
}
  MS_SMH_ENABLE = 'SMH/Enable';

{
Disable Status Message History for a contact

wParam: hContact
lParam: ignored
}
  MS_SMH_DISABLE = 'SMH/Disable';

{$ENDIF}
