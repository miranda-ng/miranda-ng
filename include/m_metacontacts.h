/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2004-07 Scott Ellis (www.scottellis.com.au mail@scottellis.com.au)
Copyright (c) 2004 Universite Louis PASTEUR, STRASBOURG.

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
*/

#ifndef M_METACONTACTS_H__
#define M_METACONTACTS_H__ 1

#include <m_core.h>

// standard module for all mc-related information
#define META_PROTO   "MetaContacts"
#define META_PROTOW L"MetaContacts"

// fired when a metacontact's default contact changes (fired upon creation of metacontact also, when default is initially set)
// wParam=(HANDLE)hMetaContact
// lParam=(HANDLE)hDefaultContact
#define ME_MC_DEFAULTTCHANGED "MetaContacts/DefaultChanged"

// fired when a metacontact's subcontacts change (fired upon creation of metacontact, when contacts are added or removed, and when
//	contacts are reordered) - a signal to re-read metacontact data
// wParam=(HANDLE)hMetaContact
// lParam=0
#define ME_MC_SUBCONTACTSCHANGED "MetaContacts/SubcontactsChanged"

// wParam=(BOOL)bEnabled
// lParam=0
// signalizes that metacontacts are enabled or disabled
#define ME_MC_ENABLED "MetaContacts/OnEnabled"

/////////////////////////////////////////////////////////////////////////////////////////
// binary interface to MC

#if defined(__cplusplus)
extern "C"
{
#endif

// checks whether metacontacts are enabled
MIR_APP_DLL(BOOL) db_mc_isEnabled(void);
MIR_APP_DLL(void) db_mc_enable(BOOL);

// checks whether a contact is a metacontact
MIR_APP_DLL(BOOL) db_mc_isMeta(MCONTACT hMetaContact);

// checks whether a contact is a subcontact of existing MC
MIR_APP_DLL(BOOL) db_mc_isSub(MCONTACT hSubContact);

// returns a handle to the default contact, or null on failure
MIR_APP_DLL(MCONTACT) db_mc_getDefault(MCONTACT hMetaContact);

// returns the default contact number, or -1 on failure
MIR_APP_DLL(int) db_mc_getDefaultNum(MCONTACT hMetaContact);

// returns the number of subcontacts, or -1 on failure
MIR_APP_DLL(int) db_mc_getSubCount(MCONTACT hMetaContact);

// returns parent hContact for a subcontact or NULL if it's not a sub
MIR_APP_DLL(MCONTACT) db_mc_getMeta(MCONTACT hSubContact);

// returns parent hContact for a subcontact or hContact itself if it's not a sub
MIR_APP_DLL(MCONTACT) db_mc_tryMeta(MCONTACT hContact);

// returns a subcontact with the given index
MIR_APP_DLL(MCONTACT) db_mc_getSub(MCONTACT hMetaContact, int iNum);

// returns the default sub a SRMM window
MIR_APP_DLL(MCONTACT) db_mc_getSrmmSub(MCONTACT hContact);

// gets the handle for the 'most online' contact
MIR_APP_DLL(MCONTACT) db_mc_getMostOnline(MCONTACT hContact);

// sets the default contact, using the subcontact's handle
MIR_APP_DLL(int) db_mc_setDefault(MCONTACT hMetaContact, MCONTACT hSub, BOOL bWriteDb);

// sets the default contact, using the subcontact's number
MIR_APP_DLL(int) db_mc_setDefaultNum(MCONTACT hMetaContact, int iNum, BOOL bWriteDb);

// converts a contact into a meta
MIR_APP_DLL(MCONTACT) db_mc_convertToMeta(MCONTACT hContact);

// adds a contact to meta
MIR_APP_DLL(int) db_mc_addToMeta(MCONTACT hSub, MCONTACT hMetaContact);

// removes a contact to meta
MIR_APP_DLL(int) db_mc_removeFromMeta(MCONTACT hSub);

#if defined(__cplusplus)
}
#endif

#endif
