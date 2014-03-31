/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "variables.h"

struct _tagType
{
	int    cnfCode;
	TCHAR* str;
}
static builtinCnfs[] =
{
	{ CNF_FIRSTNAME,   STR_FIRSTNAME  }, 
	{ CNF_LASTNAME,    STR_LASTNAME   },
	{ CNF_NICK,        STR_NICK       },
	{ CNF_CUSTOMNICK,  STR_CUSTOMNICK },
	{ CNF_EMAIL,       STR_EMAIL      },
	{ CNF_CITY,        STR_CITY       },
	{ CNF_STATE,       STR_STATE      },
	{ CNF_COUNTRY,     STR_COUNTRY    },
	{ CNF_PHONE,       STR_PHONE      },
	{ CNF_HOMEPAGE,    STR_HOMEPAGE   },
	{ CNF_ABOUT,       STR_ABOUT      },
	{ CNF_GENDER,      STR_GENDER     },
	{ CNF_AGE,         STR_AGE        },
	{ CNF_FIRSTLAST,   STR_FIRSTLAST  },
	{ CNF_UNIQUEID,    STR_UNIQUEID   },
	{ CNF_DISPLAY,     STR_DISPLAY    },
	{ CNF_FAX,         STR_FAX        },
	{ CNF_CELLULAR,    STR_CELLULAR   },
	{ CNF_TIMEZONE,    STR_TIMEZONE   },
	{ CNF_MYNOTES,     STR_MYNOTES    },
	{ CNF_BIRTHDAY,    STR_BIRTHDAY   },
	{ CNF_BIRTHMONTH,  STR_BIRTHMONTH },
	{ CNF_BIRTHYEAR,   STR_BIRTHYEAR  },
	{ CNF_STREET,      STR_STREET     },
	{ CNF_ZIP,         STR_ZIP        },
	{ CNF_LANGUAGE1,   STR_LANGUAGE1  },
	{ CNF_LANGUAGE2,   STR_LANGUAGE2  },
	{ CNF_LANGUAGE3,   STR_LANGUAGE3  },
	{ CNF_CONAME,      STR_CONAME     },
	{ CNF_CODEPT,      STR_CODEPT     },
	{ CNF_COPOSITION,  STR_COPOSITION },
	{ CNF_COSTREET,    STR_COSTREET   },
	{ CNF_COCITY,      STR_COCITY     },
	{ CNF_COSTATE,     STR_COSTATE    },
	{ CNF_COZIP,       STR_COZIP      },
	{ CNF_COCOUNTRY,   STR_COCOUNTRY  },
	{ CNF_COHOMEPAGE,  STR_COHOMEPAGE },

	{ CCNF_ACCOUNT,    STR_ACCOUNT    },
	{ CCNF_PROTOCOL,   STR_PROTOCOL   },
	{ CCNF_STATUS,     STR_STATUS     },
	{ CCNF_INTERNALIP, STR_INTERNALIP },
	{ CCNF_EXTERNALIP, STR_EXTERNALIP },
	{ CCNF_GROUP,      STR_GROUP      },
	{ CCNF_PROTOID,    STR_PROTOID    }
};

typedef struct {
	TCHAR* tszContact;
	MCONTACT hContact;
	DWORD flags;
} CONTACTCE; /* contact cache entry */

/* cache for 'getcontactfromstring' service */
static CONTACTCE *cce = NULL;
static int cacheSize = 0;
static CRITICAL_SECTION csContactCache;

static HANDLE hContactSettingChangedHook;

/*
	converts a string into a CNF_ type
*/
BYTE getContactInfoType(TCHAR* type)
{
	if (type == NULL || _tcslen(type) == 0 )
		return 0;

	for (int i=0; i < SIZEOF(builtinCnfs); i++ )
		if (!_tcscmp( builtinCnfs[i].str, type ))
			return builtinCnfs[i].cnfCode;

	return 0;
}

/*
	returns info about a contact as a string
*/
TCHAR* getContactInfoT(BYTE type, MCONTACT hContact)
{
	/* returns dynamic allocated buffer with info, or NULL if failed */
	TCHAR *res = NULL;
	char protoname[128], szVal[16];
	PROTOACCOUNT *pa;

	if (hContact == NULL)
		return NULL;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return NULL;

	switch (type) {
	case CCNF_PROTOID:
		return mir_a2t(szProto);

	case CCNF_ACCOUNT:
		pa = ProtoGetAccount(szProto);
		return pa ? mir_tstrdup(pa->tszAccountName) : NULL;

	case CCNF_PROTOCOL:
		if (CallProtoService(szProto, PS_GETNAME, (WPARAM)sizeof(protoname), (LPARAM)protoname))
			return NULL;
		return mir_a2t(protoname);

	case CCNF_STATUS:
		return mir_tstrdup((TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), GSMDF_UNICODE));

	case CCNF_INTERNALIP:
	case CCNF_EXTERNALIP:
		{
			DWORD ip = db_get_dw(hContact, szProto, (type == CCNF_INTERNALIP) ? "RealIP" : "IP", 0);
			if (ip == 0)
				return NULL;

			struct in_addr in;
			in.s_addr = htonl(ip);
			return mir_a2t( inet_ntoa(in));
		}

	case CCNF_GROUP:
		if ((res = db_get_tsa(hContact, "CList", "Group")) != NULL)
			return res;
		break;

	case CNF_UNIQUEID:
		//UID for ChatRoom
		if (db_get_b(hContact, szProto, "ChatRoom", 0) == 1)
			if ((res = db_get_tsa(hContact, szProto, "ChatRoomID")) != NULL)
				return res;

		//UID for other contact
		break;
	}

	CONTACTINFO ci = { sizeof(ci) };
	ci.hContact = hContact;
	ci.dwFlag = type | CNF_UNICODE;
	CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci);

	memset(szVal, '\0', sizeof(szVal));
	switch(ci.type) {
	case CNFT_BYTE:
		if (type == CNF_GENDER) {
			szVal[0] = (char)ci.bVal; szVal[1] = 0;
			return mir_a2t(szVal);
		}
		return itot(ci.bVal);

	case CNFT_WORD:
		return itot(ci.wVal);

	case CNFT_DWORD:
		return itot(ci.dVal);

	case CNFT_ASCIIZ:
		return ci.pszVal;
	}

	return NULL;
}

/*
	MS_VARS_GETCONTACTFROMSTRING
*/
int getContactFromString(CONTACTSINFO *ci)
{
	/* service to retrieve a contact's HANDLE from a given string */
	if (ci == NULL)
		return -1;

	TCHAR *tszContact;
	if (ci->flags & CI_UNICODE)
		tszContact = NEWTSTR_ALLOCA(ci->tszContact);
	else {
		WCHAR* tmp = mir_a2t(ci->szContact);
		tszContact = NEWTSTR_ALLOCA(tmp);
		mir_free(tmp);
	}
	if ((tszContact == NULL) || (_tcslen(tszContact) == 0))
		return -1;

	ci->hContacts = NULL;
	int count = 0;
	/* search the cache */
	{
		mir_cslock lck(csContactCache);
		for (int i=0; i < cacheSize; i++) {
			if ((!_tcscmp(cce[i].tszContact, tszContact)) && (ci->flags == cce[i].flags)) {
				/* found in cache */
				ci->hContacts = (MCONTACT*)mir_alloc(sizeof(MCONTACT));
				if (ci->hContacts == NULL)
					return -1;

				ci->hContacts[0] = cce[i].hContact;
				return 1;
			}
		}
	}

	/* contact was not in cache, do a search */
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
	{
		TCHAR *szFind = NULL;
		BOOL bMatch = FALSE;

		// <_HANDLE_:hContact>
		size_t size = _tcslen(_T(PROTOID_HANDLE)) + 36;
		szFind = (TCHAR *)mir_alloc(size * sizeof(TCHAR));
		if (szFind != NULL)
		{
			mir_sntprintf(szFind, size, _T("<%s:%p>"), _T(PROTOID_HANDLE), hContact);
			if (!_tcsncmp(tszContact, szFind, _tcslen(tszContact)))
				bMatch = TRUE;

			mir_free(szFind);
		}

		char *szProto = GetContactProto(hContact);
		if (szProto == NULL)
			continue;

		// <proto:id> (exact)
		if ((ci->flags & CI_PROTOID) && !bMatch)
		{
			TCHAR *cInfo = getContactInfoT(CNF_UNIQUEID, hContact);
			if (cInfo)
			{
				size_t size = _tcslen(cInfo) + strlen(szProto) + 4;
				szFind = (TCHAR *)mir_alloc(size * sizeof(TCHAR));
				if (szFind != NULL) {
					mir_sntprintf(szFind, size, _T("<%S:%s>"), szProto, cInfo);
					mir_free(cInfo);
					if (!_tcsncmp(tszContact, szFind, _tcslen(tszContact)))
						bMatch = TRUE;
					mir_free(szFind);
				}
			}
		}
		// id (exact)
		if ((ci->flags & CI_UNIQUEID) && (!bMatch)) {
			szFind = getContactInfoT(CNF_UNIQUEID, hContact);
			if (szFind != NULL) {
				if (!_tcscmp(tszContact, szFind))
					bMatch = TRUE;
				mir_free(szFind);
			}
		}
		// nick (not exact)
		if ((ci->flags & CI_NICK) && (!bMatch)) {
			szFind = getContactInfoT(CNF_NICK, hContact);
			if (szFind != NULL) {
				if (!_tcscmp(tszContact, szFind))
					bMatch = TRUE;
				mir_free(szFind);
			}
		}
		// list name (not exact)
		if ((ci->flags & CI_LISTNAME) && (!bMatch)) {
			szFind = getContactInfoT(CNF_DISPLAY, hContact);
			if (szFind != NULL) {
				if (!_tcscmp(tszContact, szFind))
					bMatch = TRUE;
				mir_free(szFind);
			}
		}
		// firstname (exact)
		if ((ci->flags & CI_FIRSTNAME) && (!bMatch)) {
			szFind = getContactInfoT(CNF_FIRSTNAME, hContact);
			if (szFind != NULL) {
				if (!_tcscmp(tszContact, szFind))
					bMatch = TRUE;
				mir_free(szFind);
			}
		}
		// lastname (exact)
		if ((ci->flags & CI_LASTNAME) && (!bMatch)) {
			szFind = getContactInfoT(CNF_LASTNAME, hContact);
			if (szFind != NULL) {
				if (!_tcscmp(tszContact, szFind))
					bMatch = TRUE;
				mir_free(szFind);
			}
		}
		// email (exact)
		if ((ci->flags & CI_EMAIL) && (!bMatch)) {
			szFind = getContactInfoT(CNF_EMAIL, hContact);
			if (szFind != NULL) {
				if (!_tcscmp(tszContact, szFind))
					bMatch = TRUE;
				mir_free(szFind);
			}
		}
		// CNF_ (exact)
		if ((ci->flags & CI_CNFINFO) && (!bMatch)) {
			szFind = getContactInfoT((BYTE)(ci->flags&~(CI_CNFINFO|CI_TCHAR)), hContact);
			if (szFind != NULL) {
				if (!_tcscmp(tszContact, szFind))
					bMatch = TRUE;
				mir_free(szFind);
			}
		}
		if (bMatch) {
			ci->hContacts = (MCONTACT*)mir_realloc(ci->hContacts, (count + 1)*sizeof(MCONTACT));
			if (ci->hContacts == NULL)
				return -1;

			ci->hContacts[count] = hContact;
			count += 1;
		}
	}

	if (count == 1) { /* cache the found result */
		mir_cslock lck(csContactCache);
		cce = (CONTACTCE*)mir_realloc(cce, (cacheSize+1)*sizeof(CONTACTCE));
		if (cce != NULL) {
			cce[cacheSize].hContact = ci->hContacts[0];
			cce[cacheSize].flags = ci->flags;
			cce[cacheSize].tszContact = mir_tstrdup(tszContact);
			if (cce[cacheSize].tszContact != NULL)
				cacheSize += 1;
		}
	}

	return count;
}

/* keep cache consistent */
static int contactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbw = (DBCONTACTWRITESETTING*)lParam;

	mir_cslock lck(csContactCache);
	for (int i=0; i < cacheSize; i++) {
		if (hContact != cce[i].hContact && (cce[i].flags & CI_CNFINFO) == 0 )
			continue;

		char *szProto = GetContactProto(hContact);
		if (szProto == NULL)
			continue;

		char *uid = (char*)CallProtoService(szProto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0);
		if (((!strcmp(dbw->szSetting, "Nick")) && (cce[i].flags & CI_NICK)) ||
			 ((!strcmp(dbw->szSetting, "FirstName")) && (cce[i].flags & CI_FIRSTNAME)) ||
			 ((!strcmp(dbw->szSetting, "LastName")) && (cce[i].flags & CI_LASTNAME)) ||
			 ((!strcmp(dbw->szSetting, "e-mail")) && (cce[i].flags & CI_EMAIL)) ||
			 ((!strcmp(dbw->szSetting, "MyHandle")) && (cce[i].flags & CI_LISTNAME)) ||
			 (cce[i].flags & CI_CNFINFO) != 0 || // lazy; always invalidate CNF info cache entries
			 (( ((INT_PTR)uid != CALLSERVICE_NOTFOUND) && (uid != NULL)) && (!strcmp(dbw->szSetting, uid)) && (cce[i].flags & CI_UNIQUEID)))
		{
			/* remove from cache */
			mir_free(cce[i].tszContact);
			if (cacheSize > 1) {
				MoveMemory(&cce[i], &cce[cacheSize-1], sizeof(CONTACTCE));
				cce = ( CONTACTCE* )mir_realloc(cce, (cacheSize-1)*sizeof(CONTACTCE));
				cacheSize -= 1;
			}
			else {
				mir_free(cce);
				cce = NULL;
				cacheSize = 0;
			}
			break;
		}
	}
	return 0;
}

int initContactModule()
{
	InitializeCriticalSection(&csContactCache);
	hContactSettingChangedHook = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, contactSettingChanged);
	return 0;
}

int deinitContactModule()
{
	UnhookEvent(hContactSettingChangedHook);
	DeleteCriticalSection(&csContactCache);
	return 0;
}

// returns a string in the form <PROTOID:UNIQUEID>, cannot be _HANDLE_!
// result must be freed
TCHAR* encodeContactToString(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	TCHAR *tszUniqueId = getContactInfoT(CNF_UNIQUEID, hContact);
	if (szProto == NULL || tszUniqueId == NULL)
		return NULL;

	size_t size = _tcslen(tszUniqueId) + strlen(szProto) + 4;
	TCHAR *tszResult = (TCHAR *)mir_calloc(size * sizeof(TCHAR));
	if (tszResult)
		mir_sntprintf(tszResult, size, _T("<%S:%s>"), szProto, tszUniqueId);
	return tszResult;
}

// returns a contact from a string in the form <PROTOID:UNIQUEID>
// returns INVALID_HANDLE_VALUE in case of an error.
MCONTACT decodeContactFromString(TCHAR *tszContact)
{
	MCONTACT hContact = INVALID_CONTACT_ID;
	CONTACTSINFO ci = { sizeof(ci) };
	ci.tszContact = tszContact;
	ci.flags = CI_PROTOID|CI_TCHAR;
	int count = getContactFromString( &ci );
	if (count != 1) {
		mir_free(ci.hContacts);
		return hContact;
	}

	if (ci.hContacts != NULL) {
		hContact = ci.hContacts[0];
		mir_free(ci.hContacts);
	}

	return hContact;
}
