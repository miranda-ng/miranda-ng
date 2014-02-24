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

{$IFNDEF M_USERINFO}
{$DEFINE M_USERINFO}

const

    {
        wParam : MCONTACT
        lParam : 0
        Affects: Show the user details dialog box for a contact, see notes
        Notes  : I think this can be used to display "My User Details"... if NULL(0) is used
    }
    MS_USERINFO_SHOWDIALOG          = 'UserInfo/ShowDialog';

    {
        wParam : 0
        lParam : MCONTACT
        Affects: The details dialog box was opened for a contact maybe NULL(0)
                 showing the user details -- see notes
        Notes  : The module should do whatever initialisation they need and
                 call MS_USERINFO_ADDPAGE one or more times if they want
                 pages displayed in the options dialog -- wParam should be passed
                 straight as the wParam of MS_USERINFO_ADDPAGE.
                 -
                 The builtin userinfo module is not loaded til after all plugins
                 have loaded -- therefore a HookEvent() for this event will fail,
                 use ME_SYSTEM_MODULESLOADED event to know when everything has
                 loaded and it's okay to hook this event.
        Version: v0.1.2.0+
    }
    ME_USERINFO_INITIALISE          = 'UserInfo/Initialise';

    {
        wParam : wParam from ME_USERINFO_INITIALISE
        lParam : pointer to an initialised OPTIONSDIALOGPAGE (see m_options.inc)
        Affects: Adds a page to the details dialog, see notes
        Notes  : this service should only be called within the ME_USERINFO_INITIALISE
                 event -- when the pages get (WM_INITDIALOG lParam=MCONTACT) strings
                 in the passed dialog structure can be freed soon as the service returns
                 icons must be kept around (not a problem if you're loading from resource).
                 -
                 The group elements within the OPTIONSDIALOGPAGE are ignored,
                 details dialog page should be 222x132 DLU's -- the details dialog
                 box currently has no cancel button, pages will be sent PSN_INFOCHANGED
                 thru WM_NOTIFY (idFrom=0) when a protocol ACK is broadcast for
                 the correct contact with the type ACKTYPE_GETINFO.
                 -
                 PSN_INFOCHANGED will also be sent just after the page is created
                 to help you out.
                 -
                 All PSN_* WM_NOTIFY messages have PSHNOTIFY.lParam=(LPARAM)hContact
        Version: v0.1.2.0+
    }

    PSN_INFOCHANGED                 = 1;
    { force-send a PSN_INFOCHANGED to all pages }
    PSM_FORCECHANGED                = ($0400 + 100);

    MS_USERINFO_ADDPAGE             = 'UserInfo/AddPage';

{$ENDIF}
