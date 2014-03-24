/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014 Miranda NG Team
Copyright © 2004-07 Scott Ellis
Copyright © 2004 Universite Louis PASTEUR, STRASBOURG.

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

#include "..\..\core\commonheaders.h"

#include "metacontacts.h"

// gets the handle for the 'most online' contact
// wParam=(MCONTACT)hMetaContact
// lParam=0
// returns a handle to the 'most online' contact

static INT_PTR MetaAPI_GetMostOnline(WPARAM hMetaContact, LPARAM)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == NULL) ? NULL : Meta_GetMostOnline(cc);
}

// wParam=(HANDLE)hContact
// lParam=0
// convert a given contact into a metacontact

static INT_PTR MetaAPI_ConvertToMeta(WPARAM wParam, LPARAM lParam)
{
	return Meta_Convert(wParam, lParam);
}

// wParam=(HANDLE)hContact
// lParam=(HANDLE)hMeta
// add an existing contact to a metacontact

static INT_PTR MetaAPI_AddToMeta(WPARAM wParam, LPARAM lParam)
{
	return Meta_Assign(wParam, lParam, FALSE);
}

// wParam=0
// lParam=(HANDLE)hContact
// remove a contact from a metacontact

static INT_PTR MetaAPI_RemoveFromMeta(WPARAM wParam, LPARAM lParam)
{
	// notice we switch args - to keep the API function consistent with the others
	return Meta_Delete(lParam, wParam);
}

void CreateApiServices()
{
	CreateServiceFunction(MS_MC_GETMOSTONLINECONTACT, MetaAPI_GetMostOnline);
	CreateServiceFunction(MS_MC_CONVERTTOMETA, MetaAPI_ConvertToMeta);
	CreateServiceFunction(MS_MC_ADDTOMETA, MetaAPI_AddToMeta);
	CreateServiceFunction(MS_MC_REMOVEFROMMETA, MetaAPI_RemoveFromMeta);
}
