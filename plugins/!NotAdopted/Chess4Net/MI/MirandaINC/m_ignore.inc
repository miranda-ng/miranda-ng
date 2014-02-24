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
{$IFNDEF M_IGNORE}
{$DEFINE M_IGNORE}

    { this module only provides UI and storage for blocking only, protocol modules
      are responsible for implementing the block }

const

    IGNOREEVENT_ALL         = LPARAM(-1);
    IGNOREEVENT_MESSAGE     = 1;
    IGNOREEVENT_URL         = 2;
    IGNOREEVENT_FILE        = 3;
    IGNOREEVENT_USERONLINE  = 4;
    IGNOREEVENT_AUTHORIZATION=5;
    IGNOREEVENT_YOUWEREADDED=6; // 0.3.3a+      

    {
        wParam : MCONTACT
        lParam : IGNOREEVENT_*
        Affects: Determines if a message type to a contact should be ignored, see notes
        Returns: 0 if the message type MUST be shown [non zero] if it MUST be ignored
        Notes  : MCONTACT can be NULL(0) to see what to do with a contact
                 that isn't on the list (or is unknown in some way)
                 don't use the IGNOREEVENT_ALL type!
        Version: v0.1.0.1+
    }
    MS_IGNORE_ISIGNORED     = 'Ignore/IsIgnored';

    {
        wParam : MCONTACT
        lParam : IGNOREEVENT_* constant
        Affects: Ignore future messages from a contact, see notes
        Returns: 0 on success, [nonzero] on failure
        Notes  : wParam: NULL(0) can be used to see if an unknown contact should be ignored
                 or not - you can't SET unknown contact's ignore types, this is to stop
                 a plugin allowing certain functions (I guess)
        Version: v0.1.0.1+
    }
    MS_IGNORE_IGNORE        = 'Ignore/Ignore';

    {
        wParam : MCONTACT
        lParam : IGNOREEVENT_*
        Affects: Receive future messages from a contact -- of the given type, see notes
        Returns: 0 on success, non zero on failure
        Notes  : Use NULL(0) for MCONTACT to retrieve the setting for an unknown contact
        Version: v0.1.0.1+
    }
    MS_IGNORE_UNIGNORE      = 'Ignore/Unignore';

{$ENDIF}
