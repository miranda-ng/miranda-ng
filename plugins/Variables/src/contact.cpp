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

#include "stdafx.h"

struct _tagType
{
	int    cnfCode;
	wchar_t* str;
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

/* contact cache entry */
struct CONTACTCE
{
	int flags;
	wchar_t* tszContact;
	MCONTACT hContact;
}; 

static int SortContactCache(const CONTACTCE *p1, const CONTACTCE *p2)
{
	if (p1->flags != p2->flags)
		return (p1->flags > p2->flags) ? 1 : -1;

	return wcscmp(p1->tszContact, p2->tszContact);
}

/* cache for 'getcontactfromstring' service */
static OBJLIST<CONTACTCE> arContactCache(20, SortContactCache);
static mir_cs csContactCache;

// converts a string into a CNF_ type
BYTE getContactInfoType(wchar_t* type)
{
	if (type == NULL || mir_wstrlen(type) == 0)
		return 0;

	for (int i = 0; i < _countof(builtinCnfs); i++)
		if (!mir_wstrcmp(builtinCnfs[i].str, type))
			return builtinCnfs[i].cnfCode;

	return 0;
}

// returns info about a contact as a string
wchar_t* getContactInfoT(BYTE type, MCONTACT hContact)
{
	/* returns dynamic allocated buffer with info, or NULL if failed */
	if (hContact == NULL)
		return NULL;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return NULL;

	wchar_t *res = NULL;
	switch (type) {
	case CCNF_PROTOID:
		return mir_a2u(szProto);

	case CCNF_ACCOUNT:
		{
			PROTOACCOUNT *pa = Proto_GetAccount(szProto);
			return pa ? mir_wstrdup(pa->tszAccountName) : NULL;
		}

	case CCNF_PROTOCOL:
		char protoname[128];
		if (CallProtoService(szProto, PS_GETNAME, (WPARAM)sizeof(protoname), (LPARAM)protoname))
			return NULL;
		return mir_a2u(protoname);

	case CCNF_STATUS:
		return mir_wstrdup(pcli->pfnGetStatusModeDescription(db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), 0));

	case CCNF_INTERNALIP:
	case CCNF_EXTERNALIP:
		{
			DWORD ip = db_get_dw(hContact, szProto, (type == CCNF_INTERNALIP) ? "RealIP" : "IP", 0);
			if (ip != 0) {
				struct in_addr in;
				in.s_addr = htonl(ip);
				return mir_a2u(inet_ntoa(in));
			}
		}
		return NULL;

	case CCNF_GROUP:
		if ((res = db_get_wsa(hContact, "CList", "Group")) != NULL)
			return res;
		break;

	case CNF_UNIQUEID:
		//UID for ChatRoom
		if (db_get_b(hContact, szProto, "ChatRoom", 0) == 1)
			if ((res = db_get_wsa(hContact, szProto, "ChatRoomID")) != NULL)
				return res;

		//UID for other contact
		break;
	}

	return Contact_GetInfo(type, hContact);
}

// MS_VARS_GETCONTACTFROMSTRING
MCONTACT getContactFromString(const wchar_t *tszContact, DWORD dwFlags, int nMatch)
{
	/* service to retrieve a contact's HANDLE from a given string */
	if (tszContact == NULL || *tszContact == 0)
		return INVALID_CONTACT_ID;

	bool bReturnCount;
	if (dwFlags & CI_NEEDCOUNT) {
		dwFlags &= ~CI_NEEDCOUNT;
		bReturnCount = true;
	}
	else bReturnCount = false;

	// search the cache
	{
		CONTACTCE tmp = { dwFlags, (wchar_t*)tszContact, 0 };

		mir_cslock lck(csContactCache);
		CONTACTCE *p = arContactCache.find(&tmp);
		if (p != NULL)
			return (bReturnCount) ? 1 : p->hContact; // found in cache
	}

	// contact was not in cache, do a search
	CMStringW tmp;
	int count = 0;
	MCONTACT hContact;
	LIST<void> arResults(1);

	for (hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		// <_HANDLE_:hContact>
		tmp.Format(L"<%s:%d>", _A2W(PROTOID_HANDLE), hContact);
		bool bMatch = (tmp == tszContact);

		char *szProto = GetContactProto(hContact);
		if (szProto == NULL)
			continue;

		// <proto:id> (exact)
		if ((dwFlags & CI_PROTOID) && !bMatch) {
			ptrW cInfo(getContactInfoT(CNF_UNIQUEID, hContact));
			if (cInfo) {
				tmp.Format(L"<%S:%s>", szProto, cInfo);
				if (tmp == tszContact)
					bMatch = true;
			}
		}

		// id (exact)
		if ((dwFlags & CI_UNIQUEID) && !bMatch) {
			ptrW szFind(getContactInfoT(CNF_UNIQUEID, hContact));
			if (!mir_wstrcmp(tszContact, szFind))
				bMatch = true;
		}
		
		// nick (not exact)
		if ((dwFlags & CI_NICK) && !bMatch) {
			ptrW szFind(getContactInfoT(CNF_NICK, hContact));
			if (!mir_wstrcmp(tszContact, szFind))
				bMatch = true;
		}

		// list name (not exact)
		if ((dwFlags & CI_LISTNAME) && !bMatch) {
			ptrW szFind(getContactInfoT(CNF_DISPLAY, hContact));
			if (!mir_wstrcmp(tszContact, szFind))
				bMatch = true;
		}

		// firstname (exact)
		if ((dwFlags & CI_FIRSTNAME) && !bMatch) {
			ptrW szFind(getContactInfoT(CNF_FIRSTNAME, hContact));
			if (!mir_wstrcmp(tszContact, szFind))
				bMatch = true;
		}
		
		// lastname (exact)
		if ((dwFlags & CI_LASTNAME) && !bMatch) {
			ptrW szFind(getContactInfoT(CNF_LASTNAME, hContact));
			if (!mir_wstrcmp(tszContact, szFind))
				bMatch = true;
		}

		// email (exact)
		if ((dwFlags & CI_EMAIL) && !bMatch) {
			ptrW szFind(getContactInfoT(CNF_EMAIL, hContact));
			if (!mir_wstrcmp(tszContact, szFind))
				bMatch = true;
		}

		// CNF_ (exact)
		if ((dwFlags & CI_CNFINFO) && !bMatch) {
			ptrW szFind(getContactInfoT((BYTE)(dwFlags & ~CI_CNFINFO), hContact));
			if (!mir_wstrcmp(tszContact, szFind))
				bMatch = true;
		}

		if (bMatch) {
			if (nMatch == -1)
				arResults.insert((HANDLE)hContact);
			else if (nMatch == count)
				break;
			count++;
		}
	}

	if (bReturnCount)
		return count;

	if (hContact == 0)
		return INVALID_CONTACT_ID;

	// return random contact
	if (nMatch == -1)
		return (MCONTACT)arResults[rand() % arResults.getCount()];

	// cache the found result
	if (count == 0) {
		mir_cslock lck(csContactCache);

		CONTACTCE *cce = new CONTACTCE();
		cce->hContact = hContact;
		cce->flags = dwFlags;
		cce->tszContact = mir_wstrdup(tszContact);
		arContactCache.insert(cce);
	}

	return hContact;
}

/* keep cache consistent */
static int contactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbw = (DBCONTACTWRITESETTING*)lParam;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return 0;

	char *uid = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);

	bool isNick = !strcmp(dbw->szSetting, "Nick");
	bool isFirstName = !strcmp(dbw->szSetting, "FirstName");
	bool isLastName = !strcmp(dbw->szSetting, "LastName");
	bool isEmail = !strcmp(dbw->szSetting, "e-mail");
	bool isMyHandle = !strcmp(dbw->szSetting, "MyHandle");
	bool isUid = (((INT_PTR)uid != CALLSERVICE_NOTFOUND) && (uid != NULL)) && (!strcmp(dbw->szSetting, uid));

	mir_cslock lck(csContactCache);
	for (int i = 0; i < arContactCache.getCount(); i++) {
		CONTACTCE &cce = arContactCache[i];
		if (hContact != cce.hContact && (cce.flags & CI_CNFINFO) == 0)
			continue;
		
		if ((isNick && (cce.flags & CI_NICK)) ||
			(isFirstName && (cce.flags & CI_FIRSTNAME)) ||
			(isLastName && (cce.flags & CI_LASTNAME)) ||
			(isEmail && (cce.flags & CI_EMAIL)) ||
			(isMyHandle && (cce.flags & CI_LISTNAME)) ||
			(cce.flags & CI_CNFINFO) != 0 || // lazy; always invalidate CNF info cache entries
			(isUid && (cce.flags & CI_UNIQUEID))) 
		{
			/* remove from cache */
			mir_free(cce.tszContact);
			arContactCache.remove(i);
			break;
		}
	}
	return 0;
}

int initContactModule()
{
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, contactSettingChanged);
	return 0;
}

int deinitContactModule()
{
	for (int i = 0; i < arContactCache.getCount(); i++)
		mir_free(arContactCache[i].tszContact);
	arContactCache.destroy();
	return 0;
}

// returns a string in the form <PROTOID:UNIQUEID>, cannot be _HANDLE_!
// result must be freed
wchar_t* encodeContactToString(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return NULL;

	wchar_t *tszUniqueId = getContactInfoT(CNF_UNIQUEID, hContact);
	if (tszUniqueId == NULL)
		return NULL;

	size_t size = mir_wstrlen(tszUniqueId) + mir_strlen(szProto) + 4;
	wchar_t *tszResult = (wchar_t *)mir_calloc(size * sizeof(wchar_t));
	if (tszResult)
		mir_snwprintf(tszResult, size, L"<%S:%s>", szProto, tszUniqueId);
	return tszResult;
}
