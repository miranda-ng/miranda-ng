/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org),
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
#include "modern_clist.h"
#include "modern_commonprototypes.h"
#include "modern_awaymsg.h"

void InsertContactIntoTree(MCONTACT hContact, int status);

TCHAR* UnknownConctactTranslatedName = NULL;

void InitDisplayNameCache(void)
{
	InitAwayMsgModule();
}

void FreeDisplayNameCache()
{
	UninitAwayMsgModule();
}

void cliFreeCacheItem(ClcCacheEntry *p)
{
	mir_free_and_nil(p->tszName);
	mir_free_and_nil(p->tszGroup);
	mir_free_and_nil(p->szSecondLineText);
	mir_free_and_nil(p->szThirdLineText);
	p->ssSecondLine.DestroySmileyList();
	p->ssThirdLine.DestroySmileyList();
}

void cliCheckCacheItem(ClcCacheEntry *pdnce)
{
	if (pdnce == NULL)
		return;

	if (pdnce->hContact == NULL) { //selfcontact
		if (!pdnce->tszName)
			pdnce->tszName = pcli->pfnGetContactDisplayName(NULL, GCDNF_NOCACHE);
		return;
	}

	if (pdnce->m_pszProto == NULL) {
		pdnce->m_pszProto = GetContactProto(pdnce->hContact);
		if (pdnce->m_pszProto && pdnce->tszName)
			mir_free_and_nil(pdnce->tszName);
	}

	if (pdnce->tszName == NULL) {
		pdnce->tszName = pcli->pfnGetContactDisplayName(pdnce->hContact, GCDNF_NOCACHE);
		pdnce->m_bIsUnknown = !mir_tstrcmp(pdnce->tszName, UnknownConctactTranslatedName);
	}

	if (pdnce->m_iStatus == 0) //very strange look status sort is broken let always reread status
		pdnce->m_iStatus = GetStatusForContact(pdnce->hContact, pdnce->m_pszProto);

	if (pdnce->tszGroup == NULL) {
		pdnce->tszGroup = db_get_tsa(pdnce->hContact, "CList", "Group");
		if (pdnce->tszGroup == NULL)
			pdnce->tszGroup = mir_tstrdup(_T(""));
	}

	// this variable isn't filled inside cliCreateCacheItem() because the filter could be changed dynamically
	if (pdnce->dwLastMsgTime == -1 && g_CluiData.bFilterEffective & (CLVM_FILTER_LASTMSG | CLVM_FILTER_LASTMSG_NEWERTHAN | CLVM_FILTER_LASTMSG_OLDERTHAN)) {
		pdnce->dwLastMsgTime = db_get_dw(pdnce->hContact, "CList", "mf_lastmsg", 0);
		if (pdnce->dwLastMsgTime == 0)
			pdnce->dwLastMsgTime = CompareContacts2_getLMTime(pdnce->hContact);
	}
}

int GetStatusForContact(MCONTACT hContact, char *szProto)
{
	return (szProto) ? db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) : ID_STATUS_OFFLINE;
}

int GetContactInfosForSort(MCONTACT hContact, char **Proto, TCHAR **Name, int *Status)
{
	ClcCacheEntry *cacheEntry = pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry != NULL) {
		if (Proto != NULL)  *Proto = cacheEntry->m_pszProto;
		if (Name != NULL)   *Name = cacheEntry->tszName;
		if (Status != NULL) *Status = cacheEntry->m_iStatus;
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
		pcli->pfnClcBroadcast(INTM_STATUSCHANGED, hMeta, 0);

	return 0;
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (MirandaExiting() || !pcli || hContact == NULL)
		return 0;

	ClcCacheEntry *pdnce = pcli->pfnGetCacheEntry(hContact);
	if (pdnce == NULL) {
		TRACE("!!! Very bad pdnce not found.");
		return 0;
	}

	if (pdnce->m_pszProto == NULL)
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

			if ((db_get_w(NULL, "CList", "SecondLineType", 0) == TEXT_STATUS_MESSAGE || db_get_w(NULL, "CList", "ThirdLineType", 0) == TEXT_STATUS_MESSAGE) && pdnce->hContact && pdnce->m_pszProto)
				amRequestAwayMsg(hContact);

			pcli->pfnClcBroadcast(INTM_STATUSCHANGED, hContact, 0);
		}
		else if (!strcmp(cws->szModule, META_PROTO) && !memcmp(cws->szSetting, "Status", 6)) { // Status0..N for metacontacts
			if (pcli->hwndContactTree && g_flag_bOnModulesLoadedCalled)
				pcli->pfnInitAutoRebuild(pcli->hwndContactTree);

			if ((db_get_w(NULL, "CList", "SecondLineType", SETTING_SECONDLINE_TYPE_DEFAULT) == TEXT_STATUS_MESSAGE || db_get_w(NULL, "CList", "ThirdLineType", SETTING_THIRDLINE_TYPE_DEFAULT) == TEXT_STATUS_MESSAGE) && pdnce->hContact && pdnce->m_pszProto)
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
			pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);

		else if (!strcmp(cws->szSetting, "NotOnList"))
			pdnce->NotOnList = cws->value.bVal;

		else if (!strcmp(cws->szSetting, "Expanded"))
			pdnce->IsExpanded = cws->value.bVal;

		else if (!strcmp(cws->szSetting, "Hidden")) {
			pdnce->bIsHidden = cws->value.bVal;
			if (cws->value.type == DBVT_DELETED || cws->value.bVal == 0)
				pcli->pfnChangeContactIcon(hContact, pcli->pfnIconFromStatusMode(pdnce->m_pszProto, pdnce->getStatus(), hContact));

			pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
		}
		else if (!strcmp(cws->szSetting, "noOffline")) {
			pdnce->m_bNoHiddenOffline = cws->value.bVal;
			pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
		}
	}
	else if (!strcmp(cws->szModule, "Protocol")) {
		if (!strcmp(cws->szSetting, "p")) {
			pdnce->m_pszProto = GetContactProto(hContact);
			char *szProto = (cws->value.type == DBVT_DELETED) ? NULL : cws->value.pszVal;
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
