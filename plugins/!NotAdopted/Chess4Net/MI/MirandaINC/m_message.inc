(*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

*)

{$IFNDEF M_MESSAGE}
{$DEFINE M_MESSAGE}

const

    {
        wParam : MCONTACT
        lParam : Pointer to a null terminated string
        Affects: brings up the send message dialog for a contact, see notes
        Returns: 0 on success, non zero on failure
        Notes  : returns immediately, just after the send dialog is shown,
                 the lParam is entered into the editbox of the window,
                 but it's not sent.
        Version: v0.1.2.0+ only supports a string, prior NULL(0) is expected
                 this service was defined as 'SRMsg/LaunchMessageWindow'
                 use both if compatibility use both, the correct one will work,
                 but don't rely on the message to be displayed

    }
    MS_MSG_SENDMESSAGE          = 'SRMsg/SendCommand';
    MS_MSG_SENDMESSAGE_OLD      = 'SRMsg/LaunchMessageWindow';

    {
        wParam : 0
        lParam : Pointer to a null termed string
        Affects: displays the send message dialog with the 'multiple' option open
                 and no contacts selected
        Returns: Returns 0 on success, nonzero on failure
        Version: only present after v0.1.2.1+
    }
    MS_MSG_FORWARDMESSAGE       = 'SRMsg/ForwardMessage';

{$ENDIF}
