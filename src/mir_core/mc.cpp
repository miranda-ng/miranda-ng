/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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
*/

#include "commonheaders.h"

static HANDLE hEventDefaultChanged;
static bool g_bEnabled;

void InitMetaContacts()
{
	hEventDefaultChanged = CreateHookableEvent(ME_MC_DEFAULTTCHANGED);
}

DBCachedContact* CheckMeta(MCONTACT hMeta)
{
	if (!g_bEnabled)
		return NULL;

	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hMeta);
	return (cc == NULL || cc->nSubs == -1) ? NULL : cc;
}

int Meta_GetContactNumber(DBCachedContact *cc, MCONTACT hContact)
{
	if (g_bEnabled)
		for (int i = 0; i < cc->nSubs; i++)
			if (cc->pSubs[i] == hContact)
				return i;

	return -1;
}

MCONTACT Meta_GetContactHandle(DBCachedContact *cc, int contact_number)
{
	if (contact_number >= cc->nSubs || contact_number < 0 || !g_bEnabled)
		return 0;

	return cc->pSubs[contact_number];
}

/////////////////////////////////////////////////////////////////////////////////////////
// metacontacts

MIR_CORE_DLL(BOOL) db_mc_isEnabled(void)
{
	return g_bEnabled;
}

MIR_CORE_DLL(void) db_mc_enable(BOOL bEnabled)
{
	g_bEnabled = bEnabled != 0;
}

MIR_CORE_DLL(BOOL) db_mc_isMeta(MCONTACT hContact)
{
	if (currDb == NULL || !g_bEnabled) return FALSE;

	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hContact);
	return (cc == NULL) ? FALSE : cc->nSubs != -1;
}

MIR_CORE_DLL(BOOL) db_mc_isSub(MCONTACT hContact)
{
	if (currDb == NULL || !g_bEnabled) return FALSE;

	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hContact);
	return (cc == NULL) ? FALSE : cc->parentID != 0;
}

//returns a handle to the default contact, or null on failure
MIR_CORE_DLL(MCONTACT) db_mc_getDefault(MCONTACT hMetaContact)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == NULL)
		return 0;

	return (cc->nDefault != -1) ? Meta_GetContactHandle(cc, cc->nDefault) : 0;
}

//returns the default contact number, or -1 on failure
MIR_CORE_DLL(int) db_mc_getDefaultNum(MCONTACT hMetaContact)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == NULL) ? -1 : cc->nDefault;
}

//returns the number of subcontacts, or -1 on failure
MIR_CORE_DLL(int) db_mc_getSubCount(MCONTACT hMetaContact)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == NULL) ? -1 : cc->nSubs;
}

// returns parent hContact for a subcontact or NULL if it's not a sub
MIR_CORE_DLL(MCONTACT) db_mc_getMeta(MCONTACT hSubContact)
{
	if (currDb == NULL) return false;

	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hSubContact);
	return (cc == NULL) ? NULL : cc->parentID;
}

// returns parent hContact for a subcontact or hContact itself if it's not a sub
MIR_CORE_DLL(MCONTACT) db_mc_tryMeta(MCONTACT hContact)
{
	if (currDb == NULL) return hContact;

	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hContact);
	if (cc == NULL) return hContact;

	return (cc->IsSub()) ? cc->parentID : hContact;
}

// returns a subcontact with the given index
MIR_CORE_DLL(MCONTACT) db_mc_getSub(MCONTACT hMetaContact, int iNum)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == NULL) ? 0 : Meta_GetContactHandle(cc, iNum);
}

//sets the default contact, using the subcontact's handle
MIR_CORE_DLL(int) db_mc_setDefault(MCONTACT hMetaContact, MCONTACT hSub, BOOL bWriteDb)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == NULL)
		return 1;

	int contact_number = Meta_GetContactNumber(cc, hSub);
	if (contact_number == -1) 
		return 1;

	cc->nDefault = contact_number;
	if (bWriteDb)
		currDb->MetaSetDefault(cc);
	
	NotifyEventHooks(hEventDefaultChanged, hMetaContact, hSub);
	return 0;
}

//sets the default contact, using the subcontact's number
MIR_CORE_DLL(int) db_mc_setDefaultNum(MCONTACT hMetaContact, int iNum, BOOL bWriteDb)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == NULL)
		return 1;
	if (iNum >= cc->nSubs || iNum < 0)
		return 1;

	cc->nDefault = iNum;
	if (bWriteDb)
		currDb->MetaSetDefault(cc);

	NotifyEventHooks(hEventDefaultChanged, hMetaContact, Meta_GetContactHandle(cc, iNum));
	return 0;
}

extern "C" MIR_CORE_DLL(void) db_mc_notifyDefChange(WPARAM wParam, LPARAM lParam)
{
	NotifyEventHooks(hEventDefaultChanged, wParam, lParam);
}