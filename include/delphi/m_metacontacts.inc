{
  Miranda IM: the free IM client for Microsoft  Windows 
  
  Copyright © 2004 Universite Louis PASTEUR, STRASBOURG.
  Copyright © 2004 Scott Ellis (www.scottellis.com.au mail@scottellis.com.au)
  
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
{$IFNDEF M_METACONTACTS}
{$DEFINE M_METACONTACTS}

const
// standard module for all mc-related information
  META_PROTO = 'MetaContacts';

const
  { gets the handle for the default contact
    wParam=(HANDLE)hMetaContact
    lParam=0
    returns a handle to the default contact, or null on failure }
  MS_MC_GETDEFAULTCONTACT = 'MetaContacts/GetDefault';

  { gets the contact number for the default contact
    wParam=(HANDLE)hMetaContact
    lParam=0
    returns a dword contact number, or -1 on failure }
  MS_MC_GETDEFAULTCONTACTNUM = 'MetaContacts/GetDefaultNum';

  { gets the handle for the 'most online' contact
    wParam=(HANDLE)hMetaContact
    lParam=0
    returns a handle to the 'most online' contact }
  MS_MC_GETMOSTONLINECONTACT = 'MetaContacts/GetMostOnline';

  { gets the number of subcontacts for a metacontact
    wParam=(HANDLE)hMetaContact
    lParam=0
    returns a dword representing the number of subcontacts for the given metacontact }
  MS_MC_GETNUMCONTACTS = 'MetaContacts/GetNumContacts';

  { gets the handle of a subcontact, using the subcontact's number
    wParam=(HANDLE)hMetaContact
    lParam=(dword)contact number
    returns a handle to the specified subcontact }
  MS_MC_GETSUBCONTACT = 'MetaContacts/GetSubContact';

  { sets the default contact, using the subcontact's contact number
    wParam=(HANDLE)hMetaContact
    lParam=(dword)contact number
    returns 0 on success }
  MS_MC_SETDEFAULTCONTACTNUM = 'MetaContacts/SetDefault';

  { sets the default contact, using the subcontact's handle
    wParam=(HANDLE)hMetaContact
    lParam=(HANDLE)hSubcontact
    returns 0 on success }
  MS_MC_SETDEFAULTCONTACT = 'MetaContacts/SetDefaultByHandle';

  { forces the metacontact to send using a specific subcontact, using the subcontact's contact number
    wParam=(HANDLE)hMetaContact
    lParam=(dword)contact number
    returns 0 on success }
  MS_MC_FORCESENDCONTACTNUM = 'MetaContacts/ForceSendContact';

  { forces the metacontact to send using a specific subcontact, using the subcontact's handle
    wParam=(HANDLE)hMetaContact
    lParam=(HANDLE)hSubcontact
    returns 0 on success (will fail if 'force default' is in effect) }
  MS_MC_FORCESENDCONTACT = 'MetaContacts/ForceSendContactByHandle';

  { 'unforces' the metacontact to send using a specific subcontact
    wParam=(HANDLE)hMetaContact
    lParam=0
    returns 0 on success (will fail if 'force default' is in effect) }
  MS_MC_UNFORCESENDCONTACT = 'MetaContacts/UnforceSendContact';

  { 'forces' or 'unforces' (i.e. toggles) the metacontact to send using it's default contact
    overrides (and clears) 'force send' above, and will even force use of offline contacts
    will send ME_MC_FORCESEND or ME_MC_UNFORCESEND event
    wParam=(HANDLE)hMetaContact
    lParam=0
    returns 1(true) or 0(false) representing new state of 'force default' }
  MS_MC_FORCEDEFAULT = 'MetaContacts/ForceSendDefault';

  { method to get state of 'force' for a metacontact
    wParam=(HANDLE)hMetaContact
    lParam= (dword)&contact_number or NULL
    if lparam supplied, the contact_number of the contatct 'in force' will be copied to the address it points to,
    or if none is in force, the value INVALID_CONTACT_ID will be copied
    (v0.8.0.8+ returns 1 if 'force default' is true with *lParam == default contact number, else returns 0 with *lParam as above) }
  MS_MC_GETFORCESTATE = 'MetaContacts/GetForceState';

  { fired when a metacontact's default contact changes (fired upon creation of metacontact also, when default is initially set)
    wParam=(HANDLE)hMetaContact
    lParam=(HANDLE)hDefaultContact }
  ME_MC_DEFAULTTCHANGED = 'MetaContacts/DefaultChanged';

  { fired when a metacontact's subcontacts change (fired upon creation of
    metacontact, when contacts are added or removed, and when contacts are
    reordered) - a signal to re-read metacontact data
    wParam=(HANDLE)hMetaContact
    lParam=0 }
  ME_MC_SUBCONTACTSCHANGED = 'MetaContacts/SubcontactsChanged';

  { fired when a metacontact is forced to send using a specific subcontact
    wParam=(HANDLE)hMetaContact
    lParam=(HANDLE)hForceContact  }
  ME_MC_FORCESEND = 'MetaContacts/ForceSend';

  { fired when a metacontact is 'unforced' to send using a specific subcontact
    wParam=(HANDLE)hMetaContact
    lParam=0  }
  ME_MC_UNFORCESEND = 'MetaContacts/UnforceSend';

  { wParam=(HANDLE)hContact
    lParam=0
    convert a given contact into a metacontact
  }
  MS_MC_CONVERTTOMETA = 'MetaContacts/ConvertToMetacontact';

  { wParam=(HANDLE)hContact
    lParam=(HANDLE)hMeta
    add an existing contact to a metacontact
  }
  MS_MC_ADDTOMETA = 'MetaContacts/AddToMetacontact';

  { wParam=0
    lParam=(HANDLE)hContact
    remove a contact from a metacontact
  }
  MS_MC_REMOVEFROMMETA = 'MetaContacts/RemoveFromMetacontact';


/////////////////////////////////////////////////////////////////////////////////////////
// binary interface to MC

{ returns true if a contact is a metacontact or false otherwise
}
function db_mc_isMeta(hContact:TMCONTACT):int; stdcall;
                 external CoreDLL name 'db_mc_isMeta';

{ returns true if a contact is a subcontact or false otherwise
}
function db_mc_isSub(hContact:TMCONTACT):int; stdcall;
                 external CoreDLL name 'db_mc_isSub';

{ returns a meta contact for a subcontact or 0 otherwise
}
function db_mc_getMeta(hContact:TMCONTACT):TMCONTACT; stdcall;
                 external CoreDLL name 'db_mc_getMeta';

{$ENDIF}
