/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

extern LIST<PROTOCOLDESCRIPTOR> filters;

static int GetProtocolP(MCONTACT hContact, char *szBuf, int cbLen)
{
	if (currDb == NULL)
		return 1;

	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hContact);
	if (cc && cc->szProto != NULL) {
		strncpy(szBuf, cc->szProto, cbLen);
		szBuf[cbLen - 1] = 0;
		return 0;
	}

	DBVARIANT dbv;
	dbv.type = DBVT_ASCIIZ;
	dbv.pszVal = szBuf;
	dbv.cchVal = cbLen;

	int res = currDb->GetContactSettingStatic(hContact, "Protocol", "p", &dbv);
	if (res == 0) {
		if (cc == NULL)
			cc = currDb->m_cache->AddContactToCache(hContact);

		cc->szProto = currDb->m_cache->GetCachedSetting(NULL, szBuf, 0, (int)mir_strlen(szBuf));
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(INT_PTR) Proto_ChainSend(int iOrder, CCSDATA *ccs)
{
	INT_PTR ret;

	if (iOrder == -1)
		return 1;

	for (int i = iOrder; i < filters.getCount(); i++) {
		if ((ret = CallProtoServiceInt(NULL, filters[i]->szName, ccs->szProtoService, i + 1, iOrder)) != CALLSERVICE_NOTFOUND) {
			//chain was started, exit
			return ret;
		}
	}

	char szProto[40];
	if (GetProtocolP((MCONTACT)ccs->hContact, szProto, sizeof(szProto)))
		return 1;

	PROTOACCOUNT *pa = Proto_GetAccount(szProto);
	if (pa == NULL || pa->ppro == NULL)
		return 1;

	if (pa->bOldProto)
		ret = CallProtoServiceInt(ccs->hContact, szProto, ccs->szProtoService, (WPARAM)(-1), (LPARAM)ccs);
	else
		ret = CallProtoServiceInt(ccs->hContact, szProto, ccs->szProtoService, ccs->wParam, ccs->lParam);
	if (ret == CALLSERVICE_NOTFOUND)
		ret = 1;

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(INT_PTR) CallContactService(MCONTACT hContact, const char *szProtoService, WPARAM wParam, LPARAM lParam)
{
	CCSDATA ccs = { hContact, szProtoService, wParam, lParam };
	return Proto_ChainSend(0, &ccs);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void __stdcall stubChainRecv(void *param)
{
	CCSDATA *ccs = (CCSDATA*)param;
	Proto_ChainRecv(0, ccs);
}

MIR_APP_DLL(INT_PTR) Proto_ChainRecv(int iOrder, CCSDATA *ccs)
{
	INT_PTR ret;

	// shouldn't happen - sanity check
	if (iOrder == -1)
		return 1; 
	
	// begin processing by finding end of chain
	if (iOrder == 0) {
		if (GetCurrentThreadId() != hMainThreadId) // restart this function in the main thread
			return CallFunctionAsync(stubChainRecv, ccs);

		iOrder = filters.getCount();
	}
	else iOrder--;

	for (int i = iOrder - 1; i >= 0; i--)
		if ((ret = CallProtoServiceInt(NULL, filters[i]->szName, ccs->szProtoService, i + 1, (LPARAM)ccs)) != CALLSERVICE_NOTFOUND)
			//chain was started, exit
			return ret;

	//end of chain, call network protocol again
	char szProto[40];
	if (GetProtocolP((MCONTACT)ccs->hContact, szProto, sizeof(szProto)))
		return 1;

	PROTOACCOUNT *pa = Proto_GetAccount(szProto);
	if (pa == NULL || pa->ppro == NULL)
		return 1;

	if (pa->bOldProto)
		ret = CallProtoServiceInt(ccs->hContact, szProto, ccs->szProtoService, (WPARAM)(-1), (LPARAM)ccs);
	else
		ret = CallProtoServiceInt(ccs->hContact, szProto, ccs->szProtoService, ccs->wParam, ccs->lParam);
	if (ret == CALLSERVICE_NOTFOUND)
		ret = 1;

	return ret;
}

PROTOACCOUNT* __fastcall Proto_GetAccount(MCONTACT hContact)
{
	if (hContact == NULL)
		return NULL;

	char szProto[40];
	if (GetProtocolP((MCONTACT)hContact, szProto, sizeof(szProto)))
		return NULL;

	return Proto_GetAccount(szProto);
}

MIR_APP_DLL(char*) GetContactProto(MCONTACT hContact)
{
	PROTOACCOUNT *pa = Proto_GetAccount(hContact);
	return Proto_IsAccountEnabled(pa) ? pa->szModuleName : NULL;
}

MIR_APP_DLL(char*) Proto_GetBaseAccountName(MCONTACT hContact)
{
	PROTOACCOUNT *pa = Proto_GetAccount(hContact);
	return pa ? pa->szModuleName : NULL;
}

MIR_APP_DLL(int) Proto_IsProtoOnContact(MCONTACT hContact, const char *szProto)
{
	if (szProto == NULL)
		return 0;

	char szContactProto[40];
	if (!GetProtocolP(hContact, szContactProto, sizeof(szContactProto)))
		if (!_stricmp(szProto, szContactProto))
			return -1;

	for (int i = 0; i < filters.getCount(); i++)
		if (!mir_strcmp(szProto, filters[i]->szName))
			return i + 1;

	return 0;
}

MIR_APP_DLL(int) Proto_AddToContact(MCONTACT hContact, const char *szProto)
{
	PROTOCOLDESCRIPTOR *pd = Proto_IsProtocolLoaded(szProto);
	if (pd == NULL) {
		PROTOACCOUNT *pa = Proto_GetAccount(szProto);
		if (pa) {
			db_set_s(hContact, "Protocol", "p", szProto);
			return 0;
		}
		return 1;
	}

	if (pd->type == PROTOTYPE_PROTOCOL || pd->type == PROTOTYPE_VIRTUAL)
		db_set_s(hContact, "Protocol", "p", szProto);

	return 0;
}

MIR_APP_DLL(int) Proto_RemoveFromContact(MCONTACT hContact, const char *szProto)
{
	switch (Proto_IsProtoOnContact(hContact, szProto)) {
	case 0:
		return 1;
	case -1:
		db_unset(hContact, "Protocol", "p");
	}

	return 0;
}
