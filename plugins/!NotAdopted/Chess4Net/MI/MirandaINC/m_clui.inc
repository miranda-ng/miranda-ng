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

{$IFNDEF M_CLUI}
{$DEFINE M_CLUI}

    {<</
        this header was created for use for v0.1.1.0, most of it's UI related
        stuff and you probably don't need to call it, see m_clist.inc instead.
        -- There are some functions that were implemented in v0.1.2.0 though
    />>}

const

    {
        wParam : 0
        lParam : 0
        Affects: Returns a window handle for the contact list window, see notes
        Returns: ""
        Notes  : This call has a very specific purpose internally Miranda
                 and shouldn't be used gratuitously, in almost all cases
                 there's another call to do whatever it is that you're
                 trying to do.
    }
    MS_CLUI_GETHWND                     = 'CLUI/GetHwnd';

    {
        wParam : new status
        lParam : null terminated string to a protocol ID
        Affects: Change the protocol specific status indicators, see notes!
        Returns: 0 on success, [non zero] on failure
        Notes  : protocol modules don't want to call this, they want
                 clist/protocolstatuschanged instead
    }
    MS_CLUI_PROTOCOLSTATUSCHANGED       = 'CLUI/ProtocolStatusChanged';

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
    MS_CLUI_GROUPADDED                  = 'CLUI/GroupCreated';

    {
        wParam : MCONTACT
        lParam : ICON_ID
        Affect : Change the icon for a contact, see notes
        Returns: 0 on success, [non zero] on failure
        Notes  : ICON_ID is an offset in the imagelist, see clist/geticonsimagelist
    }
    MS_CLUI_CONTACTSETICON              = 'CLUI/ContactSetIcon';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : Remove a contact from the list, see notes
        Returns: 0 on success, [non zereo] on failure
        Notes  : this contact is NOT actually being deleted, since if
                 a contact goes offline while 'hide offline' option is sset,
                 this service will be called then ALSO
    }
    MS_CLUI_CONTACTDELETED              = 'CLUI/ContactDeleted';

    {
        wParam : MCONTACT
        lParam : ICON_ID
        Affect : Add a contact to the list, see note
        returns: 0 on success, [non zero] on failure
        Notes  : the caller processes the 'hide offline' setting, so the callee
                 should not do further processing based on the value of this setting
                 -
                 WARNING: this will be called to re-add a contact when they come
                 online if 'hide offline' is on, but it cannot determine if
                 the contact is already on the list, so you may get requests to
                 add a contact when it is already on the list, which you should ignore.
                 -
                 You'll also get this whenever an event is added for a contact,
                 since if the contact was offline, it needs to be shown to
                 display the mesage, even if 'hide offlines' is on.
                 -
                 you should not resort the list on this call, a seperate resort
                 request will be sent.
                 -
                 ICON_ID is an offset in the image list, see clist/geticonsimagelist

    }
    MS_CLUI_CONTACTADDED                = 'CLUI/ContactAdded';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : Reename a contact in the lists, see notes
        Returns: 0 on success, [non zero] on failure
        Notes  : You should not re-sort the list on this call, a separate resort
                 request will be sent, you can get the new name from clist/getcontactdisplayname
    }
    MS_CLUI_CONTACTRENAMED              = 'CLUI/ContactRenamed';

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
    MS_CLUI_LISTBEGINREBUILD            = 'CLUI/ListBeginRebuild';

    {
        wParam : 0
        lParam : 0
        Affect : End a rebuild of the contact list, see notes
        Returns: 0 on success, [non zero] on error
        Notes  : if you dissplayed an hourglass in beginbuild, set it back
                 here, you do not need to explicitly sort the list
    }
    MS_CLUI_LISTENDREBUILD              = 'CLUI/ListEndRebuild';

    {
        wParam : 0
        lParam : 0
        Affect : Sort the contact list now, see notes
        Returns: 0 success, [non zero] on failure
        Notes  : Sorts are buffered so you won't get this message lots of times
                 if the lists needs to be resorted many times rapidly
    }
    MS_CLUI_SORTLIST                    = 'CLUI/SortList';

    {
        wParam : CLUICAPS_*
        lParam : 0
        Affect : Gets a load of capabilites for the loaded CLUI, see notes
        Returns: the requested value, 0 of wParam is unknown --
                 if this service is not implemented it is assumed all return
                 values will be 0.
        Version: v0.1.2.1+
    }

    { can only provide this flag to return the following set of caps, the strings
    show the database setting/type to store the list option, changing the value
    does not reflect what the change is, i.e. ontop can only be affected with
    a call to SetWindowPos() }
    CLUICAPS_FLAGS1         = 0;
    { empty groups aren't shown, 'CList/HideEmptyGroups' (byte) [changes make the list reload] }
    CLUIF_HIDEEMPTYGROUPS   = 1;
    { groups can be disabled, lists can be merged into one seamlessly, (byte) 'CList/UseGroups' }
    CLUIF_DISABLEGROUPS     = 2;
    { list can be displayed 'on top' of all other windows, 4 (byte) 'CList/OnTop'  }
    CLUIF_HASONTOPOPTION    = 4;
    { can disappear after a while of inactive use,
    (byte) 'CList/AutoHide' (word) 'CList/HideTime' }
    CLUIF_HASAUTOHIDEOPTION = 8;

    MS_CLUI_GETCAPS                     = 'CLUI/GetCaps';

    {
        wParam : MCONTACT
        lParam : MAKELPARAM(screenX, screenY)
        Affect : A contact is being dragged outside the main window
        Return : return [non zero] to show the drag cursor as "accepting" the drag
                 or zero to show the circle/slash 'not allowed'
        Version: v0.1.2.0+
    }
    ME_CLUI_CONTACTDRAGGING             = 'CLUI/ContactDragging';

    {
        wParam : MCONTACT
        lParam : MAKELPARAM(screenX, screenY)
        Affect : a contact has just been dropped outside the main window, see notes
        Notes  : return non zero to stop other hooks processing this event.
        Version: v0.1.2.0+
    }
    ME_CLUI_CONTACTDROPPED              = 'CLUI/ContactDropped';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : A contact that *was* being dragged outside the main window
                 has gone back to the main window
        Return : always return 0
        Version: v0.1.2.1+
    }
    ME_CLUI_CONTACTDRAGSTOP             = 'CLUI/ContactDragStop';

{$ENDIF}