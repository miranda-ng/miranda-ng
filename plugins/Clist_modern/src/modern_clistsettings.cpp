/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_clist.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_awaymsg.h"

void InsertContactIntoTree(MCONTACT hContact, int status);
void CListSettings_FreeCacheItemDataOption(ClcCacheEntry *pDst, DWORD flag);

static int displayNameCacheSize;

LIST<ClcCacheEntry> clistCache(50, NumericKeySortT);

int    GetStatusForContact(MCONTACT hContact, char *szProto);
TCHAR* UnknownConctactTranslatedName = NULL;

void InvalidateDNCEbyPointer(MCONTACT hContact, ClcCacheEntry *pdnce, int SettingType);

void InitCacheAsync();
void UninitCacheAsync();

void InitDisplayNameCache(void)
{
	InitCacheAsync();
	InitAwayMsgModule();
}

void FreeDisplayNameCache()
{
	UninitCacheAsync();
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
		pDst->freeName();

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

	if (flag & CCI_NAME) {
		pDst->isUnknown = pSrc->isUnknown;
		if (pSrc->isUnknown)
			pDst->tszName = pSrc->tszName;
		else
			pDst->tszName = mir_tstrdup(pSrc->tszName);
	}

	if (flag & CCI_GROUP)  pDst->tszGroup = mir_tstrdup(pSrc->tszGroup);
	if (flag & CCI_PROTO)  pDst->m_cache_cszProto = pSrc->m_cache_cszProto;
	if (flag & CCI_STATUS) pDst->m_cache_nStatus = pSrc->m_cache_nStatus;

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
		pDst->m_cache_nNoHiddenOffline = pSrc->m_cache_nNoHiddenOffline;
		pDst->m_bProtoNotExists = pSrc->m_bProtoNotExists;

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
	p->freeName();
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
			pdnce->getName();
		return;
	}

	if (pdnce->m_cache_cszProto == NULL && pdnce->m_bProtoNotExists == FALSE) {
		pdnce->m_cache_cszProto = GetContactProto(pdnce->hContact);
		if (pdnce->m_cache_cszProto == NULL)
			pdnce->m_bProtoNotExists = FALSE;
		else if (pdnce->m_cache_cszProto && pdnce->tszName)
			pdnce->freeName();
	}

	if (pdnce->tszName == NULL)
		pdnce->getName();

	else if (pdnce->isUnknown && pdnce->m_cache_cszProto && pdnce->m_bProtoNotExists == TRUE && g_flag_bOnModulesLoadedCalled) {
		if (ProtoGetAccount(pdnce->m_cache_cszProto) == NULL) {
			pdnce->m_bProtoNotExists = FALSE;
			pdnce->getName();
		}
	}

	if (pdnce->m_cache_nStatus == 0) //very strange look status sort is broken let always reread status
		pdnce->m_cache_nStatus = GetStatusForContact(pdnce->hContact, pdnce->m_cache_cszProto);

	if (pdnce->tszGroup == NULL) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ts(pdnce->hContact, "CList", "Group", &dbv)) {
			pdnce->tszGroup = mir_tstrdup(dbv.ptszVal);
			db_free(&dbv);
		}
		else pdnce->tszGroup = mir_tstrdup(_T(""));
	}

	if (pdnce->bIsHidden == -1)
		pdnce->bIsHidden = db_get_b(pdnce->hContact, "CList", "Hidden", 0);

	pdnce->m_bIsSub = db_mc_isSub(pdnce->hContact) != 0;

	if (pdnce->m_cache_nNoHiddenOffline == -1)
		pdnce->m_cache_nNoHiddenOffline = db_get_b(pdnce->hContact, "CList", "noOffline", 0);

	if (pdnce->IdleTS == -1)
		pdnce->IdleTS = db_get_dw(pdnce->hContact, pdnce->m_cache_cszProto, "IdleTS", 0);

	if (pdnce->ApparentMode == -1)
		pdnce->ApparentMode = db_get_w(pdnce->hContact, pdnce->m_cache_cszProto, "ApparentMode", 0);

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

void IvalidateDisplayNameCache(DWORD mode)
{
	for (int i = 0; i < clistCache.getCount(); i++) {
		ClcCacheEntry *pdnce = (ClcCacheEntry *)clistCache[i];
		if (mode & 16)
			InvalidateDNCEbyPointer(pdnce->hContact, pdnce, 16);
	}
}

void InvalidateDNCEbyPointer(MCONTACT hContact, ClcCacheEntry *pdnce, int SettingType)
{
	if (hContact == NULL || pdnce == NULL)
		return;

	if (SettingType == 16) {
		pdnce->ssSecondLine.DestroySmileyList();
		mir_free_and_nil(pdnce->szSecondLineText);
		pdnce->ssThirdLine.DestroySmileyList();
		mir_free_and_nil(pdnce->szThirdLineText);
		pdnce->ssSecondLine.iMaxSmileyHeight = 0;
		pdnce->ssThirdLine.iMaxSmileyHeight = 0;
		pdnce->hTimeZone = NULL;
		pdnce->dwLastMsgTime = -1;
		Cache_GetTimezone(NULL, pdnce->hContact);
		SettingType &= ~16;
	}

	if (SettingType >= DBVT_WCHAR) {
		pdnce->freeName();
		mir_free_and_nil(pdnce->tszGroup);
		pdnce->m_cache_cszProto = NULL;
		return;
	}

	if (SettingType == -1 || SettingType == DBVT_DELETED) {
		pdnce->freeName();
		mir_free_and_nil(pdnce->tszGroup);
		pdnce->m_cache_cszProto = NULL;
	}
	// in other cases clear all binary cache
	else pdnce->dwLastMsgTime = -1;

	pdnce->bIsHidden = -1;
	pdnce->m_bIsSub = pdnce->m_bProtoNotExists = false;
	pdnce->m_cache_nStatus = 0;
	pdnce->IdleTS = -1;
	pdnce->ApparentMode = -1;
	pdnce->NotOnList = -1;
	pdnce->isUnknown = FALSE;
	pdnce->m_cache_nNoHiddenOffline = -1;
	pdnce->IsExpanded = -1;
}

char *GetContactCachedProtocol(MCONTACT hContact)
{
	ClcCacheEntry *cacheEntry = NULL;
	cacheEntry = pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry && cacheEntry->m_cache_cszProto)
		return cacheEntry->m_cache_cszProto;

	return NULL;
}

int GetStatusForContact(MCONTACT hContact, char *szProto)
{
	return (szProto) ? (int)(db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE)) : ID_STATUS_OFFLINE;
}

void ClcCacheEntry::freeName()
{
	if (!isUnknown)
		mir_free(tszName);
	else
		isUnknown = false;
	tszName = NULL;
}

void ClcCacheEntry::getName()
{
	freeName();

	if (m_bProtoNotExists || !m_cache_cszProto) {
	LBL_Unknown:
		tszName = UnknownConctactTranslatedName;
		isUnknown = true;
		return;
	}

	tszName = pcli->pfnGetContactDisplayName(hContact, GCDNF_NOCACHE);
	if (!mir_tstrcmp(tszName, UnknownConctactTranslatedName)) {
		mir_free(tszName);
		goto LBL_Unknown;
	}

	isUnknown = false;
}

int GetContactInfosForSort(MCONTACT hContact, char **Proto, TCHAR **Name, int *Status)
{
	ClcCacheEntry *cacheEntry = NULL;
	cacheEntry = pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry != NULL) {
		if (Proto != NULL)  *Proto = cacheEntry->m_cache_cszProto;
		if (Name != NULL)   *Name = cacheEntry->tszName;
		if (Status != NULL) *Status = cacheEntry->m_cache_nStatus;
	}
	return (0);
};


int GetContactCachedStatus(MCONTACT hContact)
{
	ClcCacheEntry *cacheEntry = pcli->pfnGetCacheEntry(hContact);
	return pdnce___GetStatus(cacheEntry);
}

int ContactAdded(WPARAM hContact, LPARAM)
{
	if (!MirandaExiting()) {
		cli_ChangeContactIcon(hContact, pcli->pfnIconFromStatusMode((char*)GetContactCachedProtocol(hContact), ID_STATUS_OFFLINE, hContact), 1); ///by FYR
		pcli->pfnSortContacts();
	}
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

	if (pdnce->m_bProtoNotExists || !pdnce->m_cache_cszProto)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (!mir_strcmp(cws->szModule, pdnce->m_cache_cszProto)) {
		InvalidateDNCEbyPointer(hContact, pdnce, cws->value.type);

		if (!mir_strcmp(cws->szSetting, "IsSubcontact"))
			pcli->pfnInitAutoRebuild(pcli->hwndContactTree);

		if (!mir_strcmp(cws->szSetting, "Status") || wildcmp(cws->szSetting, "Status?")) {
			if (!mir_strcmp(cws->szModule, META_PROTO) && mir_strcmp(cws->szSetting, "Status")) {
				if (pcli->hwndContactTree && g_flag_bOnModulesLoadedCalled)
					pcli->pfnInitAutoRebuild(pcli->hwndContactTree);

				if ((db_get_w(NULL, "CList", "SecondLineType", SETTING_SECONDLINE_TYPE_DEFAULT) == TEXT_STATUS_MESSAGE || db_get_w(NULL, "CList", "ThirdLineType", SETTING_THIRDLINE_TYPE_DEFAULT) == TEXT_STATUS_MESSAGE) && pdnce->hContact && pdnce->m_cache_cszProto)
					amRequestAwayMsg(hContact);

				return 0;
			}

			if (pdnce->bIsHidden != 1) {
				pdnce->m_cache_nStatus = cws->value.wVal;
				if (cws->value.wVal == ID_STATUS_OFFLINE)
					if (g_CluiData.bRemoveAwayMessageForOffline)
						db_set_s(hContact, "CList", "StatusMsg", "");

				if ((db_get_w(NULL, "CList", "SecondLineType", 0) == TEXT_STATUS_MESSAGE || db_get_w(NULL, "CList", "ThirdLineType", 0) == TEXT_STATUS_MESSAGE) && pdnce->hContact && pdnce->m_cache_cszProto)
					amRequestAwayMsg(hContact);

				pcli->pfnClcBroadcast(INTM_STATUSCHANGED, hContact, 0);
				cli_ChangeContactIcon(hContact, pcli->pfnIconFromStatusMode(cws->szModule, cws->value.wVal, hContact), 0); //by FYR
				pcli->pfnSortContacts();
			}
			else {
				if (!(!mir_strcmp(cws->szSetting, "LogonTS") || !mir_strcmp(cws->szSetting, "TickTS") || !mir_strcmp(cws->szSetting, "InfoTS")))
					pcli->pfnSortContacts();

				return 0;
			}
		}
	}

	if (!mir_strcmp(cws->szModule, "CList")) {
		//name is null or (setting is myhandle)
		if (!mir_strcmp(cws->szSetting, "Rate"))
			pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);

		else if (pdnce->tszName == NULL || !mir_strcmp(cws->szSetting, "MyHandle"))
			InvalidateDNCEbyPointer(hContact, pdnce, cws->value.type);

		else if (!mir_strcmp(cws->szSetting, "Group"))
			InvalidateDNCEbyPointer(hContact, pdnce, cws->value.type);

		else if (!mir_strcmp(cws->szSetting, "Hidden")) {
			InvalidateDNCEbyPointer(hContact, pdnce, cws->value.type);
			if (cws->value.type == DBVT_DELETED || cws->value.bVal == 0) {
				char *szProto = GetContactProto(hContact);
				cli_ChangeContactIcon(hContact, pcli->pfnIconFromStatusMode(szProto,
					szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact), 1);  //by FYR
			}
			pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
		}
		else if (!mir_strcmp(cws->szSetting, "noOffline")) {
			InvalidateDNCEbyPointer(hContact, pdnce, cws->value.type);
			pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
		}
	}
	else if (!mir_strcmp(cws->szModule, "Protocol")) {
		if (!mir_strcmp(cws->szSetting, "p")) {
			InvalidateDNCEbyPointer(hContact, pdnce, cws->value.type);
			char *szProto = (cws->value.type == DBVT_DELETED) ? NULL : cws->value.pszVal;
			cli_ChangeContactIcon(hContact, pcli->pfnIconFromStatusMode(szProto,
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
