/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
#include "clc.h"
#include "chat.h"

static LIST<ClcCacheEntry> clistCache(50, NumericKeySortT);

void FreeDisplayNameCache(void)
{
	for (auto &it : clistCache) {
		g_clistApi.pfnFreeCacheItem(it);
		mir_free(it);
	}

	clistCache.destroy();
}

void InvalidateProtoInCache(const char *szModuleName)
{
	for (auto &it : clistCache)
		if (!mir_strcmp(it->szProto, szModuleName))
			it->szProto = nullptr;
}

// default handlers for the cache item creation and destruction

ClcCacheEntry* fnCreateCacheItem(MCONTACT hContact)
{
	if (hContact == 0)
		return nullptr;

	ClcCacheEntry *p = (ClcCacheEntry*)mir_calloc(sizeof(ClcCacheEntry));
	if (p == nullptr)
		return nullptr;

	p->hContact = hContact;
	p->szProto = Proto_GetBaseAccountName(hContact);
	return p;
}

void fnCheckCacheItem(ClcCacheEntry *p)
{
	if (p->tszGroup == nullptr)
		p->tszGroup = Clist_GetGroup(p->hContact);

	if (p->szProto == nullptr)
		p->szProto = Proto_GetBaseAccountName(p->hContact);

	if (p->bIsHidden == -1)
		p->bIsHidden = Contact::IsHidden(p->hContact);
}

void fnFreeCacheItem(ClcCacheEntry *p)
{
	replaceStrW(p->tszName, nullptr);
	replaceStrW(p->tszGroup, nullptr);
	p->bIsHidden = -1;
}

MIR_APP_DLL(ClcCacheEntry*) Clist_GetCacheEntry(MCONTACT hContact)
{
	ClcCacheEntry *p;
	int idx = clistCache.getIndex((ClcCacheEntry*)&hContact);
	if (idx == -1) {
		p = g_clistApi.pfnCreateCacheItem(hContact);
		if (p == nullptr)
			return nullptr;

		clistCache.insert(p);
		g_clistApi.pfnInvalidateDisplayNameCacheEntry(hContact);
	}
	else p = clistCache[idx];

	g_clistApi.pfnCheckCacheItem(p);
	return p;
}

void fnInvalidateDisplayNameCacheEntry(MCONTACT hContact)
{
	if (hContact == INVALID_CONTACT_ID) {
		FreeDisplayNameCache();
		Clist_InitAutoRebuild(g_clistApi.hwndContactTree);
	}
	else {
		int idx = clistCache.getIndex((ClcCacheEntry*)&hContact);
		if (idx != -1)
			g_clistApi.pfnFreeCacheItem(clistCache[idx]);
	}
}

MIR_APP_DLL(wchar_t*) Clist_GetContactDisplayName(MCONTACT hContact, int mode)
{
	if (hContact == 0)
		return (mode & GCDNF_NOUNKNOWN) ? nullptr : TranslateT("(Unknown contact)");
	
	ClcCacheEntry *cacheEntry = nullptr;
	if (mode & GCDNF_NOCACHE)
		mode &= ~GCDNF_NOCACHE;
	else if (mode != GCDNF_NOMYHANDLE) {
		cacheEntry = Clist_GetCacheEntry(hContact);
		if (cacheEntry && cacheEntry->tszName)
			return cacheEntry->tszName;
	}

	ptrW tszDisplayName(Contact::GetInfo((mode == GCDNF_NOMYHANDLE) ? CNF_DISPLAYNC : CNF_DISPLAY, hContact));
	if (tszDisplayName != nullptr) {
		if (cacheEntry != nullptr)
			replaceStrW(cacheEntry->tszName, tszDisplayName);
		return tszDisplayName.detach();
	}

	if (mode & GCDNF_NOUNKNOWN)
		return nullptr;

	ProtoChainSend(hContact, PSS_GETINFO, SGIF_MINIMAL, 0);

	wchar_t *buffer = TranslateT("(Unknown contact)");
	return (cacheEntry == nullptr) ? mir_wstrdup(buffer) : buffer;
}

int ContactAdded(WPARAM hContact, LPARAM)
{
	Clist_ChangeContactIcon(hContact, g_clistApi.pfnIconFromStatusMode(Proto_GetBaseAccountName(hContact), ID_STATUS_OFFLINE, 0));
	return 0;
}

int ContactDeleted(WPARAM hContact, LPARAM)
{
	Clist_Broadcast(INTM_CONTACTDELETED, hContact, 0);

	int idx = clistCache.getIndex((ClcCacheEntry*)&hContact);
	if (idx != -1) {
		g_clistApi.pfnFreeCacheItem(clistCache[idx]);
		mir_free(clistCache[idx]);
		clistCache.remove(idx);
	}

	// remove events for a contact
	for (auto &it : g_cliEvents.rev_iter())
		if (it->hContact == hContact)
			g_clistApi.pfnRemoveEvent(hContact, it->hDbEvent);

	Chat_RemoveContact(hContact);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void Dbwcs2tstr(DBCONTACTWRITESETTING *cws, wchar_t* &pStr)
{
	mir_free(pStr);

	switch (cws->value.type) {
	case -1:
	case DBVT_DELETED:
		pStr = nullptr;
		break;

	case DBVT_UTF8:
		pStr = mir_utf8decodeW(cws->value.pszVal);
		break;

	case DBVT_ASCIIZ:
		pStr = mir_a2u(cws->value.pszVal);
		break;

	case DBVT_WCHAR:
		pStr = mir_wstrdup(cws->value.pwszVal);
		break;
	}
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	// Early exit
	if (hContact == 0)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (!mir_strcmp(cws->szModule, szProto)) {
		if (!strcmp(cws->szSetting, "UIN") || !strcmp(cws->szSetting, "Nick") || !strcmp(cws->szSetting, "FirstName") || !strcmp(cws->szSetting, "LastName") || !strcmp(cws->szSetting, "e-mail")) {
			ClcCacheEntry *pdnce = Clist_GetCacheEntry(hContact);
			replaceStrW(pdnce->tszName, nullptr);
			g_clistApi.pfnCheckCacheItem(pdnce);
		}
		else if (!strcmp(cws->szSetting, "Status")) {
			if (!Contact::IsHidden(hContact))
				Clist_ChangeContactIcon(hContact, g_clistApi.pfnIconFromStatusMode(cws->szModule, cws->value.wVal, hContact));
		}
	}
	else if (!strcmp(cws->szModule, "CList")) {
		if (!strcmp(cws->szSetting, "Hidden")) {
			if (cws->value.type == DBVT_DELETED || cws->value.bVal == 0)
				Clist_ChangeContactIcon(hContact, g_clistApi.pfnIconFromStatusMode(szProto, szProto == nullptr ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact));
		}
		else if (!strcmp(cws->szSetting, "MyHandle")) {
			ClcCacheEntry *pdnce = Clist_GetCacheEntry(hContact);
			replaceStrW(pdnce->tszName, nullptr);
			g_clistApi.pfnCheckCacheItem(pdnce);
		}
		else if (!strcmp(cws->szSetting, "Group")) {
			ClcCacheEntry *pdnce = Clist_GetCacheEntry(hContact);
			Dbwcs2tstr(cws, pdnce->tszGroup);
		}
	}
	else if (!strcmp(cws->szModule, "Protocol")) {
		if (!strcmp(cws->szSetting, "p")) {
			if (cws->value.type == DBVT_DELETED)
				szProto = nullptr;
			else
				szProto = cws->value.pszVal;
			Clist_ChangeContactIcon(hContact,
				g_clistApi.pfnIconFromStatusMode(szProto, szProto == nullptr ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact));
		}
	}
	return 0;
}
