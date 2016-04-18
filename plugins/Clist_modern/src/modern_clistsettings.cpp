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
#include "modern_gettextasync.h"

void InsertContactIntoTree(MCONTACT hContact, int status);
void CListSettings_FreeCacheItemDataOption(ClcCacheEntry *pDst, DWORD flag);

static int displayNameCacheSize;

LIST<ClcCacheEntry> clistCache(50, NumericKeySortT);

TCHAR* UnknownConctactTranslatedName = NULL;

void InitDisplayNameCache(void)
{
	InitCacheAsync();
	InitAwayMsgModule();
}

void FreeDisplayNameCache()
{
	UninitAwayMsgModule();

	for (int i = 0; i < clistCache.getCount(); i++) {
		pcli->pfnFreeCacheItem(clistCache[i]);
		mir_free(clistCache[i]);
	}
	clistCache.destroy();
}

ClcCacheEntry* cliGetCacheEntry(MCONTACT hContact)
{
	ClcCacheEntry *p;
	int idx = clistCache.getIndex((ClcCacheEntry*)&hContact);
	if (idx == -1) {
		if ((p = pcli->pfnCreateCacheItem(hContact)) != NULL) {
			clistCache.insert(p);
			pcli->pfnInvalidateDisplayNameCacheEntry(hContact);
		}
	}
	else p = clistCache[idx];
	pcli->pfnCheckCacheItem(p);
	return p;
}

void CListSettings_FreeCacheItemData(ClcCacheEntry *pDst)
{
	CListSettings_FreeCacheItemDataOption(pDst, CCI_ALL);
}

void CListSettings_FreeCacheItemDataOption(ClcCacheEntry *pDst, DWORD flag)
{
	if (!pDst)
		return;

	if (flag & CCI_NAME)
		mir_free_and_nil(pDst->tszName);

	if (flag & CCI_GROUP)
		mir_free_and_nil(pDst->tszGroup);

	if (flag & CCI_LINES) {
		mir_free_and_nil(pDst->szSecondLineText);
		mir_free_and_nil(pDst->szThirdLineText);
		pDst->ssSecondLine.DestroySmileyList();
		pDst->ssThirdLine.DestroySmileyList();
	}
}

int CListSettings_GetCopyFromCache(ClcCacheEntry *pDest, DWORD flag);
int CListSettings_SetToCache(ClcCacheEntry *pSrc, DWORD flag);

void CListSettings_CopyCacheItems(ClcCacheEntry *pDst, ClcCacheEntry *pSrc, DWORD flag)
{
	if (!pDst || !pSrc) return;
	CListSettings_FreeCacheItemDataOption(pDst, flag);

	if (flag & CCI_NAME)   pDst->tszName = mir_tstrdup(pSrc->tszName);
	if (flag & CCI_GROUP)  pDst->tszGroup = mir_tstrdup(pSrc->tszGroup);
	if (flag & CCI_PROTO)  pDst->m_pszProto = pSrc->m_pszProto;
	if (flag & CCI_STATUS) pDst->m_iStatus = pSrc->m_iStatus;

	if (flag & CCI_LINES) {
		mir_free(pDst->szThirdLineText);
		pDst->szThirdLineText = mir_tstrdup(pSrc->szThirdLineText);

		mir_free(pDst->szSecondLineText);
		pDst->szSecondLineText = mir_tstrdup(pSrc->szSecondLineText);

		pDst->ssThirdLine = pSrc->ssThirdLine;
		pDst->ssSecondLine = pSrc->ssSecondLine;
	}

	if (flag & CCI_TIME)
		pDst->hTimeZone = pSrc->hTimeZone;

	if (flag & CCI_OTHER) {
		pDst->bIsHidden = pSrc->bIsHidden;
		pDst->m_bNoHiddenOffline = pSrc->m_bNoHiddenOffline;

		pDst->m_bIsSub = pSrc->m_bIsSub;
		pDst->ApparentMode = pSrc->ApparentMode;
		pDst->NotOnList = pSrc->NotOnList;
		pDst->IdleTS = pSrc->IdleTS;
		pDst->ClcContact = pSrc->ClcContact;
		pDst->IsExpanded = pSrc->IsExpanded;
	}
}

int CListSettings_GetCopyFromCache(ClcCacheEntry *pDest, DWORD flag)
{
	if (!pDest || !pDest->hContact)
		return -1;

	ClcCacheEntry *pSource = pcli->pfnGetCacheEntry(pDest->hContact);
	if (!pSource)
		return -1;

	CListSettings_CopyCacheItems(pDest, pSource, flag);
	return 0;
}

int CListSettings_SetToCache(ClcCacheEntry *pSrc, DWORD flag)
{
	if (!pSrc || !pSrc->hContact)
		return -1;

	ClcCacheEntry *pDst = pcli->pfnGetCacheEntry(pSrc->hContact);
	if (!pDst)
		return -1;

	CListSettings_CopyCacheItems(pDst, pSrc, flag);
	return 0;
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

	if (pdnce->bIsHidden == -1)
		pdnce->bIsHidden = db_get_b(pdnce->hContact, "CList", "Hidden", 0);

	pdnce->m_bIsSub = db_mc_isSub(pdnce->hContact) != 0;

	if (pdnce->m_bNoHiddenOffline == -1)
		pdnce->m_bNoHiddenOffline = db_get_b(pdnce->hContact, "CList", "noOffline", 0);

	if (pdnce->IdleTS == -1)
		pdnce->IdleTS = db_get_dw(pdnce->hContact, pdnce->m_pszProto, "IdleTS", 0);

	if (pdnce->ApparentMode == -1)
		pdnce->ApparentMode = db_get_w(pdnce->hContact, pdnce->m_pszProto, "ApparentMode", 0);

	if (pdnce->NotOnList == -1)
		pdnce->NotOnList = db_get_b(pdnce->hContact, "CList", "NotOnList", 0);

	if (pdnce->IsExpanded == -1)
		pdnce->IsExpanded = db_get_b(pdnce->hContact, "CList", "Expanded", 0);

	if (pdnce->dwLastMsgTime == -1 && g_CluiData.bFilterEffective & (CLVM_FILTER_LASTMSG | CLVM_FILTER_LASTMSG_NEWERTHAN | CLVM_FILTER_LASTMSG_OLDERTHAN)) {
		pdnce->dwLastMsgTime = db_get_dw(pdnce->hContact, "CList", "mf_lastmsg", 0);
		if (pdnce->dwLastMsgTime == 0)
			pdnce->dwLastMsgTime = CompareContacts2_getLMTime(pdnce->hContact);
	}
}

void IvalidateDisplayNameCache()
{
	for (int i = 0; i < clistCache.getCount(); i++) {
		ClcCacheEntry *pdnce = (ClcCacheEntry *)clistCache[i];
		pdnce->ssSecondLine.DestroySmileyList();
		mir_free_and_nil(pdnce->szSecondLineText);
		pdnce->ssThirdLine.DestroySmileyList();
		mir_free_and_nil(pdnce->szThirdLineText);
		pdnce->ssSecondLine.iMaxSmileyHeight = 0;
		pdnce->ssThirdLine.iMaxSmileyHeight = 0;
		pdnce->hTimeZone = NULL;
		pdnce->dwLastMsgTime = -1;
		Cache_GetTimezone(NULL, pdnce->hContact);
	}
}

char* GetContactCachedProtocol(MCONTACT hContact)
{
	ClcCacheEntry *cacheEntry = pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry && cacheEntry->m_pszProto)
		return cacheEntry->m_pszProto;

	return NULL;
}

int GetStatusForContact(MCONTACT hContact, char *szProto)
{
	return (szProto) ? db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) : ID_STATUS_OFFLINE;
}

int GetContactInfosForSort(MCONTACT hContact, char **Proto, TCHAR **Name, int *Status)
{
	ClcCacheEntry *cacheEntry = NULL;
	cacheEntry = pcli->pfnGetCacheEntry(hContact);
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
		pcli->pfnChangeContactIcon(hContact, pcli->pfnIconFromStatusMode((char*)GetContactCachedProtocol(hContact), ID_STATUS_OFFLINE, hContact), 1); ///by FYR

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
			pcli->pfnChangeContactIcon(hContact, pcli->pfnIconFromStatusMode(cws->szModule, cws->value.wVal, hContact), 0); //by FYR
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
			if (cws->value.type == DBVT_DELETED || cws->value.bVal == 0) {
				char *szProto = GetContactProto(hContact);
				pcli->pfnChangeContactIcon(hContact, pcli->pfnIconFromStatusMode(szProto,
					szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact), 1);  //by FYR
			}
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
			pcli->pfnChangeContactIcon(hContact, pcli->pfnIconFromStatusMode(szProto,
				szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact), 0);
		}
	}

	return 0;
}

int OnLoadLangpack(WPARAM, LPARAM)
{
	UnknownConctactTranslatedName = TranslateT("(Unknown contact)");
	return 0;
}
