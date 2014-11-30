/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include <commonheaders.h>

static void TZ_LoadTimeZone(MCONTACT hContact, struct TExtraCache *c, const char *szProto);

//routines for managing adding/removal of items in the list, including sorting

ClcContact* CreateClcContact( void )
{
	ClcContact* p = (ClcContact*)mir_alloc( sizeof( ClcContact ));
	if ( p != NULL ) {
		ZeroMemory(p, sizeof(ClcContact));
		p->avatarLeft = p->extraIconRightBegin = p->xStatusIcon = -1;
	}
	return p;
}

int AddInfoItemToGroup(ClcGroup *group, int flags, const TCHAR *pszText)
{
	int i = coreCli.pfnAddInfoItemToGroup(group, flags, pszText);
	ClcContact* p = group->cl.items[i];
	p->codePage = 0;
	//p->clientId = -1;
	p->bIsMeta = 0;
	p->xStatus = 0;
	p->ace = NULL;
	p->pExtra = 0;
	p->avatarLeft = p->extraIconRightBegin = -1;
	return i;
}

ClcGroup *AddGroup(HWND hwnd, struct ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers)
{
	ClcGroup *p = coreCli.pfnAddGroup(hwnd, dat, szName, flags, groupId, calcTotalMembers);
	if ( p && p->parent )
		RTL_DetectGroupName( p->parent->cl.items[ p->parent->cl.count-1] );

	return p;
}

void LoadAvatarForContact(ClcContact *p)
{
	DWORD dwFlags;

	if (p->pExtra)
		dwFlags = p->pExtra->dwDFlags;
	else
		dwFlags = cfg::getDword(p->hContact, "CList", "CLN_Flags", 0);

	if (cfg::dat.dwFlags & CLUI_FRAME_AVATARS)
		p->cFlags = (dwFlags & ECF_HIDEAVATAR ? p->cFlags & ~ECF_AVATAR : p->cFlags | ECF_AVATAR);
	else
		p->cFlags = (dwFlags & ECF_FORCEAVATAR ? p->cFlags | ECF_AVATAR : p->cFlags & ~ECF_AVATAR);

	p->ace = NULL;
	if (cfg::dat.bAvatarServiceAvail && (p->cFlags & ECF_AVATAR) && (!cfg::dat.bNoOfflineAvatars || p->wStatus != ID_STATUS_OFFLINE)) {
		p->ace = (struct avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)p->hContact, 0);
		if (p->ace != NULL && p->ace->cbSize != sizeof(struct avatarCacheEntry))
			p->ace = NULL;
		if (p->ace != NULL)
			p->ace->t_lastAccess = cfg::dat.t_now;
	}
	if (p->ace == NULL)
		 p->cFlags &= ~ECF_AVATAR;
}

int AddContactToGroup(struct ClcData *dat, ClcGroup *group, MCONTACT hContact)
{
	int i = coreCli.pfnAddContactToGroup(dat, group, hContact);
	ClcContact* p = group->cl.items[i];

	p->wStatus = cfg::getWord(hContact, p->proto, "Status", ID_STATUS_OFFLINE);
	p->xStatus = cfg::getByte(hContact, p->proto, "XStatusId", 0);

	if (p->proto)
		p->bIsMeta = !strcmp(p->proto, META_PROTO);
	else
		p->bIsMeta = FALSE;
	if (p->bIsMeta && !(cfg::dat.dwFlags & CLUI_USEMETAICONS)) {
		p->hSubContact = db_mc_getMostOnline(hContact);
		p->metaProto = GetContactProto(p->hSubContact);
		p->iImage = pcli->pfnGetContactIcon(p->hSubContact);
	}
	else {
		p->iImage = pcli->pfnGetContactIcon(hContact);
		p->metaProto = NULL;
	}

	p->codePage = cfg::getDword(hContact, "Tab_SRMsg", "ANSIcodepage", cfg::getDword(hContact, "UserInfo", "ANSIcodepage", CP_ACP));
	p->bSecondLine = cfg::getByte(hContact, "CList", "CLN_2ndline", cfg::dat.dualRowMode);

	if (dat->bisEmbedded)
		p->pExtra = 0;
	else {
		p->pExtra = cfg::getCache(p->hContact, p->proto);
		GetExtendedInfo(p, dat);
		if (p->pExtra)
			p->pExtra->proto_status_item = GetProtocolStatusItem(p->bIsMeta ? p->metaProto : p->proto);

		LoadAvatarForContact(p);
		// notify other plugins to re-supply their extra images (icq for xstatus, mBirthday etc...)
		pcli->pfnSetAllExtraIcons(hContact);
	}

	RTL_DetectAndSet(p, p->hContact);

	p->avatarLeft = p->extraIconRightBegin = -1;
	p->flags |= cfg::getByte(p->hContact, "CList", "Priority", 0) ? CONTACTF_PRIORITY : 0;

	return i;
}

void RebuildEntireList(HWND hwnd, struct ClcData *dat)
{
	DWORD style = GetWindowLongPtr(hwnd, GWL_STYLE);
	ClcGroup *group;
	DBVARIANT dbv = {0};

	RowHeight::Clear(dat);
	RowHeight::getMaxRowHeight(dat, hwnd);

	dat->list.expanded = 1;
	dat->list.hideOffline = cfg::getByte("CLC", "HideOfflineRoot", 0);
	dat->list.cl.count = 0;
	dat->list.totalMembers = 0;
	dat->selection = -1;
	dat->SelectMode = cfg::getByte("CLC", "SelectMode", 0);
	{
		for (int i = 1; ; i++) {
			DWORD groupFlags;
			TCHAR *szGroupName = pcli->pfnGetGroupName(i, &groupFlags);
			if (szGroupName == NULL)
				break;
			pcli->pfnAddGroup(hwnd, dat, szGroupName, groupFlags, i, 0);
		}
	}

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
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
				if ( !(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline)) {
					char *szProto = GetContactProto(hContact);
					if (szProto == NULL) {
						if ( !pcli->pfnIsHiddenMode(dat, ID_STATUS_OFFLINE))
							AddContactToGroup(dat, group, hContact);
					}
					else if ( !pcli->pfnIsHiddenMode(dat, (WORD) cfg::getWord(hContact, szProto, "Status", ID_STATUS_OFFLINE)))
						AddContactToGroup(dat, group, hContact);
				}
				else AddContactToGroup(dat, group, hContact);
			}
		}
	}

	if (style & CLS_HIDEEMPTYGROUPS) {
		group = &dat->list;
		group->scanIndex = 0;
		for (; ;) {
			if (group->scanIndex == group->cl.count) {
				group = group->parent;
				if (group == NULL)
					break;
			}
			else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
				if (group->cl.items[group->scanIndex]->group->cl.count == 0)
					group = pcli->pfnRemoveItemFromGroup(hwnd, group, group->cl.items[group->scanIndex], 0);
				else {
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
}

/*
 * status msg in the database has changed.
 * get it and store it properly formatted in the extra data cache
 */

BYTE GetCachedStatusMsg(TExtraCache *p, char *szProto)
{
	if (p == NULL)
		return 0;

	p->bStatusMsgValid = STATUSMSG_NOTFOUND;
	MCONTACT hContact = p->hContact;

	DBVARIANT dbv = {0};
	INT_PTR result = cfg::getTString(hContact, "CList", "StatusMsg", &dbv);
	if ( !result && mir_tstrlen(dbv.ptszVal) > 0)
		p->bStatusMsgValid = STATUSMSG_CLIST;
	else {
		if ( !szProto)
			szProto = GetContactProto(hContact);
		if (szProto) {
			if ( !result )
				db_free( &dbv );
			if ( !( result = cfg::getTString(hContact, szProto, "YMsg", &dbv)) && mir_tstrlen(dbv.ptszVal) > 0)
				p->bStatusMsgValid = STATUSMSG_YIM;
			else if ( !(result = cfg::getTString(hContact, szProto, "StatusDescr", &dbv)) && mir_tstrlen(dbv.ptszVal) > 0)
				p->bStatusMsgValid = STATUSMSG_GG;
			else if ( !(result = cfg::getTString(hContact, szProto, "XStatusMsg", &dbv)) && mir_tstrlen(dbv.ptszVal) > 0)
				p->bStatusMsgValid = STATUSMSG_XSTATUS;
		}
	}

	if (p->bStatusMsgValid == STATUSMSG_NOTFOUND) { // no status msg, consider xstatus name (if available)
		if ( !result )
			db_free( &dbv );
		result = cfg::getTString(hContact, szProto, "XStatusName", &dbv);
		if ( !result && mir_tstrlen(dbv.ptszVal) > 1) {
			int iLen = mir_tstrlen(dbv.ptszVal);
			p->bStatusMsgValid = STATUSMSG_XSTATUSNAME;
			p->statusMsg = (TCHAR *)realloc(p->statusMsg, (iLen + 2) * sizeof(TCHAR));
			_tcsncpy(p->statusMsg, dbv.ptszVal, iLen + 1);
		}
		else {
			int xStatus;
			WPARAM xStatus2;
			TCHAR xStatusName[128];

			CUSTOM_STATUS cst = { sizeof(cst) };
			cst.flags = CSSF_MASK_STATUS;
			cst.status = &xStatus;
			if (ProtoServiceExists(szProto, PS_GETCUSTOMSTATUSEX) && !ProtoCallService(szProto, PS_GETCUSTOMSTATUSEX, hContact, (LPARAM)&cst) && xStatus > 0) {
				cst.flags = CSSF_MASK_NAME | CSSF_DEFAULT_NAME | CSSF_TCHAR;
 				cst.wParam = &xStatus2;
				cst.ptszName = xStatusName;
				if ( !CallProtoService(szProto, PS_GETCUSTOMSTATUSEX, hContact, (LPARAM)&cst)) {
					TCHAR *szwXstatusName = TranslateTS(xStatusName);
					p->statusMsg = (TCHAR *)realloc(p->statusMsg, (mir_tstrlen(szwXstatusName) + 2) * sizeof(TCHAR));
					_tcsncpy(p->statusMsg, szwXstatusName, mir_tstrlen(szwXstatusName) + 1);
					p->bStatusMsgValid = STATUSMSG_XSTATUSNAME;
				}
			}
		}
	}

	if (p->bStatusMsgValid > STATUSMSG_XSTATUSNAME) {
		int j = 0;
		p->statusMsg = (TCHAR *)realloc(p->statusMsg, (mir_tstrlen(dbv.ptszVal) + 2) * sizeof(TCHAR));
		for (int i = 0; dbv.ptszVal[i]; i++) {
			if (dbv.ptszVal[i] == (TCHAR)0x0d)
				continue;
			p->statusMsg[j] = dbv.ptszVal[i] == (wchar_t)0x0a ? (wchar_t)' ' : dbv.ptszVal[i];
			j++;
		}
		p->statusMsg[j] = 0;
	}
	if ( !result )
		db_free( &dbv );

	if (p->bStatusMsgValid != STATUSMSG_NOTFOUND) {
		WORD infoTypeC2[12];
		ZeroMemory(infoTypeC2, sizeof(WORD) * 12);
		int iLen = min(mir_wstrlen(p->statusMsg), 10);
		GetStringTypeW(CT_CTYPE2, p->statusMsg, iLen, infoTypeC2);
		p->dwCFlags &= ~ECF_RTLSTATUSMSG;
		for (int i = 0; i < 10; i++) {
			if (infoTypeC2[i] == C2_RIGHTTOLEFT) {
				p->dwCFlags |= ECF_RTLSTATUSMSG;
				break;
			}
		}
	}

	if (p->hTimeZone == NULL)
		TZ_LoadTimeZone(hContact, p, szProto);
	return p->bStatusMsgValid;
}

/*
 * load time zone information for the contact
 * if TzName is set, use it. It has to be a standard windows time zone name
 * Currently, it can only be set by using UserInfoEx plugin
 *
 * Fallback: use ordinary GMT offsets (incorrect, in some cases due to DST
 * differences.
 */

static void TZ_LoadTimeZone(MCONTACT hContact, struct TExtraCache *c, const char *szProto)
{
	DWORD flags = 0;
	if (cfg::dat.bShowLocalTimeSelective)
		flags |= TZF_DIFONLY;
	c->hTimeZone = tmi.createByContact(hContact, 0, flags);
}

void ReloadExtraInfo(MCONTACT hContact)
{
	if (hContact && pcli->hwndContactTree) {
		TExtraCache *p = cfg::getCache(hContact, NULL);
		if (p) {
			char *szProto = GetContactProto(hContact);

			TZ_LoadTimeZone(hContact, p, szProto);
			InvalidateRect(pcli->hwndContactTree, NULL, FALSE);
		}
	}
}

/*
 * autodetect RTL property of the nickname, evaluates the first 10 characters of the nickname only
 */


void RTL_DetectAndSet(ClcContact *contact, MCONTACT hContact)
{
	WORD infoTypeC2[12];
	int i;
	TCHAR *szText = NULL;
	DWORD iLen;
	TExtraCache *p;

	ZeroMemory(infoTypeC2, sizeof(WORD) * 12);

	if (contact == NULL) {
		szText = pcli->pfnGetContactDisplayName(hContact, 0);
		p = cfg::getCache(hContact, NULL);
	}
	else {
		szText = contact->szText;
		p = contact->pExtra;
	}
	if (p) {
		iLen = min(mir_wstrlen(szText), 10);
		GetStringTypeW(CT_CTYPE2, szText, iLen, infoTypeC2);
		p->dwCFlags &= ~ECF_RTLNICK;
		for (i = 0; i < 10; i++) {
			if (infoTypeC2[i] == C2_RIGHTTOLEFT) {
				p->dwCFlags |= ECF_RTLNICK;
				return;
			}
		}
	}
}

void RTL_DetectGroupName(ClcContact *group)
{
	WORD infoTypeC2[12];
	int i;
	DWORD iLen;

	group->isRtl = 0;

	if (group->szText) {
		iLen = min(mir_wstrlen(group->szText), 10);
		GetStringTypeW(CT_CTYPE2, group->szText, iLen, infoTypeC2);
		for (i = 0; i < 10; i++) {
			if (infoTypeC2[i] == C2_RIGHTTOLEFT) {
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

void GetExtendedInfo(ClcContact *contact, ClcData *dat)
{
	if (dat->bisEmbedded || contact == NULL)
		return;

	if (contact->proto == NULL || contact->hContact == 0)
		return;

	TExtraCache *p = contact->pExtra;
	if (p == NULL)
		return;

	p->msgFrequency = cfg::getDword(contact->hContact, "CList", "mf_freq", 0x7fffffff);
	if (p->valid)
		return;

	p->valid = TRUE;
	p->isChatRoom = cfg::getByte(contact->hContact, contact->proto, "ChatRoom", 0);
}

void LoadSkinItemToCache(TExtraCache *cEntry, const char *szProto)
{
	MCONTACT hContact = cEntry->hContact;

	if (cfg::getByte(hContact, "EXTBK", "VALID", 0)) {
		if (cEntry->status_item == NULL)
			cEntry->status_item = reinterpret_cast<StatusItems_t *>(malloc(sizeof(StatusItems_t)));
		ZeroMemory(cEntry->status_item, sizeof(StatusItems_t));
		strcpy(cEntry->status_item->szName, "{--CONTACT--}"); // mark as "per contact" item
		cEntry->status_item->IGNORED = 0;

		cEntry->status_item->TEXTCOLOR = cfg::getDword(hContact, "EXTBK", "TEXT", RGB(20, 20, 20));
		cEntry->status_item->COLOR = cfg::getDword(hContact, "EXTBK", "COLOR1", RGB(224, 224, 224));
		cEntry->status_item->COLOR2 = cfg::getDword(hContact, "EXTBK", "COLOR2", RGB(224, 224, 224));
		cEntry->status_item->ALPHA = (BYTE)cfg::getByte(hContact, "EXTBK", "ALPHA", 100);

		cEntry->status_item->MARGIN_LEFT = (DWORD)cfg::getByte(hContact, "EXTBK", "LEFT", 0);
		cEntry->status_item->MARGIN_RIGHT = (DWORD)cfg::getByte(hContact, "EXTBK", "RIGHT", 0);
		cEntry->status_item->MARGIN_TOP = (DWORD)cfg::getByte(hContact, "EXTBK", "TOP", 0);
		cEntry->status_item->MARGIN_BOTTOM = (DWORD)cfg::getByte(hContact, "EXTBK", "BOTTOM", 0);

		cEntry->status_item->COLOR2_TRANSPARENT = (BYTE)cfg::getByte(hContact, "EXTBK", "TRANS", 1);
		cEntry->status_item->BORDERSTYLE = cfg::getDword(hContact, "EXTBK", "BDR", 0);

		cEntry->status_item->CORNER = cfg::getByte(hContact, "EXTBK", "CORNER", 0);
		cEntry->status_item->GRADIENT = cfg::getByte(hContact, "EXTBK", "GRAD", 0);
	}
	else if (cEntry->status_item) {
		free(cEntry->status_item);
		cEntry->status_item = NULL;
	}
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
	TCHAR szGroupMask[256];
	DWORD dwLocalMask;

	// always hide subcontacts (but show them on embedded contact lists)

	if (dat != NULL && dat->bHideSubcontacts && cfg::dat.bMetaEnabled && db_mc_isSub(hContact))
		return 1;

	if ( !cfg::dat.bFilterEffective)
		return dbHidden;

	if (szProto == NULL)
		szProto = GetContactProto(hContact);
	// check stickies first (priority), only if we really have stickies defined (CLVM_STICKY_CONTACTS is set).
	if (cfg::dat.bFilterEffective & CLVM_STICKY_CONTACTS) {
		if ((dwLocalMask = cfg::getDword(hContact, "CLVM", cfg::dat.current_viewmode, 0)) != 0) {
			if (cfg::dat.bFilterEffective & CLVM_FILTER_STICKYSTATUS) {
				WORD wStatus = cfg::getWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
				return !((1 << (wStatus - ID_STATUS_OFFLINE)) & HIWORD(dwLocalMask));
			}
			return 0;
		}
	}
	// check the proto, use it as a base filter result for all further checks
	if (cfg::dat.bFilterEffective & CLVM_FILTER_PROTOS) {
		mir_snprintf(szTemp, sizeof(szTemp), "%s|", szProto);
		filterResult = strstr(cfg::dat.protoFilter, szTemp) ? 1 : 0;
	}
	if (cfg::dat.bFilterEffective & CLVM_FILTER_GROUPS) {
		if ( !cfg::getTString(hContact, "CList", "Group", &dbv)) {
			mir_sntprintf(szGroupMask, SIZEOF(szGroupMask), _T("%s|"), &dbv.ptszVal[1]);
			filterResult = (cfg::dat.filterFlags & CLVM_PROTOGROUP_OP) ? (filterResult | (_tcsstr(cfg::dat.groupFilter, szGroupMask) ? 1 : 0)) : (filterResult & (_tcsstr(cfg::dat.groupFilter, szGroupMask) ? 1 : 0));
			mir_free(dbv.ptszVal);
		}
		else if (cfg::dat.filterFlags & CLVM_INCLUDED_UNGROUPED)
			filterResult = (cfg::dat.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 1;
		else
			filterResult = (cfg::dat.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 0;
	}
	if (cfg::dat.bFilterEffective & CLVM_FILTER_STATUS) {
		WORD wStatus = cfg::getWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		filterResult = (cfg::dat.filterFlags & CLVM_GROUPSTATUS_OP) ? ((filterResult | ((1 << (wStatus - ID_STATUS_OFFLINE)) & cfg::dat.statusMaskFilter ? 1 : 0))) : (filterResult & ((1 << (wStatus - ID_STATUS_OFFLINE)) & cfg::dat.statusMaskFilter ? 1 : 0));
	}
	if (cfg::dat.bFilterEffective & CLVM_FILTER_LASTMSG) {
		DWORD now;
		TExtraCache *p = cfg::getCache(hContact, szProto);
		if (p) {
			now = cfg::dat.t_now;
			now -= cfg::dat.lastMsgFilter;
			if (cfg::dat.bFilterEffective & CLVM_FILTER_LASTMSG_OLDERTHAN)
				filterResult = filterResult & (p->dwLastMsgTime < now);
			else if (cfg::dat.bFilterEffective & CLVM_FILTER_LASTMSG_NEWERTHAN)
				filterResult = filterResult & (p->dwLastMsgTime > now);
		}
	}
	return (dbHidden | !filterResult);
}
