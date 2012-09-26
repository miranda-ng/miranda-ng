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
{$IFNDEF M_SKIN}
{$DEFINE M_SKIN}

const

    // event icons

    SKINICON_EVENT_MESSAGE		= 100;
    SKINICON_EVENT_URL			= 101;
    SKINICON_EVENT_FILE			= 102;

    // other icons
    SKINICON_OTHER_MIRANDA		= 200;
    SKINICON_OTHER_EXIT			= 201;
    SKINICON_OTHER_SHOWHIDE		= 202;
    SKINICON_OTHER_GROUPOPEN    = 203; // v0.1.1.0+
    SKINICON_OTHER_GROUPSHUT    = 205; // v0.1.1.0+
    SKINICON_OTHER_USERONLINE   = 204; // v0.1.0.1+

    // menu icons are owned by the module that uses them so are not and should not
    // be skinnable. Except exit and show/hide

    // status mode icons. NOTE: These are deprecated in favour of LoadSkinnedProtoIcon()
    SKINICON_STATUS_OFFLINE	    = 0;
    SKINICON_STATUS_ONLINE		= 1;
    SKINICON_STATUS_AWAY		= 2;
    SKINICON_STATUS_NA			= 3;
    SKINICON_STATUS_OCCUPIED	= 4;
    SKINICON_STATUS_DND			= 5;
    SKINICON_STATUS_FREE4CHAT	= 6;
    SKINICON_STATUS_INVISIBLE	= 7;
    SKINICON_STATUS_ONTHEPHONE	= 8;
    SKINICON_STATUS_OUTTOLUNCH	= 9;

type

    PSKINSOUNDDESC = ^TSKINSOUNDDESC;
    TSKINSOUNDDESC = record
        cbSize: int;
        { name to refer to sound when playing and in DB }
        pszName: PChar;
        { description to use for it in options dialog }
        pszDescription: PChar;
        { the default sound file to use, WITHOUT path }
        pszDefaultFile: PChar;
    end;

const

    {
        wParam : ICON_ID
        lParam : 0
        Affect : Load an icon from the user's custom skin lib, or from the exe
                 if there isn't one loaded, see notes
        Return : HICON for the new icon, do *not* DestroyIcon() the return value
                 returns NULL(0) if ICON_ID is invalid, but always success for a valid
                 ID.
    }
    MS_SKIN_LOADICON            = 'Skin/Icons/Load';

    {
        wParam : null terminated string containing the protocol name
        lParam : status_wanted
        Affect : Load an icon representing the status_wanted for a particular protocol, see notes
        Returns: an HICON for the new icon, do NOT DestroyIcon() the return value
                 returns NULL(0) on failure.
        Notes  : If wParam is NULL(0) the service will load the user's selected
                 'all protocols' status icon
    }
    MS_SKIN_LOADPROTOICON       = 'Skin/Icons/LoadProto';

    {
        wParam : 0
        lParam : Pointer to a initialised SKINSOUNDDESC
        Affect : Add a new sound so it has a default and can be changed in the options dialog
        Returns: 0 on success, [non zero] on failure
    }
    MS_SKIN_ADDNEWSOUND         = 'Skin/Sounds/AddNew';

    {
        wParam : 0
        lParam : Pointer to a null terminated string containing the name of the sound to play
        Affect : play a named sound event, play name should of been added
                 with MS_SKIN_ADDNEWSOUND,  see notes
        Notes  : function will not fail, it will play the Windows
    }
    MS_SKIN_PLAYSOUND           = 'Skin/Sounds/Play';

    {
        wParam : 0
        lParam : 0
        Affect : Sent when the icons DLL has been changed in the options dialog
                 and everyone should remake their image lists.
    }
    ME_SKIN_ICONSCHANGED        = 'Skin/IconsChanged';

{$ENDIF}
