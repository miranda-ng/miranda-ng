/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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

static LIST<ClcCacheEntry> clistCache(50, NumericKeySortT);

void FreeDisplayNameCache(void)
{
	for (int i = 0; i < clistCache.getCount(); i++) {
		cli.pfnFreeCacheItem(clistCache[i]);
		mir_free(clistCache[i]);
	}

	clistCache.destroy();
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
	return p;
}

void fnCheckCacheItem(ClcCacheEntry *p)
{
	if (p->tszGroup == nullptr) {
		p->tszGroup = db_get_wsa(p->hContact, "CList", "Group");
		if (p->tszGroup == nullptr)
			p->tszGroup = mir_wstrdup(L"");
	}

	if (p->bIsHidden == -1)
		p->bIsHidden = db_get_b(p->hContact, "CList", "Hidden", 0);
}

void fnFreeCacheItem(ClcCacheEntry *p)
{
	replaceStrW(p->tszName, nullptr);
	replaceStrW(p->tszGroup, nullptr);
	p->bIsHidden = -1;
}

ClcCacheEntry* fnGetCacheEntry(MCONTACT hContact)
{
	ClcCacheEntry *p;
	int idx = clistCache.getIndex((ClcCacheEntry*)&hContact);
	if (idx == -1) {
		if ((p = cli.pfnCreateCacheItem(hContact)) != nullptr) {
			clistCache.insert(p);
			cli.pfnInvalidateDisplayNameCacheEntry(hContact);
		}
	}
	else p = clistCache[idx];

	cli.pfnCheckCacheItem(p);
	return p;
}

void fnInvalidateDisplayNameCacheEntry(MCONTACT hContact)
{
	if (hContact == INVALID_CONTACT_ID) {
		FreeDisplayNameCache();
		cli.pfnInitAutoRebuild(cli.hwndContactTree);
	}
	else {
		int idx = clistCache.getIndex((ClcCacheEntry*)&hContact);
		if (idx != -1)
			cli.pfnFreeCacheItem(clistCache[idx]);
	}
}

wchar_t* fnGetContactDisplayName(MCONTACT hContact, int mode)
{
	if (hContact == 0)
		return TranslateT("(Unknown contact)");
	
	ClcCacheEntry *cacheEntry = nullptr;
	if (mode & GCDNF_NOCACHE)
		mode &= ~GCDNF_NOCACHE;
	else if (mode != GCDNF_NOMYHANDLE) {
		cacheEntry = cli.pfnGetCacheEntry(hContact);
		if (cacheEntry && cacheEntry->tszName)
			return cacheEntry->tszName;
	}

	ptrW tszDisplayName(Contact_GetInfo((mode == GCDNF_NOMYHANDLE) ? CNF_DISPLAYNC : CNF_DISPLAY, hContact));
	if (tszDisplayName != nullptr) {
		if (cacheEntry != nullptr)
			replaceStrW(cacheEntry->tszName, tszDisplayName);
		return tszDisplayName.detach();
	}

	ProtoChainSend(hContact, PSS_GETINFO, SGIF_MINIMAL, 0);

	wchar_t *buffer = TranslateT("(Unknown contact)");
	return (cacheEntry == nullptr) ? mir_wstrdup(buffer) : buffer;
}

int ContactAdded(WPARAM hContact, LPARAM)
{
	cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(GetContactProto(hContact), ID_STATUS_OFFLINE, 0));
	return 0;
}

int ContactDeleted(WPARAM hContact, LPARAM)
{
	Clist_Broadcast(INTM_CONTACTDELETED, hContact, 0);

	int idx = clistCache.getIndex((ClcCacheEntry*)&hContact);
	if (idx != -1) {
		cli.pfnFreeCacheItem(clistCache[idx]);
		mir_free(clistCache[idx]);
		clistCache.remove(idx);
	}
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
		pStr = mir_wstrdup(cws->value.ptszVal);
		break;
	}
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	// Early exit
	if (hContact == 0)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	char *szProto = GetContactProto(hContact);
	if (!mir_strcmp(cws->szModule, szProto)) {
		if (!strcmp(cws->szSetting, "UIN") || !strcmp(cws->szSetting, "Nick") || !strcmp(cws->szSetting, "FirstName") || !strcmp(cws->szSetting, "LastName") || !strcmp(cws->szSetting, "e-mail")) {
			ClcCacheEntry *pdnce = cli.pfnGetCacheEntry(hContact);
			replaceStrW(pdnce->tszName, nullptr);
			cli.pfnCheckCacheItem(pdnce);
		}
		else if (!strcmp(cws->szSetting, "Status")) {
			if (!db_get_b(hContact, "CList", "Hidden", 0))
				cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(cws->szModule, cws->value.wVal, hContact));
		}
	}
	else if (!strcmp(cws->szModule, "CList")) {
		if (!strcmp(cws->szSetting, "Hidden")) {
			if (cws->value.type == DBVT_DELETED || cws->value.bVal == 0)
				cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(szProto, szProto == nullptr ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact));
		}
		else if (!strcmp(cws->szSetting, "MyHandle")) {
			ClcCacheEntry *pdnce = cli.pfnGetCacheEntry(hContact);
			replaceStrW(pdnce->tszName, nullptr);
			cli.pfnCheckCacheItem(pdnce);
		}
		else if (!strcmp(cws->szSetting, "Group")) {
			ClcCacheEntry *pdnce = cli.pfnGetCacheEntry(hContact);
			Dbwcs2tstr(cws, pdnce->tszGroup);
		}
	}
	else if (!strcmp(cws->szModule, "Protocol")) {
		if (!strcmp(cws->szSetting, "p")) {
			if (cws->value.type == DBVT_DELETED)
				szProto = nullptr;
			else
				szProto = cws->value.pszVal;
			cli.pfnChangeContactIcon(hContact,
				cli.pfnIconFromStatusMode(szProto, szProto == nullptr ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact));
		}
	}
	return 0;
}
