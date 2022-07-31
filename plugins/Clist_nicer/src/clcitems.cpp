/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"

/*
* load time zone information for the contact
* if TzName is set, use it. It has to be a standard windows time zone name
* Currently, it can only be set by using UserInfoEx plugin
*
* Fallback: use ordinary GMT offsets (incorrect, in some cases due to DST
* differences.
*/

static void TZ_LoadTimeZone(MCONTACT hContact, struct TExtraCache *c)
{
	uint32_t flags = 0;
	if (cfg::dat.bShowLocalTimeSelective)
		flags |= TZF_DIFONLY;
	c->hTimeZone = TimeZone_CreateByContact(hContact, nullptr, flags);
}

//routines for managing adding/removal of items in the list, including sorting

ClcContact* CreateClcContact(void)
{
	ClcContact* p = (ClcContact*)mir_calloc(sizeof(ClcContact));
	if (p != nullptr)
		p->avatarLeft = p->extraIconRightBegin = p->xStatusIcon = -1;

	return p;
}

ClcContact* AddInfoItemToGroup(ClcGroup *group, int flags, const wchar_t *pszText)
{
	ClcContact *p = coreCli.pfnAddInfoItemToGroup(group, flags, pszText);
	p->avatarLeft = p->extraIconRightBegin = -1;
	return p;
}

ClcGroup *AddGroup(HWND hwnd, struct ClcData *dat, const wchar_t *szName, uint32_t flags, int groupId, int calcTotalMembers)
{
	ClcGroup *p = coreCli.pfnAddGroup(hwnd, dat, szName, flags, groupId, calcTotalMembers);
	if (p && p->parent)
		RTL_DetectGroupName(p->parent->cl[p->parent->cl.getCount() - 1]);

	return p;
}

void LoadAvatarForContact(ClcContact *p)
{
	uint32_t dwFlags;

	if (p->pExtra)
		dwFlags = p->pExtra->dwDFlags;
	else
		dwFlags = g_plugin.getDword(p->hContact, "CLN_Flags");

	if (cfg::dat.dwFlags & CLUI_FRAME_AVATARS)
		p->cFlags = (dwFlags & ECF_HIDEAVATAR ? p->cFlags & ~ECF_AVATAR : p->cFlags | ECF_AVATAR);
	else
		p->cFlags = (dwFlags & ECF_FORCEAVATAR ? p->cFlags | ECF_AVATAR : p->cFlags & ~ECF_AVATAR);

	p->ace = nullptr;
	if (cfg::dat.bAvatarServiceAvail && (p->cFlags & ECF_AVATAR) && (!cfg::dat.bNoOfflineAvatars || p->wStatus != ID_STATUS_OFFLINE)) {
		p->ace = (struct AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)p->hContact, 0);
		if (p->ace != nullptr)
			p->ace->t_lastAccess = cfg::dat.t_now;
	}
	if (p->ace == nullptr)
		p->cFlags &= ~ECF_AVATAR;
}

ClcContact* AddContactToGroup(struct ClcData *dat, ClcGroup *group, MCONTACT hContact)
{
	ClcContact *p = coreCli.pfnAddContactToGroup(dat, group, hContact);

	p->wStatus = db_get_w(hContact, p->pce->szProto, "Status", ID_STATUS_OFFLINE);
	p->xStatus = db_get_b(hContact, p->pce->szProto, "XStatusId", 0);

	if (p->pce->szProto)
		p->bIsMeta = !mir_strcmp(p->pce->szProto, META_PROTO);
	else
		p->bIsMeta = FALSE;
	if (p->bIsMeta && !(cfg::dat.dwFlags & CLUI_USEMETAICONS)) {
		p->hSubContact = db_mc_getMostOnline(hContact);
		p->metaProto = Proto_GetBaseAccountName(p->hSubContact);
		p->iImage = Clist_GetContactIcon(p->hSubContact);
	}
	else {
		p->iImage = Clist_GetContactIcon(hContact);
		p->metaProto = nullptr;
	}

	p->codePage = db_get_dw(hContact, "Tab_SRMsg", "ANSIcodepage", db_get_dw(hContact, "UserInfo", "ANSIcodepage", CP_ACP));
	p->bSecondLine = g_plugin.getByte(hContact, "CLN_2ndline", cfg::dat.dualRowMode);

	if (dat->bisEmbedded)
		p->pExtra = nullptr;
	else {
		p->pExtra = cfg::getCache(p->hContact, p->pce->szProto);
		GetExtendedInfo(p, dat);
		if (p->pExtra)
			p->pExtra->proto_status_item = GetProtocolStatusItem(p->bIsMeta ? p->metaProto : p->pce->szProto);

		LoadAvatarForContact(p);
		// notify other plugins to re-supply their extra images (icq for xstatus, mBirthday etc...)
		ExtraIcon_SetAll(hContact);
	}

	RTL_DetectAndSet(p, p->hContact);

	p->avatarLeft = p->extraIconRightBegin = -1;
	p->flags |= g_plugin.getByte(p->hContact, "Priority", 0) ? CONTACTF_PRIORITY : 0;

	return p;
}

void RebuildEntireList(HWND hwnd, struct ClcData *dat)
{
	RowHeight::Clear(dat);
	RowHeight::getMaxRowHeight(dat, hwnd);

	dat->SelectMode = db_get_b(0, "CLC", "SelectMode", 0);

	coreCli.pfnRebuildEntireList(hwnd, dat);
}

/*
 * status msg in the database has changed.
 * get it and store it properly formatted in the extra data cache
 */

uint8_t GetCachedStatusMsg(TExtraCache *p, char *szProto)
{
	if (p == nullptr)
		return 0;

	p->bStatusMsgValid = STATUSMSG_NOTFOUND;
	MCONTACT hContact = p->hContact;

	DBVARIANT dbv = { 0 };
	INT_PTR result = g_plugin.getWString(hContact, "StatusMsg", &dbv);
	if (!result && mir_wstrlen(dbv.pwszVal) > 0)
		p->bStatusMsgValid = STATUSMSG_CLIST;
	else {
		if (!szProto)
			szProto = Proto_GetBaseAccountName(hContact);
		if (szProto) {
			if (!result)
				db_free(&dbv);
			if (!(result = db_get_ws(hContact, szProto, "YMsg", &dbv)) && mir_wstrlen(dbv.pwszVal) > 0)
				p->bStatusMsgValid = STATUSMSG_YIM;
			else if (!(result = db_get_ws(hContact, szProto, "StatusDescr", &dbv)) && mir_wstrlen(dbv.pwszVal) > 0)
				p->bStatusMsgValid = STATUSMSG_GG;
			else if (!(result = db_get_ws(hContact, szProto, "XStatusMsg", &dbv)) && mir_wstrlen(dbv.pwszVal) > 0)
				p->bStatusMsgValid = STATUSMSG_XSTATUS;
		}
	}

	if (p->bStatusMsgValid == STATUSMSG_NOTFOUND) { // no status msg, consider xstatus name (if available)
		if (!result)
			db_free(&dbv);
		result = db_get_ws(hContact, szProto, "XStatusName", &dbv);
		if (!result && mir_wstrlen(dbv.pwszVal) > 1) {
			size_t iLen = mir_wstrlen(dbv.pwszVal);
			p->bStatusMsgValid = STATUSMSG_XSTATUSNAME;
			p->statusMsg = (wchar_t *)realloc(p->statusMsg, (iLen + 2) * sizeof(wchar_t));
			wcsncpy(p->statusMsg, dbv.pwszVal, iLen + 1);
		}
		else {
			int xStatus;
			WPARAM xStatus2;
			wchar_t xStatusName[128];

			CUSTOM_STATUS cst = { sizeof(cst) };
			cst.flags = CSSF_MASK_STATUS;
			cst.status = &xStatus;
			if (ProtoServiceExists(szProto, PS_GETCUSTOMSTATUSEX) && !CallProtoService(szProto, PS_GETCUSTOMSTATUSEX, hContact, (LPARAM)&cst) && xStatus > 0) {
				cst.flags = CSSF_MASK_NAME | CSSF_DEFAULT_NAME | CSSF_UNICODE;
				cst.wParam = &xStatus2;
				cst.ptszName = xStatusName;
				if (!CallProtoService(szProto, PS_GETCUSTOMSTATUSEX, hContact, (LPARAM)&cst)) {
					wchar_t *szwXstatusName = TranslateW(xStatusName);
					p->statusMsg = (wchar_t *)realloc(p->statusMsg, (mir_wstrlen(szwXstatusName) + 2) * sizeof(wchar_t));
					wcsncpy(p->statusMsg, szwXstatusName, mir_wstrlen(szwXstatusName) + 1);
					p->bStatusMsgValid = STATUSMSG_XSTATUSNAME;
				}
			}
		}
	}

	if (p->bStatusMsgValid > STATUSMSG_XSTATUSNAME) {
		int j = 0;
		p->statusMsg = (wchar_t *)realloc(p->statusMsg, (mir_wstrlen(dbv.pwszVal) + 2) * sizeof(wchar_t));
		for (int i = 0; dbv.pwszVal[i]; i++) {
			if (dbv.pwszVal[i] == (wchar_t)0x0d)
				continue;
			p->statusMsg[j] = dbv.pwszVal[i] == (wchar_t)0x0a ? (wchar_t)' ' : dbv.pwszVal[i];
			j++;
		}
		p->statusMsg[j] = 0;
	}
	if (!result)
		db_free(&dbv);

	if (p->bStatusMsgValid != STATUSMSG_NOTFOUND) {
		uint16_t infoTypeC2[12];
		memset(infoTypeC2, 0, sizeof(infoTypeC2));
		int iLen = min((int)mir_wstrlen(p->statusMsg), 10);
		GetStringTypeW(CT_CTYPE2, p->statusMsg, iLen, infoTypeC2);
		p->dwCFlags &= ~ECF_RTLSTATUSMSG;
		for (int i = 0; i < 10; i++) {
			if (infoTypeC2[i] == C2_RIGHTTOLEFT) {
				p->dwCFlags |= ECF_RTLSTATUSMSG;
				break;
			}
		}
	}

	if (p->hTimeZone == nullptr)
		TZ_LoadTimeZone(hContact, p);
	return p->bStatusMsgValid;
}

void ReloadExtraInfo(MCONTACT hContact)
{
	if (hContact && g_clistApi.hwndContactTree) {
		TExtraCache *p = cfg::getCache(hContact, nullptr);
		if (p) {
			TZ_LoadTimeZone(hContact, p);
			InvalidateRect(g_clistApi.hwndContactTree, nullptr, FALSE);
		}
	}
}

/*
 * autodetect RTL property of the nickname, evaluates the first 10 characters of the nickname only
 */


void RTL_DetectAndSet(ClcContact *contact, MCONTACT hContact)
{
	uint16_t infoTypeC2[12];
	wchar_t *szText;
	TExtraCache *p;

	memset(infoTypeC2, 0, sizeof(infoTypeC2));

	if (contact == nullptr) {
		szText = Clist_GetContactDisplayName(hContact);
		p = cfg::getCache(hContact, nullptr);
	}
	else {
		szText = contact->szText;
		p = contact->pExtra;
	}
	if (p) {
		int iLen = min((int)mir_wstrlen(szText), 10);
		GetStringTypeW(CT_CTYPE2, szText, iLen, infoTypeC2);
		p->dwCFlags &= ~ECF_RTLNICK;
		for (int i = 0; i < 10; i++) {
			if (infoTypeC2[i] == C2_RIGHTTOLEFT) {
				p->dwCFlags |= ECF_RTLNICK;
				return;
			}
		}
	}
}

void RTL_DetectGroupName(ClcContact *group)
{
	uint16_t infoTypeC2[12];

	group->isRtl = 0;

	if (group->szText) {
		int iLen = min((int)mir_wstrlen(group->szText), 10);
		GetStringTypeW(CT_CTYPE2, group->szText, iLen, infoTypeC2);
		for (int i = 0; i < 10; i++) {
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
	if (dat->bisEmbedded || contact == nullptr)
		return;

	if (contact->pce->szProto == nullptr || contact->hContact == 0)
		return;

	TExtraCache *p = contact->pExtra;
	if (p == nullptr)
		return;

	p->msgFrequency = g_plugin.getDword(contact->hContact, "mf_freq", 0x7fffffff);
	if (p->valid)
		return;

	p->valid = TRUE;
	p->isChatRoom = Contact::IsGroupChat(contact->hContact, contact->pce->szProto);
}

void LoadSkinItemToCache(TExtraCache *cEntry)
{
	MCONTACT hContact = cEntry->hContact;

	if (db_get_b(hContact, "EXTBK", "VALID", 0)) {
		if (cEntry->status_item == nullptr)
			cEntry->status_item = reinterpret_cast<StatusItems_t *>(malloc(sizeof(StatusItems_t)));
		memset(cEntry->status_item, 0, sizeof(StatusItems_t));
		strncpy_s(cEntry->status_item->szName, "{--CONTACT--}", _TRUNCATE); // mark as "per contact" item
		cEntry->status_item->IGNORED = 0;

		cEntry->status_item->TEXTCOLOR = db_get_dw(hContact, "EXTBK", "TEXT", RGB(20, 20, 20));
		cEntry->status_item->COLOR = db_get_dw(hContact, "EXTBK", "COLOR1", RGB(224, 224, 224));
		cEntry->status_item->COLOR2 = db_get_dw(hContact, "EXTBK", "COLOR2", RGB(224, 224, 224));
		cEntry->status_item->ALPHA = (uint8_t)db_get_b(hContact, "EXTBK", "ALPHA", 100);

		cEntry->status_item->MARGIN_LEFT = (uint32_t)db_get_b(hContact, "EXTBK", "LEFT", 0);
		cEntry->status_item->MARGIN_RIGHT = (uint32_t)db_get_b(hContact, "EXTBK", "RIGHT", 0);
		cEntry->status_item->MARGIN_TOP = (uint32_t)db_get_b(hContact, "EXTBK", "TOP", 0);
		cEntry->status_item->MARGIN_BOTTOM = (uint32_t)db_get_b(hContact, "EXTBK", "BOTTOM", 0);

		cEntry->status_item->COLOR2_TRANSPARENT = (uint8_t)db_get_b(hContact, "EXTBK", "TRANS", 1);
		cEntry->status_item->BORDERSTYLE = db_get_dw(hContact, "EXTBK", "BDR", 0);

		cEntry->status_item->CORNER = db_get_b(hContact, "EXTBK", "CORNER", 0);
		cEntry->status_item->GRADIENT = db_get_b(hContact, "EXTBK", "GRAD", 0);
	}
	else if (cEntry->status_item) {
		free(cEntry->status_item);
		cEntry->status_item = nullptr;
	}
}

/*
* checks the currently active view mode filter and returns true, if the contact should be hidden
* if no view mode is active, it returns the CList/Hidden setting
* also cares about sub contacts (if meta is active)
*/

int CLVM_GetContactHiddenStatus(MCONTACT hContact, char *szProto, struct ClcData *dat)
{
	int dbHidden = Contact::IsHidden(hContact); // default hidden state, always respect it.

	// always hide subcontacts (but show them on embedded contact lists)
	if (dat != nullptr && dat->bHideSubcontacts && cfg::dat.bMetaEnabled && db_mc_isSub(hContact))
		return 1;

	if (!cfg::dat.bFilterEffective)
		return dbHidden;

	if (szProto == nullptr)
		szProto = Proto_GetBaseAccountName(hContact);
	// check stickies first (priority), only if we really have stickies defined (CLVM_STICKY_CONTACTS is set).
	if (cfg::dat.bFilterEffective & CLVM_STICKY_CONTACTS) {
		uint32_t dwLocalMask = db_get_dw(hContact, "CLVM", cfg::dat.current_viewmode, 0);
		if (dwLocalMask != 0) {
			if (cfg::dat.bFilterEffective & CLVM_FILTER_STICKYSTATUS) {
				uint16_t wStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
				return !((1 << (wStatus - ID_STATUS_OFFLINE)) & HIWORD(dwLocalMask));
			}
			return 0;
		}
	}

	// check the proto, use it as a base filter result for all further checks
	int filterResult = 1;
	if (cfg::dat.bFilterEffective & CLVM_FILTER_PROTOS) {
		char szTemp[64];
		mir_snprintf(szTemp, "%s|", szProto);
		filterResult = strstr(cfg::dat.protoFilter, szTemp) ? 1 : 0;
	}

	if (cfg::dat.bFilterEffective & CLVM_FILTER_GROUPS) {
		ptrW tszGroup(Clist_GetGroup(hContact));
		if (tszGroup != NULL) {
			wchar_t szGroupMask[256];
			mir_snwprintf(szGroupMask, L"%s|", tszGroup.get());
			int bHasGroup = wcsstr(cfg::dat.groupFilter, szGroupMask) ? 1 : 0;
			filterResult = (cfg::dat.filterFlags & CLVM_PROTOGROUP_OP) ? (filterResult | bHasGroup) : (filterResult & bHasGroup);
		}
		else if (cfg::dat.filterFlags & CLVM_INCLUDED_UNGROUPED)
			filterResult = (cfg::dat.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 1;
		else
			filterResult = (cfg::dat.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 0;
	}

	if (cfg::dat.bFilterEffective & CLVM_FILTER_STATUS) {
		uint16_t wStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		filterResult = (cfg::dat.filterFlags & CLVM_GROUPSTATUS_OP) ? ((filterResult | ((1 << (wStatus - ID_STATUS_OFFLINE)) & cfg::dat.statusMaskFilter ? 1 : 0))) : (filterResult & ((1 << (wStatus - ID_STATUS_OFFLINE)) & cfg::dat.statusMaskFilter ? 1 : 0));
	}

	if (cfg::dat.bFilterEffective & CLVM_FILTER_LASTMSG) {
		TExtraCache *p = cfg::getCache(hContact, szProto);
		if (p) {
			uint32_t now = cfg::dat.t_now;
			now -= cfg::dat.lastMsgFilter;
			if (cfg::dat.bFilterEffective & CLVM_FILTER_LASTMSG_OLDERTHAN)
				filterResult = filterResult & (p->dwLastMsgTime < now);
			else if (cfg::dat.bFilterEffective & CLVM_FILTER_LASTMSG_NEWERTHAN)
				filterResult = filterResult & (p->dwLastMsgTime > now);
		}
	}
	return (dbHidden | !filterResult);
}
