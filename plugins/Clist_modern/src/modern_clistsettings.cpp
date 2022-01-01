/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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
#include "modern_awaymsg.h"

void cliFreeCacheItem(ClcCacheEntry *p)
{
	replaceStrW(p->szSecondLineText, nullptr);
	replaceStrW(p->szThirdLineText, nullptr);
	p->ssSecondLine.DestroySmileyList();
	p->ssThirdLine.DestroySmileyList();

	corecli.pfnFreeCacheItem(p);
}

void cliCheckCacheItem(ClcCacheEntry *pdnce)
{
	if (pdnce == nullptr)
		return;

	if (pdnce->hContact == 0) { //selfcontact
		if (!pdnce->tszName)
			pdnce->tszName = Clist_GetContactDisplayName(0, GCDNF_NOCACHE);
		return;
	}

	if (pdnce->szProto == nullptr) {
		pdnce->szProto = Proto_GetBaseAccountName(pdnce->hContact);
		if (pdnce->szProto && pdnce->tszName)
			replaceStrW(pdnce->tszName, nullptr);
	}

	if (pdnce->tszName == nullptr) {
		pdnce->tszName = Clist_GetContactDisplayName(pdnce->hContact, GCDNF_NOCACHE | GCDNF_NOUNKNOWN);
		if (!pdnce->tszName) {
			pdnce->tszName = mir_wstrdup(TranslateT("(Unknown contact)"));
			pdnce->m_bIsUnknown = true;
		}
		else pdnce->m_bIsUnknown = false;
	}

	if (pdnce->m_iStatus == 0) //very strange look status sort is broken let always reread status
		pdnce->m_iStatus = GetStatusForContact(pdnce->hContact, pdnce->szProto);

	// this variable isn't filled inside cliCreateCacheItem() because the filter could be changed dynamically
	if (pdnce->dwLastMsgTime == -1 && g_CluiData.bFilterEffective & (CLVM_FILTER_LASTMSG | CLVM_FILTER_LASTMSG_NEWERTHAN | CLVM_FILTER_LASTMSG_OLDERTHAN))
		pdnce->dwLastMsgTime = CompareContacts2_getLMTime(pdnce->hContact);

	corecli.pfnCheckCacheItem(pdnce);
}

int GetStatusForContact(MCONTACT hContact, char *szProto)
{
	return (szProto) ? db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) : ID_STATUS_OFFLINE;
}

int GetContactInfosForSort(MCONTACT hContact, char **Proto, wchar_t **Name, int *Status)
{
	ClcCacheEntry *cacheEntry = Clist_GetCacheEntry(hContact);
	if (cacheEntry != nullptr) {
		if (Proto != nullptr)  *Proto = cacheEntry->szProto;
		if (Name != nullptr)   *Name = cacheEntry->tszName;
		if (Status != nullptr) *Status = cacheEntry->m_iStatus;
	}
	return 0;
}

int GetContactCachedStatus(MCONTACT hContact)
{
	return Clist_GetCacheEntry(hContact)->getStatus();
}

int ContactAdded(WPARAM hContact, LPARAM)
{
	if (!MirandaExiting())
		Clist_ChangeContactIcon(hContact, g_clistApi.pfnIconFromStatusMode(Proto_GetBaseAccountName(hContact), ID_STATUS_OFFLINE, hContact));

	return 0;
}

int MetaStatusChanged(WPARAM hMeta, LPARAM)
{
	ClcCacheEntry *pdnce = Clist_GetCacheEntry(hMeta);
	if (pdnce)
		Clist_Broadcast(INTM_STATUSCHANGED, hMeta, 0);

	return 0;
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (MirandaExiting() || !hContact)
		return 0;

	ClcCacheEntry *pdnce = Clist_GetCacheEntry(hContact);
	if (pdnce == nullptr) {
		TRACE("!!! Very bad pdnce not found.");
		return 0;
	}

	if (pdnce->szProto == nullptr)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (!strcmp(cws->szModule, pdnce->szProto)) {
		if (!strcmp(cws->szSetting, "Status")) {
			pdnce->m_iStatus = cws->value.wVal;
			pdnce->dwLastOnlineTime = time(0);
			if (pdnce->bIsHidden)
				return 0;

			if (cws->value.wVal == ID_STATUS_OFFLINE)
				if (g_CluiData.bRemoveAwayMessageForOffline)
					g_plugin.setString(hContact, "StatusMsg", "");

			if ((g_plugin.secondLine.getType() == TEXT_STATUS_MESSAGE || g_plugin.thirdLine.getType() == TEXT_STATUS_MESSAGE) && pdnce->hContact && pdnce->szProto)
				amRequestAwayMsg(hContact);

			Clist_Broadcast(INTM_STATUSCHANGED, hContact, 0);
		}
		else if (!strcmp(cws->szModule, META_PROTO) && !memcmp(cws->szSetting, "Status", 6)) { // Status0..N for metacontacts
			if (g_clistApi.hwndContactTree && g_bOnModulesLoadedCalled)
				Clist_InitAutoRebuild(g_clistApi.hwndContactTree);

			if ((g_plugin.secondLine.getType() == TEXT_STATUS_MESSAGE || g_plugin.thirdLine.getType() == TEXT_STATUS_MESSAGE) && pdnce->hContact && pdnce->szProto)
				amRequestAwayMsg(hContact);
		}
		else if (!strcmp(cws->szSetting, "ApparentMode"))
			pdnce->ApparentMode = cws->value.wVal;
		else if (!strcmp(cws->szSetting, "IdleTS"))
			pdnce->IdleTS = cws->value.dVal;
		else if (!strcmp(cws->szSetting, "IsSubcontact")) {
			pdnce->m_bIsSub = (cws->value.type == DBVT_DELETED) ? false : cws->value.bVal != 0;
			Clist_InitAutoRebuild(g_clistApi.hwndContactTree);
		}
	}

	if (!strcmp(cws->szModule, "CList")) {
		// name is null or (setting is myhandle)
		if (!strcmp(cws->szSetting, "Rate"))
			Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);

		else if (!strcmp(cws->szSetting, "NotOnList"))
			pdnce->NotOnList = cws->value.bVal;

		else if (!strcmp(cws->szSetting, "Expanded"))
			pdnce->IsExpanded = cws->value.bVal;

		else if (!strcmp(cws->szSetting, "Hidden")) {
			pdnce->bIsHidden = cws->value.bVal;
			if (cws->value.type == DBVT_DELETED || cws->value.bVal == 0)
				Clist_ChangeContactIcon(hContact, g_clistApi.pfnIconFromStatusMode(pdnce->szProto, pdnce->getStatus(), hContact));

			Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
		}
		else if (!strcmp(cws->szSetting, "noOffline")) {
			pdnce->m_bNoHiddenOffline = cws->value.bVal;
			Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
		}
	}
	else if (!strcmp(cws->szModule, "Protocol")) {
		if (!strcmp(cws->szSetting, "p")) {
			pdnce->szProto = Proto_GetBaseAccountName(hContact);
			char *szProto = (cws->value.type == DBVT_DELETED) ? nullptr : cws->value.pszVal;
			Clist_ChangeContactIcon(hContact, g_clistApi.pfnIconFromStatusMode(szProto, pdnce->getStatus(), hContact));
		}
	}

	return 0;
}

int OnLoadLangpack(WPARAM, LPARAM)
{
	if (!MirandaLoading())
		ApplyViewMode(g_CluiData.current_viewmode);
	return 0;
}
