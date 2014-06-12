{
Copyright (C) 2005 Ricardo Pescuma Domenecci

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

{$IFNDEF M_SMR}
{$DEFINE M_SMR}

const
{
Return TRUE is smr is enabled for this protocol
If is enabled, status message is kept under CList\StatusMsg db key in user data

wParam: protocol name
lParam: ignored
}
  MS_SMR_ENABLED_FOR_PROTOCOL = 'SMR/MsgRetrievalEnabledForProtocol';

{
Return TRUE is smr is enabled for this contact and its protocol (smr can be
disabled per user, if protocol is enabled)
If is enabled, status message is kept under CList\StatusMsg db key in user data

wParam: hContact
lParam: ignored
}
  MS_SMR_ENABLED_FOR_CONTACT = 'SMR/MsgRetrievalEnabledForUser';

{
Enable status message retrieval for a contact

wParam: hContact
lParam: ignored
}
  MS_SMR_ENABLE_CONTACT = 'SMR/EnableContactMsgRetrieval';

{
Disable status message retrieval for a contact

wParam: hContact
lParam: ignored
}
  MS_SMR_DISABLE_CONTACT = 'SMR/DisableContactMsgRetrieval';

{$ENDIF}
