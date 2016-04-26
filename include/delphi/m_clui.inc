{
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
}

{$IFNDEF M_CLUI}
{$DEFINE M_CLUI}

{<</
    this header was created for use for v0.1.1.0, most of it's UI related
    stuff and you probably don't need to call it, see m_clist.inc instead.
    -- There are some functions that were implemented in v0.1.2.0 though
/>>}

const
  {
    wParam : Handle to a group
    lParam : 1 or 0
    Affect : A new group was created, add it to the list, see notes
    Notes  : lParam is set to 1 or 0 if the user just created
             the group or not.
             -
             this is also called when the contact list is being rebuilt,
             new groups are always created with the name 'New group'
  }
  MS_CLUI_GROUPADDED:PAnsiChar = 'CLUI/GroupCreated';

  {
    wParam : 0
    lParam : 0
    Affect : Start a rebuild of the contact list, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : this is the cue to clear the existing content of the list
             expect to get a series of :

             clui/groupadded
             clui/contactadded
             clui/resortlist
  }
  MS_CLUI_LISTBEGINREBUILD:PAnsiChar = 'CLUI/ListBeginRebuild';

  {
    wParam : 0
    lParam : 0
    Affect : End a rebuild of the contact list, see notes
    Returns: 0 on success, [non zero] on error
    Notes  : if you dissplayed an hourglass in beginbuild, set it back
             here, you do not need to explicitly sort the list
  }
  MS_CLUI_LISTENDREBUILD:PAnsiChar = 'CLUI/ListEndRebuild';

  {
    wParam : TMCONTACT
    lParam : MAKELPARAM(screenX, screenY)
    Affect : A contact is being dragged outside the main window
    Return : return [non zero] to show the drag cursor as "accepting" the drag
             or zero to show the circle/slash 'not allowed'
    Version: v0.1.2.0+
  }
  ME_CLUI_CONTACTDRAGGING:PAnsiChar = 'CLUI/ContactDragging';

  {
    wParam : TMCONTACT
    lParam : MAKELPARAM(screenX, screenY)
    Affect : a contact has just been dropped outside the main window, see notes
    Notes  : return non zero to stop other hooks processing this event.
    Version: v0.1.2.0+
  }
  ME_CLUI_CONTACTDROPPED:PAnsiChar = 'CLUI/ContactDropped';

  {
    wParam : TMCONTACT
    lParam : 0
    Affect : A contact that *was* being dragged outside the main window
             has gone back to the main window
    Return : always return 0
    Version: v0.1.2.1+
  }
  ME_CLUI_CONTACTDRAGSTOP:PAnsiChar = 'CLUI/ContactDragStop';

  // return TRUE if Clist Module Support Metacontacts
  ME_CLUI_METASUPPORT:PAnsiChar = 'CLUI/MetaContactSupport';

{$ENDIF}
