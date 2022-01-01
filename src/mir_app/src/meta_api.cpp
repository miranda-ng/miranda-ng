/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014-22 Miranda NG team
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

#include "stdafx.h"

#include "metacontacts.h"

MIR_APP_DLL(BOOL) db_mc_isEnabled(void)
{
	return g_bMetaEnabled;
}

MIR_APP_DLL(void) db_mc_enable(BOOL bEnabled)
{
	g_bMetaEnabled = bEnabled != 0;

	NotifyEventHooks(hEventEnabled, g_bMetaEnabled, 0);
}

MIR_APP_DLL(BOOL) db_mc_isMeta(MCONTACT hContact)
{
	if (g_pCurrDb == nullptr || !g_bMetaEnabled)
		return FALSE;

	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hContact);
	return (cc == nullptr) ? FALSE : cc->nSubs != -1;
}

MIR_APP_DLL(BOOL) db_mc_isSub(MCONTACT hContact)
{
	if (g_pCurrDb == nullptr || !g_bMetaEnabled)
		return FALSE;

	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hContact);
	return (cc == nullptr) ? FALSE : cc->parentID != 0;
}

// returns a handle to the default contact, or null on failure
MIR_APP_DLL(MCONTACT) db_mc_getDefault(MCONTACT hMetaContact)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == nullptr)
		return 0;

	return (cc->nDefault != -1) ? Meta_GetContactHandle(cc, cc->nDefault) : 0;
}

// returns the default contact number, or -1 on failure
MIR_APP_DLL(int) db_mc_getDefaultNum(MCONTACT hMetaContact)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == nullptr) ? -1 : cc->nDefault;
}

// returns the number of subcontacts, or -1 on failure
MIR_APP_DLL(int) db_mc_getSubCount(MCONTACT hMetaContact)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == nullptr) ? -1 : cc->nSubs;
}

// returns parent hContact for a subcontact or NULL if it's not a sub
MIR_APP_DLL(MCONTACT) db_mc_getMeta(MCONTACT hSubContact)
{
	if (g_pCurrDb == nullptr)
		return 0;

	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hSubContact);
	return (cc == nullptr) ? NULL : cc->parentID;
}

// returns parent hContact for a subcontact or hContact itself if it's not a sub
MIR_APP_DLL(MCONTACT) db_mc_tryMeta(MCONTACT hContact)
{
	if (g_pCurrDb == nullptr)
		return hContact;

	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hContact);
	if (cc == nullptr) return hContact;

	return (cc->IsSub()) ? cc->parentID : hContact;
}

// returns a subcontact with the given index
MIR_APP_DLL(MCONTACT) db_mc_getSub(MCONTACT hMetaContact, int iNum)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == nullptr) ? 0 : Meta_GetContactHandle(cc, iNum);
}

// gets the handle for the 'most online' contact
MIR_APP_DLL(MCONTACT) db_mc_getMostOnline(MCONTACT hMetaContact)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == nullptr) ? 0 : Meta_GetMostOnline(cc);
}

// returns manually chosen sub in the meta window
MIR_APP_DLL(MCONTACT) db_mc_getSrmmSub(MCONTACT hContact)
{
	MetaSrmmData tmp = { hContact };
	if (MetaSrmmData *p = arMetaWindows.find(&tmp))
		return p->m_hSub;

	return db_mc_getMostOnline(hContact);
}

// converts a given contact into a metacontact
MIR_APP_DLL(MCONTACT) db_mc_convertToMeta(MCONTACT hContact)
{
	return Meta_Convert(hContact, 0);
}

// adds an existing contact to a metacontact
MIR_APP_DLL(int) db_mc_addToMeta(MCONTACT hSub, MCONTACT hMetaContact)
{
	return Meta_Assign(hSub, hMetaContact, false);
}

// removes a contact from a metacontact
MIR_APP_DLL(int) db_mc_removeFromMeta(MCONTACT hSub)
{
	// notice we switch args - to keep the API function consistent with the others
	return Meta_Delete(hSub, true);
}

// sets the default contact, using the subcontact's handle
MIR_APP_DLL(int) db_mc_setDefault(MCONTACT hMetaContact, MCONTACT hSub, BOOL bWriteDb)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == nullptr)
		return 1;

	int contact_number = Meta_GetContactNumber(cc, hSub);
	if (contact_number == -1) 
		return 1;

	if (cc->nDefault != contact_number) {
		cc->nDefault = contact_number;
		if (bWriteDb)
			g_pCurrDb->MetaSetDefault(cc);

		NotifyEventHooks(hEventDefaultChanged, hMetaContact, hSub);
	}
	return 0;
}

// sets the default contact, using the subcontact's number
MIR_APP_DLL(int) db_mc_setDefaultNum(MCONTACT hMetaContact, int iNum, BOOL bWriteDb)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == nullptr)
		return 1;
	if (iNum >= cc->nSubs || iNum < 0)
		return 1;

	if (cc->nDefault != iNum) {
		cc->nDefault = iNum;
		if (bWriteDb)
			g_pCurrDb->MetaSetDefault(cc);

		NotifyEventHooks(hEventDefaultChanged, hMetaContact, Meta_GetContactHandle(cc, iNum));
	}
	return 0;
}
