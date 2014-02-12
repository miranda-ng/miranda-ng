// Copyright © 2010-2012 SecureIM developers (baloo and others), sss
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "commonheaders.h"

extern bool bMetaContacts;

bool metaIsProtoMetaContacts(MCONTACT hContact)
{
	if(bMetaContacts) {
		LPSTR proto = GetContactProto(hContact);
		if( proto && strcmp(proto,"MetaContacts")==0 ) {
			return true;
		}
	}
	return false;
}

bool metaIsDefaultSubContact(MCONTACT hContact) 
{
	if(bMetaContacts)
		return (MCONTACT)CallService(MS_MC_GETDEFAULTCONTACT,(WPARAM)CallService(MS_MC_GETMETACONTACT,hContact,0),0)==hContact;
	return false;
}

MCONTACT metaGetContact(MCONTACT hContact) 
{
	if(bMetaContacts)
		if(metaIsSubcontact(hContact))
			return (MCONTACT)CallService(MS_MC_GETMETACONTACT,hContact,0);
	return NULL;
}

bool metaIsSubcontact(MCONTACT hContact)
{
	if(bMetaContacts)
		return CallService(MS_MC_GETMETACONTACT,hContact,0) != 0;
	return false;
}

MCONTACT metaGetMostOnline(MCONTACT hContact) 
{
	if(bMetaContacts)
		if(metaIsProtoMetaContacts(hContact))
			return (MCONTACT)CallService(MS_MC_GETMOSTONLINECONTACT,hContact,0);
	return NULL;
}

MCONTACT metaGetDefault(MCONTACT hContact) 
{
	if(bMetaContacts)
		if(metaIsProtoMetaContacts(hContact))
			return (MCONTACT)CallService(MS_MC_GETDEFAULTCONTACT,hContact,0);
	return NULL;
}

DWORD metaGetContactsNum(MCONTACT hContact)
{
	if(bMetaContacts)
		return CallService(MS_MC_GETNUMCONTACTS, hContact, 0);
	return 0;
}

MCONTACT metaGetSubcontact(MCONTACT hContact, int num)
{
	if(bMetaContacts)
		return (MCONTACT)CallService(MS_MC_GETSUBCONTACT, hContact, (LPARAM)num);
	return 0;
}
