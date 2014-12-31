/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include "clc.h"

static LIST<ClcCacheEntry> clistCache(50, NumericKeySortT);

void FreeDisplayNameCache(void)
{
	for (int i=0; i < clistCache.getCount(); i++) {
		cli.pfnFreeCacheItem(clistCache[i]);
		mir_free(clistCache[i]);
	}

	clistCache.destroy();
}

// default handlers for the cache item creation and destruction

ClcCacheEntry* fnCreateCacheItem(MCONTACT hContact)
{
	ClcCacheEntry* p = (ClcCacheEntry*)mir_calloc(sizeof(ClcCacheEntry));
	if (p == NULL)
		return NULL;

	p->hContact = hContact;
	return p;
}

void fnCheckCacheItem(ClcCacheEntry *p)
{
	DBVARIANT dbv;
	if (p->tszGroup == NULL) {
		if (!db_get_ts(p->hContact, "CList", "Group", &dbv)) {
			p->tszGroup = mir_tstrdup(dbv.ptszVal);
			mir_free(dbv.ptszVal);
		}
		else p->tszGroup = mir_tstrdup(_T(""));
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
		if (cacheEntry->tszName)
			return cacheEntry->tszName;
	}

	CONTACTINFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	if (ci.hContact == NULL)
		ci.szProto = "ICQ";
	ci.dwFlag = ((mode == GCDNF_NOMYHANDLE) ? CNF_DISPLAYNC : CNF_DISPLAY) | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (cacheEntry != NULL)
				cacheEntry->tszName = ci.pszVal;
			return ci.pszVal;
		}

		if (ci.type == CNFT_DWORD) {
			TCHAR *buffer = (TCHAR*) mir_alloc(15 * sizeof(TCHAR));
			_ltot(ci.dVal, buffer, 10);
			if (cacheEntry != NULL)
				cacheEntry->tszName = buffer;
			return buffer;
		}
	}

	CallContactService(hContact, PSS_GETINFO, SGIF_MINIMAL, 0);
	TCHAR *buffer = TranslateT("(Unknown contact)");
	return (cacheEntry == NULL) ? mir_tstrdup(buffer) : buffer;
}

INT_PTR GetContactDisplayName(WPARAM hContact, LPARAM lParam)
{
	static char retVal[200];
	ClcCacheEntry *cacheEntry = NULL;

	if (lParam & GCDNF_UNICODE)
		return (INT_PTR)cli.pfnGetContactDisplayName(hContact, lParam & ~GCDNF_UNICODE);

	if (lParam & GCDNF_NOCACHE)
		lParam &= ~GCDNF_NOCACHE;
	else if (lParam != GCDNF_NOMYHANDLE) {
		cacheEntry = cli.pfnGetCacheEntry(hContact);
		if (cacheEntry->tszName) {
			strncpy(retVal, _T2A(cacheEntry->tszName), SIZEOF(retVal));
			return (INT_PTR)retVal;
		}
	}

	CONTACTINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	if (ci.hContact == NULL) // killme !!!!!!!!!!
		ci.szProto = "ICQ";
	ci.dwFlag = ((lParam == GCDNF_NOMYHANDLE) ? CNF_DISPLAYNC : CNF_DISPLAY) | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			strncpy(retVal, _T2A(ci.pszVal), SIZEOF(retVal));
			if (cacheEntry == NULL) {
				mir_free(ci.pszVal);
				return (INT_PTR)mir_strdup(retVal);
			}

			cacheEntry->tszName = ci.pszVal;
			return (INT_PTR)retVal;
		}
		if (ci.type == CNFT_DWORD) {
			_ltoa(ci.dVal, retVal, 10);
			if (cacheEntry == NULL)
				return (INT_PTR)mir_strdup(retVal);

			cacheEntry->tszName = mir_a2u(retVal);
			return (INT_PTR)retVal;
		}
	}

	CallContactService(hContact, PSS_GETINFO, SGIF_MINIMAL, 0);
	char* result = Translate("(Unknown contact)");
	return (INT_PTR)((cacheEntry == NULL) ? mir_strdup(result) : result);
}

INT_PTR InvalidateDisplayName(WPARAM wParam, LPARAM)
{
	cli.pfnInvalidateDisplayNameCacheEntry(wParam);
	return 0;
}

int ContactAdded(WPARAM wParam, LPARAM)
{
	cli.pfnChangeContactIcon(wParam, cli.pfnIconFromStatusMode(GetContactProto(wParam), ID_STATUS_OFFLINE, NULL), 1);
	cli.pfnSortContacts();
	return 0;
}

int ContactDeleted(WPARAM wParam, LPARAM)
{
	CallService(MS_CLUI_CONTACTDELETED, wParam, 0);
	return 0;
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	DBVARIANT dbv;

	// Early exit
	if (hContact == NULL)
		return 0;

	dbv.pszVal = NULL;
	if (!db_get(hContact, "Protocol", "p", &dbv)) {
		if (!strcmp(cws->szModule, dbv.pszVal)) {
			cli.pfnInvalidateDisplayNameCacheEntry(hContact);
			if (!strcmp(cws->szSetting, "UIN") || !strcmp(cws->szSetting, "Nick") || !strcmp(cws->szSetting, "FirstName")
				 ||  !strcmp(cws->szSetting, "LastName") || !strcmp(cws->szSetting, "e-mail")) {
					CallService(MS_CLUI_CONTACTRENAMED, hContact, 0);
				}
			else if (!strcmp(cws->szSetting, "Status")) {
				if (!db_get_b(hContact, "CList", "Hidden", 0)) {
					if (db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT)) {
						// User's state is changing, and we are hideOffline-ing
						if (cws->value.wVal == ID_STATUS_OFFLINE) {
							cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(cws->szModule, cws->value.wVal, hContact), 0);
							CallService(MS_CLUI_CONTACTDELETED, hContact, 0);
							mir_free(dbv.pszVal);
							return 0;
						}
						cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(cws->szModule, cws->value.wVal, hContact), 1);
					}
					cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(cws->szModule, cws->value.wVal, hContact), 0);
				}
			}
			else {
				mir_free(dbv.pszVal);
				return 0;
			}
			cli.pfnSortContacts();
		}
	}

	if (!strcmp(cws->szModule, "CList")) {
		if (!strcmp(cws->szSetting, "Hidden")) {
			if (cws->value.type == DBVT_DELETED || cws->value.bVal == 0) {
				char *szProto = GetContactProto(hContact);
				cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(szProto, szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact), 1);
			}
			else
				CallService(MS_CLUI_CONTACTDELETED, hContact, 0);
		}
		if (!strcmp(cws->szSetting, "MyHandle"))
			cli.pfnInvalidateDisplayNameCacheEntry(hContact);
	}

	if (!strcmp(cws->szModule, "Protocol")) {
		if (!strcmp(cws->szSetting, "p")) {
			char *szProto;
			if (cws->value.type == DBVT_DELETED)
				szProto = NULL;
			else
				szProto = cws->value.pszVal;
			cli.pfnChangeContactIcon(hContact,
				cli.pfnIconFromStatusMode(szProto,
					szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status",
					ID_STATUS_OFFLINE), hContact), 0);
		}
	}

	// Clean up
	if (dbv.pszVal)
		mir_free(dbv.pszVal);

	return 0;
}
