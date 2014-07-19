/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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
#define META_PROTO "MetaContacts"

//gets the handle for the 'most online' contact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns a handle to the 'most online' contact
#define MS_MC_GETMOSTONLINECONTACT "MetaContacts/GetMostOnline"

// fired when a metacontact's default contact changes (fired upon creation of metacontact also, when default is initially set)
// wParam=(HANDLE)hMetaContact
// lParam=(HANDLE)hDefaultContact
#define ME_MC_DEFAULTTCHANGED "MetaContacts/DefaultChanged"

// fired when a metacontact's subcontacts change (fired upon creation of metacontact, when contacts are added or removed, and when
//	contacts are reordered) - a signal to re-read metacontact data
// wParam=(HANDLE)hMetaContact
// lParam=0
#define ME_MC_SUBCONTACTSCHANGED "MetaContacts/SubcontactsChanged"

// wParam=(HANDLE)hContact
// lParam=0
// convert a given contact into a metacontact
#define MS_MC_CONVERTTOMETA "MetaContacts/ConvertToMetacontact"

// wParam=(HANDLE)hContact
// lParam=(HANDLE)hMeta
// add an existing contact to a metacontact
#define MS_MC_ADDTOMETA "MetaContacts/AddToMetacontact"

// wParam=0
// lParam=(HANDLE)hContact
// remove a contact from a metacontact
#define MS_MC_REMOVEFROMMETA "MetaContacts/RemoveFromMetacontact"

// wParam=(HANDLE)hContact
// lParam=(HANDLE)hMeta
// add an existing contact to a metacontact
#define MS_MC_GETSRMMSUB "MetaContacts/GetSrmmSub"

/////////////////////////////////////////////////////////////////////////////////////////
// binary interface to MC

#if defined(__cplusplus)
extern "C"
{
#endif

MCONTACT __forceinline db_mc_getMostOnline(MCONTACT hContact)
{	return CallService(MS_MC_GETMOSTONLINECONTACT, hContact, 0);
}

MCONTACT __forceinline db_mc_getSrmmSub(MCONTACT hContact)
{	return CallService(MS_MC_GETSRMMSUB, hContact, 0);
}

// checks whether metacontacts are enabled
MIR_CORE_DLL(BOOL) db_mc_isEnabled(void);
MIR_CORE_DLL(void) db_mc_enable(BOOL);

// checks whether a contact is a metacontact
MIR_CORE_DLL(BOOL) db_mc_isMeta(MCONTACT hMetaContact);

// checks whether a contact is a subcontact of existing MC
MIR_CORE_DLL(BOOL) db_mc_isSub(MCONTACT hSubContact);

//returns a handle to the default contact, or null on failure
MIR_CORE_DLL(MCONTACT) db_mc_getDefault(MCONTACT hMetaContact);

//returns the default contact number, or -1 on failure
MIR_CORE_DLL(int) db_mc_getDefaultNum(MCONTACT hMetaContact);

//returns the number of subcontacts, or -1 on failure
MIR_CORE_DLL(int) db_mc_getSubCount(MCONTACT hMetaContact);

// returns parent hContact for a subcontact or NULL if it's not a sub
MIR_CORE_DLL(MCONTACT) db_mc_getMeta(MCONTACT hSubContact);

// returns parent hContact for a subcontact or hContact itself if it's not a sub
MIR_CORE_DLL(MCONTACT) db_mc_tryMeta(MCONTACT hContact);

// returns a subcontact with the given index
MIR_CORE_DLL(MCONTACT) db_mc_getSub(MCONTACT hMetaContact, int iNum);

//sets the default contact, using the subcontact's handle
MIR_CORE_DLL(int) db_mc_setDefault(MCONTACT hMetaContact, MCONTACT hSub, BOOL bWriteDb);

//sets the default contact, using the subcontact's number
MIR_CORE_DLL(int) db_mc_setDefaultNum(MCONTACT hMetaContact, int iNum, BOOL bWriteDb);

#if defined(__cplusplus)
}
#endif

#endif
