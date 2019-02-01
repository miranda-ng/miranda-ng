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
  { fired when a metacontact's default contact changes (fired upon creation of metacontact also, when default is initially set)
    wParam=(HANDLE)hMetaContact
    lParam=(HANDLE)hDefaultContact }
  ME_MC_DEFAULTTCHANGED:PAnsiChar = 'MetaContacts/DefaultChanged';

  { fired when a metacontact's subcontacts change (fired upon creation of
    metacontact, when contacts are added or removed, and when contacts are
    reordered) - a signal to re-read metacontact data
    wParam=(HANDLE)hMetaContact
    lParam=0 }
  ME_MC_SUBCONTACTSCHANGED:PAnsiChar = 'MetaContacts/SubcontactsChanged';

  { signalizes that metacontacts are enabled or disabled
    wParam=(BOOL)bEnabled
    lParam=0
  }
  ME_MC_ENABLED:PAnsiChar = 'MetaContacts/OnEnabled';

/////////////////////////////////////////////////////////////////////////////////////////
// binary interface to MC

{ returns true if metacontacts are enabled or false otherwise }
function db_mc_isEnabled():bool; stdcall; external AppDLL;

{ returns true if a contact is a metacontact or false otherwise }
procedure db_mc_enable(bEnabled:bool); stdcall; external AppDLL;

{ returns true if a contact is a metacontact or false otherwise }
function db_mc_isMeta(hContact:TMCONTACT):bool; stdcall; external AppDLL;

{ returns true if a contact is a subcontact or false otherwise }
function db_mc_isSub(hContact:TMCONTACT):bool; stdcall; external AppDLL;

{ gets the handle for the default contact or 0 on error }
function db_mc_getDefault(hContact:TMCONTACT):TMCONTACT; stdcall; external AppDLL;

{ gets the contact number for the default contact or -1 on error }
function db_mc_getDefaultNum(hContact:TMCONTACT):int; stdcall; external AppDLL;

{ returns a meta contact for a subcontact or 0 otherwise }
function db_mc_getMeta(hContact:TMCONTACT):TMCONTACT; stdcall; external AppDLL;

{ returns a meta contact for a subcontact or hContact itself otherwise }
function db_mc_tryMeta(hContact:TMCONTACT):TMCONTACT; stdcall; external AppDLL;

{ returns a subcontact with the given index or 0 }
function db_mc_getSub(hContact:TMCONTACT; idx:int):TMCONTACT; stdcall; external AppDLL;

// gets the handle for the 'most online' contact
function db_mc_getMostOnline(hMetaContact:TMCONTACT):TMCONTACT; stdcall; external AppDLL;

{ gets the number of subcontacts for a metacontact or -1 on error }
function db_mc_getSubCount(hContact:TMCONTACT):int; stdcall; external AppDLL;

{ sets the default contact, using the subcontact's handle }
function db_mc_setDefault(hContact,hSub:TMCONTACT; bWriteDb:bool):int; stdcall; external AppDLL;

{ sets the default contact, using the subcontact's number }
function db_mc_setDefaultNum(hContact:TMCONTACT; idx:int; bWriteDb:bool):int; stdcall; external AppDLL;

// converts a given contact into a metacontact
function db_mc_convertToMeta(hContact:TMCONTACT):int; stdcall; external AppDLL;

// converts a given contact into a metacontact
function db_mc_addToMeta(hContact,hMeta:TMCONTACT):int; stdcall; external AppDLL;

{$ENDIF}
