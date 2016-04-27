/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org),
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
	ClcCacheEntry *p = (ClcCacheEntry*)mir_calloc(sizeof(ClcCacheEntry));
	if (p == NULL)
		return NULL;

	p->hContact = hContact;
	return p;
}

void fnCheckCacheItem(ClcCacheEntry *p)
{
	if (p->tszGroup == NULL) {
		p->tszGroup = db_get_tsa(p->hContact, "CList", "Group");
		if (p->tszGroup == NULL)
			p->tszGroup = mir_tstrdup(_T(""));
	}

	if (p->bIsHidden == -1)
		p->bIsHidden = db_get_b(p->hContact, "CList", "Hidden", 0);
}

void fnFreeCacheItem(ClcCacheEntry *p)
{
	if (p->tszName) { mir_free(p->tszName); p->tszName = NULL; }
	if (p->tszGroup) { mir_free(p->tszGroup); p->tszGroup = NULL; }
	p->bIsHidden = -1;
}

ClcCacheEntry* fnGetCacheEntry(MCONTACT hContact)
{
	ClcCacheEntry *p;
	int idx = clistCache.getIndex((ClcCacheEntry*)&hContact);
	if (idx == -1) {
		if ((p = cli.pfnCreateCacheItem(hContact)) != NULL) {
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

TCHAR* fnGetContactDisplayName(MCONTACT hContact, int mode)
{
	ClcCacheEntry *cacheEntry = NULL;

	if (mode & GCDNF_NOCACHE)
		mode &= ~GCDNF_NOCACHE;
	else if (mode != GCDNF_NOMYHANDLE) {
		cacheEntry = cli.pfnGetCacheEntry(hContact);
		if (cacheEntry && cacheEntry->tszName)
			return cacheEntry->tszName;
	}

	CONTACTINFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	if (ci.hContact == NULL)
		ci.szProto = "ICQ";
	ci.dwFlag = ((mode == GCDNF_NOMYHANDLE) ? CNF_DISPLAYNC : CNF_DISPLAY) | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (cacheEntry != NULL)
				replaceStrT(cacheEntry->tszName, ci.pszVal);
			return ci.pszVal;
		}

		if (ci.type == CNFT_DWORD) {
			TCHAR *buffer = (TCHAR*)mir_alloc(15 * sizeof(TCHAR));
			_ltot(ci.dVal, buffer, 10);
			if (cacheEntry != NULL)
				replaceStrT(cacheEntry->tszName, buffer);
			return buffer;
		}
	}

	CallContactService(hContact, PSS_GETINFO, SGIF_MINIMAL, 0);
	TCHAR *buffer = TranslateT("(Unknown contact)");
	return (cacheEntry == NULL) ? mir_tstrdup(buffer) : buffer;
}

int ContactAdded(WPARAM hContact, LPARAM)
{
	cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(GetContactProto(hContact), ID_STATUS_OFFLINE, NULL));
	return 0;
}

int ContactDeleted(WPARAM hContact, LPARAM)
{
	int idx = clistCache.getIndex((ClcCacheEntry*)&hContact);
	if (idx != -1) {
		cli.pfnFreeCacheItem(clistCache[idx]);
		mir_free(clistCache[idx]);
		clistCache.remove(idx);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void Dbwcs2tstr(DBCONTACTWRITESETTING *cws, TCHAR* &pStr)
{
	mir_free(pStr);

	switch (cws->value.type) {
	case -1:
	case DBVT_DELETED:
		pStr = NULL;
		break;

	case DBVT_UTF8:
		pStr = mir_utf8decodeT(cws->value.pszVal);
		break;

	case DBVT_ASCIIZ:
		pStr = mir_a2t(cws->value.pszVal);
		break;

	case DBVT_WCHAR:
		pStr = mir_u2t(cws->value.ptszVal);
		break;
	}
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	// Early exit
	if (hContact == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	char *szProto = GetContactProto(hContact);
	if (!mir_strcmp(cws->szModule, szProto)) {
		if (!strcmp(cws->szSetting, "UIN") || !strcmp(cws->szSetting, "Nick") || !strcmp(cws->szSetting, "FirstName") || !strcmp(cws->szSetting, "LastName") || !strcmp(cws->szSetting, "e-mail")) {
			ClcCacheEntry *pdnce = cli.pfnGetCacheEntry(hContact);
			replaceStrT(pdnce->tszName, NULL);
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
				cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(szProto, szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact));
		}
		else if (!strcmp(cws->szSetting, "MyHandle")) {
			ClcCacheEntry *pdnce = cli.pfnGetCacheEntry(hContact);
			replaceStrT(pdnce->tszName, NULL);
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
				szProto = NULL;
			else
				szProto = cws->value.pszVal;
			cli.pfnChangeContactIcon(hContact,
				cli.pfnIconFromStatusMode(szProto, szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact));
		}
	}
	return 0;
}
