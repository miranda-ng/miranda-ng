/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-15 Miranda NG project (http://miranda-ng.org),
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

static BOOL bModuleInitialized = FALSE;
static HANDLE hHooks[4];

static int CompareAccounts(const PROTOACCOUNT* p1, const PROTOACCOUNT* p2)
{
	return mir_strcmp(p1->szModuleName, p2->szModuleName);
}

LIST<PROTOACCOUNT> accounts(10, CompareAccounts);

/////////////////////////////////////////////////////////////////////////////////////////

static int EnumDbModules(const char *szModuleName, DWORD, LPARAM)
{
	ptrA szProtoName(db_get_sa(NULL, szModuleName, "AM_BaseProto"));
	if (szProtoName) {
		if (!Proto_GetAccount(szModuleName)) {
			PROTOACCOUNT *pa = (PROTOACCOUNT*)mir_calloc(sizeof(PROTOACCOUNT));
			pa->cbSize = sizeof(*pa);
			pa->szModuleName = mir_strdup(szModuleName);
			pa->szProtoName = szProtoName.detach();
			pa->tszAccountName = mir_a2t(szModuleName);
			pa->bIsVisible = TRUE;
			pa->bIsEnabled = FALSE;
			pa->iOrder = accounts.getCount();
			accounts.insert(pa);
		}
	}
	return 0;
}

void LoadDbAccounts(void)
{
	int ver = db_get_dw(NULL, "Protocols", "PrVer", -1);
	int count = db_get_dw(NULL, "Protocols", "ProtoCount", 0);

	for (int i = 0; i < count; i++) {
		char buf[10];
		_itoa(i, buf, 10);
		char *szModuleName = db_get_sa(NULL, "Protocols", buf);
		if (szModuleName == NULL)
			continue;

		PROTOACCOUNT *pa = (PROTOACCOUNT*)mir_calloc(sizeof(PROTOACCOUNT));
		if (pa == NULL) {
			mir_free(szModuleName);
			continue;
		}
		pa->cbSize = sizeof(*pa);
		pa->szModuleName = szModuleName;

		_itoa(OFFSET_VISIBLE + i, buf, 10);
		pa->bIsVisible = db_get_dw(NULL, "Protocols", buf, 1) != 0;

		_itoa(OFFSET_PROTOPOS + i, buf, 10);
		pa->iOrder = db_get_dw(NULL, "Protocols", buf, 1);

		if (ver >= 4) {
			_itoa(OFFSET_NAME + i, buf, 10);
			pa->tszAccountName = db_get_tsa(NULL, "Protocols", buf);

			_itoa(OFFSET_ENABLED + i, buf, 10);
			pa->bIsEnabled = db_get_dw(NULL, "Protocols", buf, 1) != 0;

			pa->szProtoName = db_get_sa(NULL, szModuleName, "AM_BaseProto");
		}
		else pa->bIsEnabled = true;

		if (!pa->szProtoName) {
			pa->szProtoName = mir_strdup(szModuleName);
			db_set_s(NULL, szModuleName, "AM_BaseProto", pa->szProtoName);
		}

		if (!pa->tszAccountName)
			pa->tszAccountName = mir_a2t(szModuleName);

		accounts.insert(pa);
	}

	if (CheckProtocolOrder())
		WriteDbAccounts();

	int anum = accounts.getCount();
	CallService(MS_DB_MODULES_ENUM, 0, (LPARAM)EnumDbModules);
	if (anum != accounts.getCount())
		WriteDbAccounts();
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	int  arrlen;
	char **pszSettingName;
}
enumDB_ProtoProcParam;

static int enumDB_ProtoProc(const char* szSetting, LPARAM lParam)
{
	if (szSetting) {
		enumDB_ProtoProcParam* p = (enumDB_ProtoProcParam*)lParam;

		p->arrlen++;
		p->pszSettingName = (char**)mir_realloc(p->pszSettingName, p->arrlen*sizeof(char*));
		p->pszSettingName[p->arrlen - 1] = mir_strdup(szSetting);
	}
	return 0;
}

void WriteDbAccounts()
{
	// enum all old settings to delete
	enumDB_ProtoProcParam param = { 0, NULL };

	DBCONTACTENUMSETTINGS dbces;
	dbces.pfnEnumProc = enumDB_ProtoProc;
	dbces.szModule = "Protocols";
	dbces.ofsSettings = 0;
	dbces.lParam = (LPARAM)&param;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);

	// delete all settings
	if (param.arrlen) {
		for (int i = 0; i < param.arrlen; i++) {
			db_unset(0, "Protocols", param.pszSettingName[i]);
			mir_free(param.pszSettingName[i]);
		}
		mir_free(param.pszSettingName);
	}

	// write new data
	for (int i = 0; i < accounts.getCount(); i++) {
		PROTOACCOUNT *pa = accounts[i];

		char buf[20];
		_itoa(i, buf, 10);
		db_set_s(NULL, "Protocols", buf, pa->szModuleName);

		_itoa(OFFSET_PROTOPOS + i, buf, 10);
		db_set_dw(NULL, "Protocols", buf, pa->iOrder);

		_itoa(OFFSET_VISIBLE + i, buf, 10);
		db_set_dw(NULL, "Protocols", buf, pa->bIsVisible);

		_itoa(OFFSET_ENABLED + i, buf, 10);
		db_set_dw(NULL, "Protocols", buf, pa->bIsEnabled);

		_itoa(OFFSET_NAME + i, buf, 10);
		db_set_ts(NULL, "Protocols", buf, pa->tszAccountName);
	}

	db_unset(0, "Protocols", "ProtoCount");
	db_set_dw(0, "Protocols", "ProtoCount", accounts.getCount());
	db_set_dw(0, "Protocols", "PrVer", 4);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnContactDeleted(WPARAM hContact, LPARAM lParam)
{
	if (hContact) {
		PROTOACCOUNT *pa = Proto_GetAccount(hContact);
		if (Proto_IsAccountEnabled(pa) && pa->ppro)
			pa->ppro->OnEvent(EV_PROTO_ONCONTACTDELETED, hContact, lParam);
	}
	return 0;
}

static int OnDbSettingsChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact) {
		PROTOACCOUNT *pa = Proto_GetAccount(hContact);
		if (Proto_IsAccountEnabled(pa) && pa->ppro)
			pa->ppro->OnEvent(EV_PROTO_DBSETTINGSCHANGED, hContact, lParam);
	}
	return 0;
}

static int InitializeStaticAccounts(WPARAM, LPARAM)
{
	int count = 0;

	for (int i = 0; i < accounts.getCount(); i++) {
		PROTOACCOUNT *pa = accounts[i];
		if (!pa->ppro || !Proto_IsAccountEnabled(pa))
			continue;

		pa->ppro->OnEvent(EV_PROTO_ONLOAD, 0, 0);

		if (!pa->bOldProto)
			count++;
	}

	BuildProtoMenus();

	if (count == 0 && !db_get_b(NULL, "FirstRun", "AccManager", 0)) {
		db_set_b(NULL, "FirstRun", "AccManager", 1);
		CallService(MS_PROTO_SHOWACCMGR, 0, 0);
	}
	// This is for pack creators with a profile with predefined accounts
	else if (db_get_b(NULL, "FirstRun", "ForceShowAccManager", 0)) {
		CallService(MS_PROTO_SHOWACCMGR, 0, 0);
		db_unset(NULL, "FirstRun", "ForceShowAccManager");
	}
	return 0;
}

static int UninitializeStaticAccounts(WPARAM, LPARAM)
{
	for (int i = 0; i < accounts.getCount(); i++) {
		PROTOACCOUNT *pa = accounts[i];
		if (pa->ppro && Proto_IsAccountEnabled(pa))
			if (pa->ppro->OnEvent(EV_PROTO_ONREADYTOEXIT, 0, 0) != TRUE)
				return 1;
	}

	for (int i = 0; i < accounts.getCount(); i++) {
		PROTOACCOUNT *pa = accounts[i];
		if (pa->ppro && Proto_IsAccountEnabled(pa))
			pa->ppro->OnEvent(EV_PROTO_ONEXIT, 0, 0);
	}

	return 0;
}

int LoadAccountsModule(void)
{
	bModuleInitialized = TRUE;

	for (int i = 0; i < accounts.getCount(); i++) {
		PROTOACCOUNT *pa = accounts[i];
		pa->bDynDisabled = !Proto_IsProtocolLoaded(pa->szProtoName);
		if (pa->ppro)
			continue;

		if (!Proto_IsAccountEnabled(pa))
			continue;

		if (!ActivateAccount(pa))
			pa->bDynDisabled = TRUE;
	}

	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, InitializeStaticAccounts);
	hHooks[1] = HookEvent(ME_SYSTEM_PRESHUTDOWN, UninitializeStaticAccounts);
	hHooks[2] = HookEvent(ME_DB_CONTACT_DELETED, OnContactDeleted);
	hHooks[3] = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnDbSettingsChanged);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static HANDLE CreateProtoServiceEx(const char* szModule, const char* szService, MIRANDASERVICEOBJ pFunc, void* param)
{
	char tmp[100];
	mir_snprintf(tmp, "%s%s", szModule, szService);
	return CreateServiceFunctionObj(tmp, pFunc, param);
}

BOOL ActivateAccount(PROTOACCOUNT *pa)
{
	PROTOCOLDESCRIPTOR* ppd = Proto_IsProtocolLoaded(pa->szProtoName);
	if (ppd == NULL)
		return FALSE;

	if (ppd->fnInit == NULL)
		return FALSE;

	PROTO_INTERFACE *ppi = ppd->fnInit(pa->szModuleName, pa->tszAccountName);
	if (ppi == NULL)
		return FALSE;

	pa->ppro = ppi;
	ppi->m_iDesiredStatus = ppi->m_iStatus = ID_STATUS_OFFLINE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct DeactivationThreadParam
{
	PROTO_INTERFACE *ppro;
	pfnUninitProto fnUninit;
	bool bIsDynamic, bErase;
};

pfnUninitProto GetProtocolDestructor(char *szProto);

static int DeactivationThread(DeactivationThreadParam* param)
{
	PROTO_INTERFACE *p = (PROTO_INTERFACE*)param->ppro;
	p->SetStatus(ID_STATUS_OFFLINE);

	char *szModuleName = NEWSTR_ALLOCA(p->m_szModuleName);

	if (param->bIsDynamic) {
		while (p->OnEvent(EV_PROTO_ONREADYTOEXIT, 0, 0) != TRUE)
			SleepEx(100, TRUE);

		p->OnEvent(EV_PROTO_ONEXIT, 0, 0);
	}

	KillObjectThreads(p); // waits for them before terminating
	KillObjectEventHooks(p); // untie an object from the outside world

	if (param->bErase)
		p->OnEvent(EV_PROTO_ONERASE, 0, 0);

	if (param->fnUninit)
		param->fnUninit(p);

	KillObjectServices(p);

	if (param->bErase)
		EraseAccount(szModuleName);

	delete param;
	return 0;
}

void DeactivateAccount(PROTOACCOUNT *pa, bool bIsDynamic, bool bErase)
{
	if (pa->ppro == NULL) {
		if (bErase)
			EraseAccount(pa->szModuleName);
		return;
	}

	if (pa->hwndAccMgrUI) {
		DestroyWindow(pa->hwndAccMgrUI);
		pa->hwndAccMgrUI = NULL;
		pa->bAccMgrUIChanged = FALSE;
	}

	DeactivationThreadParam *param = new DeactivationThreadParam;
	param->ppro = pa->ppro;
	param->fnUninit = GetProtocolDestructor(pa->szProtoName);
	param->bIsDynamic = bIsDynamic;
	param->bErase = bErase;
	pa->ppro = NULL;
	if (bIsDynamic)
		mir_forkthread((pThreadFunc)DeactivationThread, param);
	else
		DeactivationThread(param);
}

/////////////////////////////////////////////////////////////////////////////////////////

void EraseAccount(const char* pszModuleName)
{
	// remove protocol contacts first
	for (MCONTACT hContact = db_find_first(pszModuleName); hContact != NULL;) {
		MCONTACT hNext = db_find_next(hContact, pszModuleName);
		CallService(MS_DB_CONTACT_DELETE, hContact, 0);
		hContact = hNext;
	}

	// remove all protocol settings
	CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)pszModuleName);
}

/////////////////////////////////////////////////////////////////////////////////////////

void UnloadAccount(PROTOACCOUNT *pa, bool bIsDynamic, bool bErase)
{
	DeactivateAccount(pa, bIsDynamic, bErase);

	mir_free(pa->tszAccountName);
	mir_free(pa->szProtoName);
	// szModuleName should be freed only on a program's exit.
	// otherwise many plugins dependand on static protocol names will crash!
	// do NOT fix this 'leak', please
	if (!bIsDynamic) {
		mir_free(pa->szModuleName);
		mir_free(pa);
	}
}

void UnloadAccountsModule()
{
	if (!bModuleInitialized) return;

	for (int i = accounts.getCount() - 1; i >= 0; i--) {
		PROTOACCOUNT *pa = accounts[i];
		UnloadAccount(pa, false, false);
		accounts.remove(i);
	}
	accounts.destroy();

	for (int i = 0; i < _countof(hHooks); i++)
		UnhookEvent(hHooks[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////

void BuildProtoMenus()
{
	for (int i = 0; i < accounts.getCount(); i++) {
		PROTOACCOUNT *pa = accounts[i];
		if (cli.pfnGetProtocolVisibility(pa->szModuleName) == 0)
			continue;

		if (pa->ppro)
			pa->ppro->OnEvent(EV_PROTO_ONMENU, 0, 0);
	}
}

void RebuildProtoMenus()
{
	RebuildMenuOrder();
	BuildProtoMenus();
}
