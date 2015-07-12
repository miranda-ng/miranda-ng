/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: clcitems.cpp 138 2010-11-01 10:51:15Z silvercircle $
 *
 */

#include <commonheaders.h>
#include <m_icq.h>

extern HANDLE hExtraImageListRebuilding, hExtraImageApplying;

static void TZ_LoadTimeZone(MCONTACT hContact, struct TExtraCache *c, const char *szProto);

//routines for managing adding/removal of items in the list, including sorting

ClcContact* CLC::CreateClcContact()
{
	struct ClcContact* p = (struct ClcContact*)mir_alloc( sizeof( struct ClcContact ) );
	if ( p != NULL ) {
		ZeroMemory(p, sizeof(struct ClcContact));
		//p->clientId = -1;
		p->extraCacheEntry = -1;
		p->avatarLeft = p->extraIconRightBegin = -1;
		p->isRtl = 0;
		p->ace = 0;
	}
	return p;
}

int CLC::AddInfoItemToGroup(ClcGroup *group, int flags, const wchar_t *pszText)
{
	int i = coreCli.pfnAddInfoItemToGroup(group, flags, pszText);
	ClcContact* p = group->cl.items[i];
	p->bIsMeta = 0;
	p->xStatus = 0;
	p->ace = NULL;
	p->extraCacheEntry = -1;
	p->avatarLeft = p->extraIconRightBegin = -1;
	return i;
}

ClcGroup* CLC::AddGroup(HWND hwnd, struct ClcData *dat, const wchar_t *szName, DWORD flags, int groupId, int calcTotalMembers)
{
	struct ClcGroup *p = coreCli.pfnAddGroup( hwnd, dat, szName, flags, groupId, calcTotalMembers);

		if ( p && p->parent )
			RTL_DetectGroupName( p->parent->cl.items[ p->parent->cl.count-1] );
	return p;
}

ClcGroup* CLC::RemoveItemFromGroup(HWND hwnd, ClcGroup *group, ClcContact *contact, int updateTotalCount)
{
	if(contact->extraCacheEntry >= 0 && contact->extraCacheEntry < cfg::nextCacheEntry) {
		if(cfg::eCache[contact->extraCacheEntry].floater && cfg::eCache[contact->extraCacheEntry].floater->hwnd)
			ShowWindow(cfg::eCache[contact->extraCacheEntry].floater->hwnd, SW_HIDE);
	}
	return(coreCli.pfnRemoveItemFromGroup(hwnd, group, contact, updateTotalCount));
}

void LoadAvatarForContact(struct ClcContact *p)
{
    if(cfg::dat.dwFlags & CLUI_FRAME_AVATARS)
        p->cFlags = (p->dwDFlags & ECF_HIDEAVATAR ? p->cFlags & ~ECF_AVATAR : p->cFlags | ECF_AVATAR);
    else
        p->cFlags = (p->dwDFlags & ECF_FORCEAVATAR ? p->cFlags | ECF_AVATAR : p->cFlags & ~ECF_AVATAR);

    p->ace = NULL;
    if(/*(p->cFlags & ECF_AVATAR) && */(!cfg::dat.bNoOfflineAvatars || p->wStatus != ID_STATUS_OFFLINE)) {
        p->ace = (struct avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)p->hContact, 0);
        if(CALLSERVICE_NOTFOUND == (INT_PTR)p->ace)
        	p->ace = 0;
        if (p->ace != NULL && p->ace->cbSize != sizeof(struct avatarCacheEntry))
            p->ace = NULL;
        if (p->ace != NULL)
            p->ace->t_lastAccess = cfg::dat.t_now;
    }
    if(p->ace == NULL)
        p->cFlags &= ~ECF_AVATAR;
}

int CLC::AddContactToGroup(struct ClcData *dat, struct ClcGroup *group, MCONTACT hContact)
{
	int i = coreCli.pfnAddContactToGroup( dat, group, hContact );
	struct ClcContact* p = group->cl.items[i];

	p->wStatus = cfg::getWord(hContact, p->proto, "Status", ID_STATUS_OFFLINE);
	p->xStatus = cfg::getByte(hContact, p->proto, "XStatusId", 0);
    //p->iRowHeight = -1;

	if (p->proto)
		p->bIsMeta = !strcmp(p->proto, cfg::dat.szMetaName);
	else
		p->bIsMeta = FALSE;
	if (p->bIsMeta && cfg::dat.bMetaAvail) {
		p->hSubContact = db_mc_getMostOnline(hContact);
		p->metaProto = GetContactProto(p->hSubContact);
		p->iImage = CallService(MS_CLIST_GETCONTACTICON, (WPARAM) p->hSubContact, 0);
	} else {
		p->iImage = CallService(MS_CLIST_GETCONTACTICON, (WPARAM) hContact, 0);
		p->metaProto = NULL;
	}
    p->bSecondLine = cfg::dat.dualRowMode;
    p->bSecondLineLocal = cfg::getByte(hContact, "CList", "CLN_2ndline", -1);

    p->dwDFlags = cfg::getDword(hContact, "CList", "CLN_Flags", 0);

	if(dat->bisEmbedded)
		p->extraCacheEntry = -1;
	else {
		p->extraCacheEntry = cfg::getCache(p->hContact, p->proto);
		GetExtendedInfo( p, dat);
		if(p->extraCacheEntry >= 0 && p->extraCacheEntry < cfg::nextCacheEntry) {
			cfg::eCache[p->extraCacheEntry].proto_status_item = GetProtocolStatusItem(p->bIsMeta ? p->metaProto : p->proto);
			if(cfg::getByte(p->hContact, "CList", "floating", 0) && g_floatoptions.enabled) {
				if(cfg::eCache[p->extraCacheEntry].floater == NULL)
					FLT_Create(p->extraCacheEntry);
				else {
					ShowWindow(cfg::eCache[p->extraCacheEntry].floater->hwnd, SW_SHOWNOACTIVATE);
					FLT_Update(dat, p);
				}
			}
		}
        LoadAvatarForContact(p);
        // notify other plugins to re-supply their extra images (icq for xstatus, mBirthday etc...)
		pcli->pfnSetAllExtraIcons(hContact);
	}
	RTL_DetectAndSet( p, p->hContact);
	p->avatarLeft = p->extraIconRightBegin = -1;
	p->flags |= cfg::getByte(p->hContact, "CList", "Priority", 0) ? CONTACTF_PRIORITY : 0;

	return i;
}

void CLC::RebuildEntireList(HWND hwnd, ClcData *dat)
{
	char				*szProto;
	DWORD				style = GetWindowLong(hwnd, GWL_STYLE);
	MCONTACT			hContact;
	ClcGroup			*group;
	DBVARIANT			dbv = {0};

	RowHeight::Clear(dat);
	RowHeight::getMaxRowHeight(dat, hwnd);

	dat->list.expanded = 1;
	dat->list.hideOffline = cfg::getByte("CLC", "HideOfflineRoot", 0);
	dat->list.cl.count = 0;
	dat->list.totalMembers = 0;
	dat->selection = -1;
	dat->SelectMode = cfg::getByte("CLC", "SelectMode", 0); {
		int i;
		wchar_t *szGroupName;
		DWORD groupFlags;

		for (i = 1; ; i++) {
			szGroupName = pcli->pfnGetGroupName(i, &groupFlags);
			if (szGroupName == NULL)
				break;
			pcli->pfnAddGroup(hwnd, dat, szGroupName, groupFlags, i, 0);
		}
	}

	hContact = db_find_first();
	while (hContact) {
		if (style & CLS_SHOWHIDDEN || !CLVM_GetContactHiddenStatus(hContact, NULL, dat)) {
			ZeroMemory((void *)&dbv, sizeof(dbv));
			if (cfg::getTString(hContact, "CList", "Group", &dbv))
				group = &dat->list;
			else {
				group = pcli->pfnAddGroup(hwnd, dat, dbv.ptszVal, (DWORD) - 1, 0, 0);
				mir_free(dbv.ptszVal);
			}

			if (group != NULL) {
				group->totalMembers++;
				if (!(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline)) {
					szProto = GetContactProto(hContact);
					if (szProto == NULL) {
						if (!pcli->pfnIsHiddenMode(dat, ID_STATUS_OFFLINE))
							AddContactToGroup(dat, group, hContact);
					} else if (!pcli->pfnIsHiddenMode(dat, (WORD) cfg::getWord(hContact, szProto, "Status", ID_STATUS_OFFLINE)))
						AddContactToGroup(dat, group, hContact);
				} else
					AddContactToGroup(dat, group, hContact);
			}
		}
		hContact = db_find_next(hContact);
	}

	if (style & CLS_HIDEEMPTYGROUPS) {
		group = &dat->list;
		group->scanIndex = 0;
		for (; ;) {
			if (group->scanIndex == group->cl.count) {
				group = group->parent;
				if (group == NULL)
					break;
			} else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
				if (group->cl.items[group->scanIndex]->group->cl.count == 0) {
					group = pcli->pfnRemoveItemFromGroup(hwnd, group, group->cl.items[group->scanIndex], 0);
				} else {
					group = group->cl.items[group->scanIndex]->group;
					group->scanIndex = 0;
				}
				continue;
			}
			group->scanIndex++;
		}
	}
	pcli->pfnSortCLC(hwnd, dat, 0);
	pcli->pfnSetAllExtraIcons(NULL);
	if(!dat->bisEmbedded)
		FLT_SyncWithClist();
}

/*
 * status msg in the database has changed.
 * get it and store it properly formatted in the extra data cache
 */

BYTE GetCachedStatusMsg(int iExtraCacheEntry, char *szProto)
{
	DBVARIANT		dbv = {0};
	MCONTACT		hContact;
	TExtraCache		*cEntry;
	int				result;

	if(iExtraCacheEntry < 0 || iExtraCacheEntry > cfg::nextCacheEntry)
		return 0;

	cEntry = &cfg::eCache[iExtraCacheEntry];

	cEntry->bStatusMsgValid = STATUSMSG_NOTFOUND;
	hContact = cEntry->hContact;

	result = cfg::getTString(hContact, "CList", "StatusMsg", &dbv);
	if ( !result && lstrlen(dbv.ptszVal) > 1)
		cEntry->bStatusMsgValid = STATUSMSG_CLIST;
	else {
		if (!szProto)
			szProto = GetContactProto(hContact);
		if(szProto) {
			if ( !result )
				db_free(&dbv);
			if( !( result = cfg::getTString(hContact, szProto, "YMsg", &dbv)) && lstrlen(dbv.ptszVal) > 1)
				cEntry->bStatusMsgValid = STATUSMSG_YIM;
			else if ( !(result = cfg::getTString(hContact, szProto, "StatusDescr", &dbv)) && lstrlen(dbv.ptszVal) > 1)
				cEntry->bStatusMsgValid = STATUSMSG_GG;
			else if( !(result = cfg::getTString(hContact, szProto, "XStatusMsg", &dbv)) && lstrlen(dbv.ptszVal) > 1)
				cEntry->bStatusMsgValid = STATUSMSG_XSTATUS;
	}	}

	if(cEntry->bStatusMsgValid == STATUSMSG_NOTFOUND) {      // no status msg, consider xstatus name (if available)
		if ( !result )
			db_free( &dbv );
		result = cfg::getTString(hContact, szProto, "XStatusName", &dbv);
		if ( !result && lstrlen(dbv.ptszVal) > 1) {
			int iLen = lstrlen(dbv.ptszVal);
			cEntry->bStatusMsgValid = STATUSMSG_XSTATUSNAME;
			cEntry->statusMsg = (wchar_t *)realloc(cEntry->statusMsg, (iLen + 2) * sizeof(wchar_t));
			_tcsncpy(cEntry->statusMsg, dbv.ptszVal, iLen + 1);
		}
		else {
			int xStatus;
			WPARAM xStatus2;
			wchar_t xStatusName[128];

			CUSTOM_STATUS cst = { sizeof(cst) };
			cst.flags = CSSF_MASK_STATUS;
			cst.status = &xStatus;
			if (ProtoServiceExists(szProto, PS_GETCUSTOMSTATUSEX) && !CallProtoService(szProto, PS_GETCUSTOMSTATUSEX, hContact, (LPARAM)&cst) && xStatus > 0) {
				cst.flags = CSSF_MASK_NAME | CSSF_DEFAULT_NAME | CSSF_TCHAR;
 				cst.wParam = &xStatus2;
				cst.ptszName = xStatusName;
				if (!CallProtoService(szProto, PS_GETCUSTOMSTATUSEX, hContact, (LPARAM)&cst)) {
					wchar_t *szwXstatusName = TranslateTS(xStatusName);
					cEntry->statusMsg = (wchar_t *)realloc(cEntry->statusMsg, (lstrlen(szwXstatusName) + 2) * sizeof(wchar_t));
					_tcsncpy(cEntry->statusMsg, szwXstatusName, lstrlen(szwXstatusName) + 1);
					cEntry->bStatusMsgValid = STATUSMSG_XSTATUSNAME;
				}
			}
		}
	}
	if(cEntry->bStatusMsgValid > STATUSMSG_XSTATUSNAME) {
		int j = 0, i;
		cEntry->statusMsg = (wchar_t *)realloc(cEntry->statusMsg, (lstrlen(dbv.ptszVal) + 2) * sizeof(wchar_t));
		for(i = 0; dbv.ptszVal[i]; i++) {
			if(dbv.ptszVal[i] == (wchar_t)0x0d)
				continue;
			cEntry->statusMsg[j] = dbv.ptszVal[i] == (wchar_t)0x0a ? (wchar_t)' ' : dbv.ptszVal[i];
			j++;
		}
		cEntry->statusMsg[j] = (wchar_t)0;
	}
	if ( !result )
		db_free( &dbv );

	if(cEntry->bStatusMsgValid != STATUSMSG_NOTFOUND) {
		WORD infoTypeC2[12];
		int iLen, i
			;
		ZeroMemory(infoTypeC2, sizeof(WORD) * 12);
		iLen = min(lstrlenW(cEntry->statusMsg), 10);
		GetStringTypeW(CT_CTYPE2, cEntry->statusMsg, iLen, infoTypeC2);
		cEntry->dwCFlags &= ~ECF_RTLSTATUSMSG;
		for(i = 0; i < 10; i++) {
			if(infoTypeC2[i] == C2_RIGHTTOLEFT) {
				cEntry->dwCFlags |= ECF_RTLSTATUSMSG;
				break;
			}
		}
	}
	if(cEntry->hTimeZone == 0)
		TZ_LoadTimeZone(hContact, cEntry, szProto);
	return cEntry->bStatusMsgValid;;
}

/*
 * load time zone information for the contact
 * if TzName is set, use it. It has to be a standard windows time zone name
 * Currently, it can only be set by using UserInfoEx plugin
 *
 * Fallback: use ordinary GMT offsets (incorrect, in some cases due to DST
 * differences.
 */

static void TZ_LoadTimeZone(MCONTACT hContact, TExtraCache *c, const char *szProto)
{
	DWORD flags = 0;
	if (cfg::dat.bShowLocalTimeSelective) flags |= TZF_DIFONLY;
		c->hTimeZone = TimeZone_CreateByContact(hContact, 0, flags);
}

void ReloadExtraInfo(MCONTACT hContact)
{
	if(hContact && pcli->hwndContactTree) {
		int index = cfg::getCache(hContact, NULL);
		if(index >= 0 && index < cfg::nextCacheEntry) {
			char *szProto = GetContactProto(hContact);

			TZ_LoadTimeZone(hContact, &cfg::eCache[index], szProto);
			InvalidateRect(pcli->hwndContactTree, NULL, FALSE);
		}
	}
}

/*
 * autodetect RTL property of the nickname, evaluates the first 10 characters of the nickname only
 */

void RTL_DetectAndSet(struct ClcContact *contact, MCONTACT hContact)
{
    WORD infoTypeC2[12];
    int i, index;
    wchar_t *szText = NULL;
    DWORD iLen;

    ZeroMemory(infoTypeC2, sizeof(WORD) * 12);

    if(contact == NULL) {
        szText = pcli->pfnGetContactDisplayName(hContact, 0);
        index = cfg::getCache(hContact, NULL);
    }
    else {
        szText = contact->szText;
        index = contact->extraCacheEntry;
    }
    if(index >= 0 && index < cfg::nextCacheEntry) {
        iLen = min(lstrlenW(szText), 10);
        GetStringTypeW(CT_CTYPE2, szText, iLen, infoTypeC2);
        cfg::eCache[index].dwCFlags &= ~ECF_RTLNICK;
        for(i = 0; i < 10; i++) {
            if(infoTypeC2[i] == C2_RIGHTTOLEFT) {
                cfg::eCache[index].dwCFlags |= ECF_RTLNICK;
                return;
            }
        }
    }
}

void RTL_DetectGroupName(struct ClcContact *group)
{
    WORD infoTypeC2[12];
    int i;
    DWORD iLen;

    group->isRtl = 0;

    if(group->szText) {
        iLen = min(lstrlenW(group->szText), 10);
        GetStringTypeW(CT_CTYPE2, group->szText, iLen, infoTypeC2);
        for(i = 0; i < 10; i++) {
            if(infoTypeC2[i] == C2_RIGHTTOLEFT) {
                group->isRtl = 1;
                return;
            }
        }
    }
}
/*
 * check for exteneded user information - email, phone numbers, homepage
 * set extra icons accordingly
 */

void GetExtendedInfo(struct ClcContact *contact, struct ClcData *dat)
{
    int index;

    if(dat->bisEmbedded || contact == NULL)
        return;

    if(contact->proto == NULL || contact->hContact == 0)
        return;

    index = contact->extraCacheEntry;

    if(index >= 0 && index < cfg::nextCacheEntry) {
        if(cfg::eCache[index].valid)
            return;
        cfg::eCache[index].valid = TRUE;
    }
    else
        return;
	cfg::eCache[index].isChatRoom = cfg::getByte(contact->hContact, contact->proto, "ChatRoom", 0);
	cfg::eCache[index].msgFrequency = cfg::getDword(contact->hContact, "CList", "mf_freq", 0x7fffffff);

}

DWORD CalcXMask(MCONTACT hContact)
{
    DWORD dwXMask = cfg::getDword(hContact, "CList", "CLN_xmask", 0);
    int   i;
    DWORD dwResult = cfg::dat.dwExtraImageMask, bForced, bHidden;

    for(i = 0; i <= 10; i++) {
        bForced = (dwXMask & (1 << (2 * i)));
        bHidden = (dwXMask & (1 << (2 * i + 1)));
        if(bForced == 0 && bHidden == 0)
            continue;
        else if(bForced)
            dwResult |= (1 << i);
        else if(bHidden)
            dwResult &= ~(1 << i);
    }
    return(dwResult);
}

/*
 * checks the currently active view mode filter and returns true, if the contact should be hidden
 * if no view mode is active, it returns the CList/Hidden setting
 * also cares about sub contacts (if meta is active)
 */

int __fastcall CLVM_GetContactHiddenStatus(MCONTACT hContact, char *szProto, struct ClcData *dat)
{
    int dbHidden = cfg::getByte(hContact, "CList", "Hidden", 0);		// default hidden state, always respect it.
    int filterResult = 1;
    DBVARIANT dbv = {0};
    char szTemp[64];
    wchar_t szGroupMask[256];
    DWORD dwLocalMask;

    // always hide subcontacts (but show them on embedded contact lists)

    if(cfg::dat.bMetaAvail && dat != NULL && dat->bHideSubcontacts && cfg::dat.bMetaEnabled && cfg::getByte(hContact, cfg::dat.szMetaName, "IsSubcontact", 0))
        return 1;

    if(cfg::dat.bFilterEffective) {
		if (szProto == NULL)
			szProto = GetContactProto(hContact);
		// check stickies first (priority), only if we really have stickies defined (CLVM_STICKY_CONTACTS is set).
        if(cfg::dat.bFilterEffective & CLVM_STICKY_CONTACTS) {
            if((dwLocalMask = cfg::getDword(hContact, "CLVM", cfg::dat.current_viewmode, 0)) != 0) {
                if(cfg::dat.bFilterEffective & CLVM_FILTER_STICKYSTATUS) {
                    WORD wStatus = cfg::getWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
                    return !((1 << (wStatus - ID_STATUS_OFFLINE)) & HIWORD(dwLocalMask));
                }
                return 0;
            }
        }
        // check the proto, use it as a base filter result for all further checks
		if(cfg::dat.bFilterEffective & CLVM_FILTER_PROTOS) {
            mir_snprintf(szTemp, sizeof(szTemp), "%s|", szProto);
            filterResult = strstr(cfg::dat.protoFilter, szTemp) ? 1 : 0;
        }
        if(cfg::dat.bFilterEffective & CLVM_FILTER_GROUPS) {
            if(!cfg::getTString(hContact, "CList", "Group", &dbv)) {
                _sntprintf(szGroupMask, safe_sizeof(szGroupMask), _T("%s|"), &dbv.ptszVal[1]);
                filterResult = (cfg::dat.filterFlags & CLVM_PROTOGROUP_OP) ? (filterResult | (_tcsstr(cfg::dat.groupFilter, szGroupMask) ? 1 : 0)) : (filterResult & (_tcsstr(cfg::dat.groupFilter, szGroupMask) ? 1 : 0));
                mir_free(dbv.ptszVal);
            }
            else if(cfg::dat.filterFlags & CLVM_INCLUDED_UNGROUPED)
                filterResult = (cfg::dat.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 1;
            else
                filterResult = (cfg::dat.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 0;
        }
        if(cfg::dat.bFilterEffective & CLVM_FILTER_STATUS) {
            WORD wStatus = cfg::getWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
            filterResult = (cfg::dat.filterFlags & CLVM_GROUPSTATUS_OP) ? ((filterResult | ((1 << (wStatus - ID_STATUS_OFFLINE)) & cfg::dat.statusMaskFilter ? 1 : 0))) : (filterResult & ((1 << (wStatus - ID_STATUS_OFFLINE)) & cfg::dat.statusMaskFilter ? 1 : 0));
        }
		if(cfg::dat.bFilterEffective & CLVM_FILTER_LASTMSG) {
			DWORD now;
			int iEntry = cfg::getCache(hContact, szProto);
			if(iEntry >= 0 && iEntry <= cfg::nextCacheEntry) {
				now = cfg::dat.t_now;
				now -= cfg::dat.lastMsgFilter;
				if(cfg::dat.bFilterEffective & CLVM_FILTER_LASTMSG_OLDERTHAN)
					filterResult = filterResult & (cfg::eCache[iEntry].dwLastMsgTime < now);
				else if(cfg::dat.bFilterEffective & CLVM_FILTER_LASTMSG_NEWERTHAN)
					filterResult = filterResult & (cfg::eCache[iEntry].dwLastMsgTime > now);
			}
		}
        return (dbHidden | !filterResult);
    }
    else
        return dbHidden;
}
