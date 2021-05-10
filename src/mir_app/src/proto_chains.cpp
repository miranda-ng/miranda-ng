/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
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

static int GetProtocolP(MCONTACT hContact, char *szBuf, int cbLen)
{
	if (g_pCurrDb == nullptr)
		return 1;

	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hContact);
	if (cc && cc->szProto != nullptr) {
		strncpy(szBuf, cc->szProto, cbLen);
		szBuf[cbLen - 1] = 0;
		return 0;
	}

	DBVARIANT dbv;
	dbv.type = DBVT_ASCIIZ;
	dbv.pszVal = szBuf;
	dbv.cchVal = cbLen;

	int res = g_pCurrDb->GetContactSettingStatic(hContact, "Protocol", "p", &dbv);
	if (res == 0) {
		if (cc == nullptr)
			cc = g_pCurrDb->getCache()->AddContactToCache(hContact);

		cc->szProto = g_pCurrDb->getCache()->GetCachedSetting(nullptr, szBuf, 0, mir_strlen(szBuf));
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(INT_PTR) Proto_ChainSend(int iOrder, CCSDATA *ccs)
{
	INT_PTR ret;

	if (iOrder == -1)
		return 1;

	for (int i = iOrder; i < g_arFilters.getCount(); i++) {
		if ((ret = CallProtoServiceInt(0, g_arFilters[i]->szName, ccs->szProtoService, i + 1, LPARAM(ccs))) != CALLSERVICE_NOTFOUND) {
			//chain was started, exit
			return ret;
		}
	}

	char szProto[40];
	if (GetProtocolP(ccs->hContact, szProto, sizeof(szProto)))
		return 1;

	PROTOACCOUNT *pa = Proto_GetAccount(szProto);
	if (pa == nullptr || pa->ppro == nullptr)
		return 1;

	if (pa->bOldProto)
		ret = CallProtoServiceInt(ccs->hContact, szProto, ccs->szProtoService, -1, (LPARAM)ccs);
	else
		ret = CallProtoServiceInt(ccs->hContact, szProto, ccs->szProtoService, ccs->wParam, ccs->lParam);
	if (ret == CALLSERVICE_NOTFOUND)
		ret = 1;

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR stubChainRecv(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	return Proto_ChainRecv(wParam, ccs);
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
			return CallServiceSync(MS_PROTO_HIDDENSTUB, iOrder, LPARAM(ccs));

		iOrder = g_arFilters.getCount();
	}
	else iOrder--;

	for (int i = iOrder - 1; i >= 0; i--)
		if ((ret = CallProtoServiceInt(0, g_arFilters[i]->szName, ccs->szProtoService, i + 1, (LPARAM)ccs)) != CALLSERVICE_NOTFOUND)
			//chain was started, exit
			return ret;

	//end of chain, call network protocol again
	char szProto[40];
	if (GetProtocolP(ccs->hContact, szProto, sizeof(szProto)))
		return 1;

	PROTOACCOUNT *pa = Proto_GetAccount(szProto);
	if (pa == nullptr || pa->ppro == nullptr)
		return 1;

	if (pa->bOldProto)
		ret = CallProtoServiceInt(ccs->hContact, szProto, ccs->szProtoService, -1, (LPARAM)ccs);
	else
		ret = CallProtoServiceInt(ccs->hContact, szProto, ccs->szProtoService, ccs->wParam, ccs->lParam);
	if (ret == CALLSERVICE_NOTFOUND)
		ret = 1;

	return ret;
}

MIR_APP_DLL(PROTOACCOUNT*) Proto_GetContactAccount(MCONTACT hContact)
{
	if (hContact == 0)
		return nullptr;

	char szProto[40];
	if (GetProtocolP(hContact, szProto, sizeof(szProto)))
		return nullptr;

	return Proto_GetAccount(szProto);
}

MIR_APP_DLL(char*) Proto_GetBaseAccountName(MCONTACT hContact)
{
	PROTOACCOUNT *pa = Proto_GetContactAccount(hContact);
	return pa->IsEnabled() ? pa->szModuleName : nullptr;
}

MIR_APP_DLL(int) Proto_IsProtoOnContact(MCONTACT hContact, const char *szProto)
{
	if (szProto == nullptr)
		return 0;

	char szContactProto[40];
	if (!GetProtocolP(hContact, szContactProto, sizeof(szContactProto)))
		if (!_stricmp(szProto, szContactProto))
			return -1;

	for (auto &it : g_arFilters)
		if (!mir_strcmp(szProto, it->szName))
			return g_arFilters.indexOf(&it) + 1;

	return 0;
}

MIR_APP_DLL(int) Proto_AddToContact(MCONTACT hContact, const char *szProto)
{
	if (auto *pa = Proto_GetAccount(szProto)) {
		db_set_s(hContact, "Protocol", "p", szProto);

		if (pa->ppro)
			pa->ppro->OnContactAdded(hContact);
		return 0;
	}

	if (auto *pd = Proto_IsProtocolLoaded(szProto)) {
		if (pd->type == PROTOTYPE_PROTOCOL || pd->type == PROTOTYPE_VIRTUAL || pd->type == PROTOTYPE_PROTOWITHACCS)
			db_set_s(hContact, "Protocol", "p", szProto);

		return 0;
	}
	
	return 1;
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
