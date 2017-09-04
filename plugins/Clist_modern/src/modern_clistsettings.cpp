/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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

void InsertContactIntoTree(MCONTACT hContact, int status);

wchar_t* UnknownConctactTranslatedName = nullptr;

void cliFreeCacheItem(ClcCacheEntry *p)
{
	mir_free_and_nil(p->szSecondLineText);
	mir_free_and_nil(p->szThirdLineText);
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
			pdnce->tszName = pcli->pfnGetContactDisplayName(0, GCDNF_NOCACHE);
		return;
	}

	if (pdnce->m_pszProto == nullptr) {
		pdnce->m_pszProto = GetContactProto(pdnce->hContact);
		if (pdnce->m_pszProto && pdnce->tszName)
			mir_free_and_nil(pdnce->tszName);
	}

	if (pdnce->tszName == nullptr) {
		pdnce->tszName = pcli->pfnGetContactDisplayName(pdnce->hContact, GCDNF_NOCACHE);
		pdnce->m_bIsUnknown = !mir_wstrcmp(pdnce->tszName, UnknownConctactTranslatedName);
	}

	if (pdnce->m_iStatus == 0) //very strange look status sort is broken let always reread status
		pdnce->m_iStatus = GetStatusForContact(pdnce->hContact, pdnce->m_pszProto);

	// this variable isn't filled inside cliCreateCacheItem() because the filter could be changed dynamically
	if (pdnce->dwLastMsgTime == -1 && g_CluiData.bFilterEffective & (CLVM_FILTER_LASTMSG | CLVM_FILTER_LASTMSG_NEWERTHAN | CLVM_FILTER_LASTMSG_OLDERTHAN)) {
		pdnce->dwLastMsgTime = db_get_dw(pdnce->hContact, "CList", "mf_lastmsg", 0);
		if (pdnce->dwLastMsgTime == 0)
			pdnce->dwLastMsgTime = CompareContacts2_getLMTime(pdnce->hContact);
	}

	corecli.pfnCheckCacheItem(pdnce);
}

int GetStatusForContact(MCONTACT hContact, char *szProto)
{
	return (szProto) ? db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) : ID_STATUS_OFFLINE;
}

int GetContactInfosForSort(MCONTACT hContact, char **Proto, wchar_t **Name, int *Status)
{
	ClcCacheEntry *cacheEntry = pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry != nullptr) {
		if (Proto != nullptr)  *Proto = cacheEntry->m_pszProto;
		if (Name != nullptr)   *Name = cacheEntry->tszName;
		if (Status != nullptr) *Status = cacheEntry->m_iStatus;
	}
	return 0;
}

int GetContactCachedStatus(MCONTACT hContact)
{
	return pcli->pfnGetCacheEntry(hContact)->getStatus();
}

int ContactAdded(WPARAM hContact, LPARAM)
{
	if (!MirandaExiting())
		pcli->pfnChangeContactIcon(hContact, pcli->pfnIconFromStatusMode(GetContactProto(hContact), ID_STATUS_OFFLINE, hContact));

	return 0;
}

int MetaStatusChanged(WPARAM hMeta, LPARAM)
{
	ClcCacheEntry *pdnce = pcli->pfnGetCacheEntry(hMeta);
	if (pdnce)
		Clist_Broadcast(INTM_STATUSCHANGED, hMeta, 0);

	return 0;
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (MirandaExiting() || !pcli || !hContact)
		return 0;

	ClcCacheEntry *pdnce = pcli->pfnGetCacheEntry(hContact);
	if (pdnce == nullptr) {
		TRACE("!!! Very bad pdnce not found.");
		return 0;
	}

	if (pdnce->m_pszProto == nullptr)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (!strcmp(cws->szModule, pdnce->m_pszProto)) {
		if (!strcmp(cws->szSetting, "Status")) {
			pdnce->m_iStatus = cws->value.wVal;
			if (pdnce->bIsHidden)
				return 0;

			if (cws->value.wVal == ID_STATUS_OFFLINE)
				if (g_CluiData.bRemoveAwayMessageForOffline)
					db_set_s(hContact, "CList", "StatusMsg", "");

			if ((db_get_w(0, "CList", "SecondLineType", 0) == TEXT_STATUS_MESSAGE || db_get_w(0, "CList", "ThirdLineType", 0) == TEXT_STATUS_MESSAGE) && pdnce->hContact && pdnce->m_pszProto)
				amRequestAwayMsg(hContact);

			Clist_Broadcast(INTM_STATUSCHANGED, hContact, 0);
		}
		else if (!strcmp(cws->szModule, META_PROTO) && !memcmp(cws->szSetting, "Status", 6)) { // Status0..N for metacontacts
			if (pcli->hwndContactTree && g_flag_bOnModulesLoadedCalled)
				pcli->pfnInitAutoRebuild(pcli->hwndContactTree);

			if ((db_get_w(0, "CList", "SecondLineType", SETTING_SECONDLINE_TYPE_DEFAULT) == TEXT_STATUS_MESSAGE || db_get_w(0, "CList", "ThirdLineType", SETTING_THIRDLINE_TYPE_DEFAULT) == TEXT_STATUS_MESSAGE) && pdnce->hContact && pdnce->m_pszProto)
				amRequestAwayMsg(hContact);
		}
		else if (!strcmp(cws->szSetting, "ApparentMode"))
			pdnce->ApparentMode = cws->value.wVal;
		else if (!strcmp(cws->szSetting, "IdleTS"))
			pdnce->IdleTS = cws->value.dVal;
		else if (!strcmp(cws->szSetting, "IsSubcontact")) {
			pdnce->m_bIsSub = (cws->value.type == DBVT_DELETED) ? false : cws->value.bVal != 0;
			pcli->pfnInitAutoRebuild(pcli->hwndContactTree);
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
				pcli->pfnChangeContactIcon(hContact, pcli->pfnIconFromStatusMode(pdnce->m_pszProto, pdnce->getStatus(), hContact));

			Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
		}
		else if (!strcmp(cws->szSetting, "noOffline")) {
			pdnce->m_bNoHiddenOffline = cws->value.bVal;
			Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
		}
	}
	else if (!strcmp(cws->szModule, "Protocol")) {
		if (!strcmp(cws->szSetting, "p")) {
			pdnce->m_pszProto = GetContactProto(hContact);
			char *szProto = (cws->value.type == DBVT_DELETED) ? nullptr : cws->value.pszVal;
			pcli->pfnChangeContactIcon(hContact, pcli->pfnIconFromStatusMode(szProto, pdnce->getStatus(), hContact));
		}
	}

	return 0;
}

int OnLoadLangpack(WPARAM, LPARAM)
{
	UnknownConctactTranslatedName = TranslateT("(Unknown contact)");
	return 0;
}
