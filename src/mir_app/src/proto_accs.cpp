/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "clc.h"

bool CheckProtocolOrder(void);
void BuildProtoMenus();

HICON Proto_GetIcon(PROTO_INTERFACE *ppro, int iconIndex);

static bool bModuleInitialized = false;
static HANDLE hHooks[4];

static int CompareAccounts(const PROTOACCOUNT* p1, const PROTOACCOUNT* p2)
{
	return mir_strcmp(p1->szModuleName, p2->szModuleName);
}

LIST<PROTOACCOUNT> g_arAccounts(10, CompareAccounts);

/////////////////////////////////////////////////////////////////////////////////////////

static int EnumDbModules(const char *szModuleName, void*)
{
	ptrA szProtoName(db_get_sa(0, szModuleName, "AM_BaseProto"));
	if (szProtoName) {
		if (!Proto_GetAccount(szModuleName)) {
			PROTOACCOUNT *pa = new PROTOACCOUNT(szModuleName);
			pa->szProtoName = szProtoName.detach();
			pa->tszAccountName = mir_a2u(szModuleName);
			pa->bIsVisible = true;
			pa->bIsEnabled = false;
			pa->iOrder = g_arAccounts.getCount();
			g_arAccounts.insert(pa);
		}
	}
	return 0;
}

void LoadDbAccounts(void)
{
	int ver = db_get_dw(0, "Protocols", "PrVer", -1);
	int count = db_get_dw(0, "Protocols", "ProtoCount", 0);

	for (int i = 0; i < count; i++) {
		char buf[10];
		_itoa(i, buf, 10);
		ptrA szModuleName(db_get_sa(0, "Protocols", buf));
		if (szModuleName == nullptr)
			continue;

		PROTOACCOUNT *pa = Proto_GetAccount(szModuleName);
		if (pa == nullptr) {
			pa = new PROTOACCOUNT(szModuleName);
			g_arAccounts.insert(pa);
		}

		_itoa(OFFSET_VISIBLE + i, buf, 10);
		pa->bIsVisible = db_get_dw(0, "Protocols", buf, 1) != 0;

		_itoa(OFFSET_PROTOPOS + i, buf, 10);
		pa->iOrder = db_get_dw(0, "Protocols", buf, 1);

		if (ver >= 4) {
			_itoa(OFFSET_NAME + i, buf, 10);
			pa->tszAccountName = db_get_wsa(0, "Protocols", buf);

			_itoa(OFFSET_ENABLED + i, buf, 10);
			pa->bIsEnabled = db_get_dw(0, "Protocols", buf, 1) != 0;
			if (!pa->bIsEnabled && !mir_strcmp(pa->szModuleName, META_PROTO)) {
				pa->bIsEnabled = true;
				db_set_dw(0, "Protocols", buf, 1);
			}
			pa->szProtoName = db_get_sa(0, szModuleName, "AM_BaseProto");
		}
		else pa->bIsEnabled = true;

		if (!pa->szProtoName) {
			pa->szProtoName = mir_strdup(szModuleName);
			db_set_s(0, szModuleName, "AM_BaseProto", pa->szProtoName);
		}

		if (!pa->tszAccountName)
			pa->tszAccountName = mir_a2u(szModuleName);
	}

	if (CheckProtocolOrder())
		WriteDbAccounts();

	int anum = g_arAccounts.getCount();
	db_enum_modules(EnumDbModules);
	if (anum != g_arAccounts.getCount())
		WriteDbAccounts();
}

/////////////////////////////////////////////////////////////////////////////////////////

void WriteDbAccounts()
{
	// enum all old settings to delete
	db_delete_module(0, "Protocols");

	// write new data
	for (int i = 0; i < g_arAccounts.getCount(); i++) {
		PROTOACCOUNT *pa = g_arAccounts[i];

		char buf[20];
		_itoa(i, buf, 10);
		db_set_s(0, "Protocols", buf, pa->szModuleName);

		_itoa(OFFSET_PROTOPOS + i, buf, 10);
		db_set_dw(0, "Protocols", buf, pa->iOrder);

		_itoa(OFFSET_VISIBLE + i, buf, 10);
		db_set_dw(0, "Protocols", buf, pa->bIsVisible);

		_itoa(OFFSET_ENABLED + i, buf, 10);
		db_set_dw(0, "Protocols", buf, pa->bIsEnabled);

		_itoa(OFFSET_NAME + i, buf, 10);
		db_set_ws(0, "Protocols", buf, pa->tszAccountName);
	}

	db_set_dw(0, "Protocols", "ProtoCount", g_arAccounts.getCount());
	db_set_dw(0, "Protocols", "PrVer", 4);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnContactDeleted(WPARAM hContact, LPARAM)
{
	if (auto *ppro = Proto_GetInstance(hContact))
		ppro->OnContactDeleted(hContact);
	return 0;
}

static int OnEventEdited(WPARAM hContact, LPARAM hDbEvent)
{
	if (auto *ppro = Proto_GetInstance(hContact))
		ppro->OnEventEdited(hContact, hDbEvent);
	return 0;
}

void InitStaticAccounts()
{
	int count = 0;

	for (auto &pa : g_arAccounts) {
		if (!pa->ppro || !pa->IsEnabled())
			continue;

		pa->ppro->OnModulesLoaded();

		if (!pa->bOldProto)
			count++;

		if (pa->IsVisible())
			pa->ppro->OnBuildProtoMenu();
	}

	if (count == 0 && !db_get_b(0, "FirstRun", "AccManager", 0)) {
		db_set_b(0, "FirstRun", "AccManager", 1);
		CallService(MS_PROTO_SHOWACCMGR, 0, 0);
	}
	// This is for pack creators with a profile with predefined g_arAccounts
	else if (db_get_b(0, "FirstRun", "ForceShowAccManager", 0)) {
		CallService(MS_PROTO_SHOWACCMGR, 0, 0);
		db_unset(0, "FirstRun", "ForceShowAccManager");
	}
}

static int UninitializeStaticAccounts(WPARAM, LPARAM)
{
	// request permission to exit first
	for (auto &pa : g_arAccounts)
		if (pa->ppro && pa->IsEnabled())
			if (!pa->ppro->IsReadyToExit())
				return 1;

	// okay, all protocols are ready, exiting
	for (auto &pa : g_arAccounts)
		if (pa->ppro && pa->IsEnabled())
			pa->ppro->OnShutdown();

	return 0;
}

int LoadAccountsModule(void)
{
	bModuleInitialized = true;

	for (auto &pa : g_arAccounts) {
		pa->bDynDisabled = !Proto_IsProtocolLoaded(pa->szProtoName);
		if (pa->ppro)
			continue;

		if (!pa->IsEnabled())
			continue;

		if (!ActivateAccount(pa, false))
			pa->bDynDisabled = true;
	}

	hHooks[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, UninitializeStaticAccounts);
	hHooks[2] = HookEvent(ME_DB_CONTACT_DELETED, OnContactDeleted);
	hHooks[3] = HookEvent(ME_DB_EVENT_EDITED, OnEventEdited);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static HANDLE CreateProtoServiceEx(const char* szModule, const char* szService, MIRANDASERVICEOBJ pFunc, void* param)
{
	char tmp[100];
	mir_snprintf(tmp, "%s%s", szModule, szService);
	return CreateServiceFunctionObj(tmp, pFunc, param);
}

bool ActivateAccount(PROTOACCOUNT *pa, bool bIsDynamic)
{
	MBaseProto *ppd = Proto_GetProto(pa->szProtoName);
	if (ppd == nullptr)
		return false;

	if (ppd->fnInit == nullptr)
		return false;

	PROTO_INTERFACE *ppi = pa->ppro;
	if (ppi == nullptr) {
		ppi = ppd->fnInit(pa->szModuleName, pa->tszAccountName);
		if (ppi == nullptr)
			return false;

		pa->ppro = ppi;

		if (bIsDynamic) {
			if (g_bModulesLoadedFired)
				pa->ppro->OnModulesLoaded();
			if (!db_get_b(0, "CList", "MoveProtoMenus", true))
				pa->ppro->OnBuildProtoMenu();
			pa->bDynDisabled = false;
		}
	}

	if (ppi->m_hProtoIcon == nullptr)
		ppi->m_hProtoIcon = IcoLib_IsManaged(Skin_LoadProtoIcon(pa->szModuleName, ID_STATUS_ONLINE));
	ppi->m_iDesiredStatus = ppi->m_iStatus = ID_STATUS_OFFLINE;
	return true;
}

MIR_APP_DLL(int) Proto_GetAverageStatus(int *pAccountNumber)
{
	int netProtoCount = 0, averageMode = 0;

	for (auto &pa : g_arAccounts) {
		if (!pa->IsVisible() || pa->IsLocked())
			continue;

		netProtoCount++;
		if (averageMode == 0)
			averageMode = pa->iRealStatus;
		else if (averageMode > 0 && averageMode != pa->iRealStatus) {
			averageMode = -1;
			if (pAccountNumber == nullptr)
				break;
		}
	}

	if (pAccountNumber)
		*pAccountNumber = netProtoCount;
	return averageMode;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct DeactivationThreadParam
{
	PROTO_INTERFACE *ppro;
	pfnUninitProto fnUninit;
	int flags;
};

pfnUninitProto GetProtocolDestructor(char *szProto);

static void __cdecl DeactivationThread(DeactivationThreadParam *param)
{
	PROTO_INTERFACE *p = (PROTO_INTERFACE*)param->ppro;
	p->SetStatus(ID_STATUS_OFFLINE);

	char *szModuleName = NEWSTR_ALLOCA(p->m_szModuleName);

	if (param->flags & DAF_DYNAMIC) {
		while (!p->IsReadyToExit())
			SleepEx(100, TRUE);

		p->OnShutdown();
	}

	KillObjectThreads(p); // waits for them before terminating
	KillObjectEventHooks(p); // untie an object from the outside world

	if (param->flags & DAF_ERASE)
		p->OnErase();

	if (param->fnUninit)
		param->fnUninit(p);

	KillObjectServices(p);

	if (param->flags & DAF_ERASE)
		EraseAccount(szModuleName);

	delete param;
}

void DeactivateAccount(PROTOACCOUNT *pa, int flags)
{
	if (pa->hwndAccMgrUI) {
		DestroyWindow(pa->hwndAccMgrUI);
		pa->hwndAccMgrUI = nullptr;
		pa->bAccMgrUIChanged = FALSE;
	}

	if (flags & DAF_DYNAMIC)
		NotifyEventHooks(hAccListChanged, PRAC_REMOVED, (LPARAM)pa);
	else
		pa->iIconBase = -1;

	if (pa->ppro == nullptr) {
		if (flags & DAF_ERASE)
			EraseAccount(pa->szModuleName);
		return;
	}

	DeactivationThreadParam *param = new DeactivationThreadParam;
	param->ppro = pa->ppro;
	param->fnUninit = GetProtocolDestructor(pa->szProtoName);
	param->flags = flags;
	pa->ppro = nullptr;
	if (flags & DAF_FORK)
		mir_forkThread<DeactivationThreadParam>(DeactivationThread, param);
	else
		DeactivationThread(param);
}

/////////////////////////////////////////////////////////////////////////////////////////

void KillModuleAccounts(HINSTANCE hInst)
{
	for (auto &pd : g_arProtos.rev_iter()) {
		if (pd->hInst != hInst)
			continue;

		for (auto &pa : g_arAccounts.rev_iter()) {
			if (!mir_strcmp(pa->szProtoName, pd->szName)) {
				pa->bDynDisabled = true;
				DeactivateAccount(pa, DAF_DYNAMIC);
			}
		}

		g_arProtos.removeItem(&pd);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void EraseAccount(const char *pszModuleName)
{
	// remove protocol contacts first
	for (MCONTACT hContact = db_find_first(pszModuleName); hContact != 0;) {
		MCONTACT hNext = db_find_next(hContact, pszModuleName);
		db_delete_contact(hContact);
		hContact = hNext;
	}

	// remove all protocol settings
	db_delete_module(0, pszModuleName);
}

/////////////////////////////////////////////////////////////////////////////////////////

void UnloadAccount(PROTOACCOUNT *pa, int flags)
{
	DeactivateAccount(pa, flags);

	// szModuleName should be freed only on a program's exit.
	// otherwise many plugins dependand on static protocol names will crash!
	// do NOT fix this 'leak', please
	if (!(flags & DAF_DYNAMIC))
		delete pa;
	else {
		replaceStrW(pa->tszAccountName, 0);
		replaceStr(pa->szProtoName, 0);
		replaceStr(pa->szUniqueId, 0);
	}
}

void UnloadAccountsModule()
{
	if (!bModuleInitialized)
		return;

	auto T = g_arAccounts.rev_iter();
	for (auto &it : T) {
		UnloadAccount(it, 0);
		g_arAccounts.removeItem(&it);
	}
	g_arAccounts.destroy();

	for (auto &it : hHooks)
		UnhookEvent(it);
}
