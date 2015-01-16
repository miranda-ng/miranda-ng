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

#include "..\..\core\commonheaders.h"

#include "../clist/clc.h"

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

static int EnumDbModules(const char *szModuleName, DWORD ofsModuleName, LPARAM lParam)
{
	DBVARIANT dbv;
	if (!db_get_s(NULL, szModuleName, "AM_BaseProto", &dbv)) {
		if (!Proto_GetAccount(szModuleName)) {
			PROTOACCOUNT *pa = (PROTOACCOUNT*)mir_calloc(sizeof(PROTOACCOUNT));
			pa->cbSize = sizeof(*pa);
			pa->szModuleName = mir_strdup(szModuleName);
			pa->szProtoName = mir_strdup(dbv.pszVal);
			pa->tszAccountName = mir_a2t(szModuleName);
			pa->bIsVisible = TRUE;
			pa->bIsEnabled = FALSE;
			pa->iOrder = accounts.getCount();
			accounts.insert(pa);
		}
		db_free(&dbv);
	}
	return 0;
}

void LoadDbAccounts(void)
{
	DBVARIANT dbv;
	int ver = db_get_dw(NULL, "Protocols", "PrVer", -1);
	int count = db_get_dw(NULL, "Protocols", "ProtoCount", 0);

	for (int i = 0; i < count; i++) {
		char buf[10];
		_itoa(i, buf, 10);
		if (db_get_s(NULL, "Protocols", buf, &dbv))
			continue;

		PROTOACCOUNT *pa = (PROTOACCOUNT*)mir_calloc(sizeof(PROTOACCOUNT));
		if (pa == NULL) {
			db_free(&dbv);
			continue;
		}
		pa->cbSize = sizeof(*pa);
		pa->szModuleName = mir_strdup(dbv.pszVal);
		db_free(&dbv);

		_itoa(OFFSET_VISIBLE + i, buf, 10);
		pa->bIsVisible = db_get_dw(NULL, "Protocols", buf, 1) != 0;

		_itoa(OFFSET_PROTOPOS + i, buf, 10);
		pa->iOrder = db_get_dw(NULL, "Protocols", buf, 1);

		if (ver >= 4) {
			db_free(&dbv);
			_itoa(OFFSET_NAME + i, buf, 10);
			if (!db_get_ts(NULL, "Protocols", buf, &dbv)) {
				pa->tszAccountName = mir_tstrdup(dbv.ptszVal);
				db_free(&dbv);
			}

			_itoa(OFFSET_ENABLED + i, buf, 10);
			pa->bIsEnabled = db_get_dw(NULL, "Protocols", buf, 1) != 0;

			if (!db_get_s(NULL, pa->szModuleName, "AM_BaseProto", &dbv)) {
				pa->szProtoName = mir_strdup(dbv.pszVal);
				db_free(&dbv);
			}
		}
		else pa->bIsEnabled = true;

		if (!pa->szProtoName) {
			pa->szProtoName = mir_strdup(pa->szModuleName);
			db_set_s(NULL, pa->szModuleName, "AM_BaseProto", pa->szProtoName);
		}

		if (!pa->tszAccountName)
			pa->tszAccountName = mir_a2t(pa->szModuleName);

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

static INT_PTR stub1(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)ppi->AddToList(wParam, (PROTOSEARCHRESULT*)lParam);
}

static INT_PTR stub2(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)ppi->AddToListByEvent(HIWORD(wParam), LOWORD(wParam), (MEVENT)lParam);
}

static INT_PTR stub3(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM)
{
	return (INT_PTR)ppi->Authorize((MEVENT)wParam);
}

static INT_PTR stub4(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)ppi->AuthDeny((MEVENT)wParam, StrConvT((const char*)lParam));
}

static INT_PTR stub11(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	PROTOFILERESUME* pfr = (PROTOFILERESUME*)lParam;
	return (INT_PTR)ppi->FileResume((HANDLE)wParam, &pfr->action, (const PROTOCHAR**)&pfr->szFilename);
}

static INT_PTR stub12(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)ppi->GetCaps(wParam, lParam);
}

static INT_PTR stub13(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM)
{
	return (INT_PTR)Proto_GetIcon(ppi, wParam);
}

static INT_PTR stub15(PROTO_INTERFACE* ppi, WPARAM, LPARAM lParam)
{
	return (INT_PTR)ppi->SearchBasic(StrConvT((char*)lParam));
}

static INT_PTR stub16(PROTO_INTERFACE* ppi, WPARAM, LPARAM lParam)
{
	return (INT_PTR)ppi->SearchByEmail(StrConvT((char*)lParam));
}

static INT_PTR stub17(PROTO_INTERFACE* ppi, WPARAM, LPARAM lParam)
{
	PROTOSEARCHBYNAME* psbn = (PROTOSEARCHBYNAME*)lParam;
	return (INT_PTR)ppi->SearchByName(StrConvT((char*)psbn->pszNick),
		StrConvT((char*)psbn->pszFirstName), StrConvT((char*)psbn->pszLastName));
}

static INT_PTR stub18(PROTO_INTERFACE* ppi, WPARAM, LPARAM lParam)
{
	return (INT_PTR)ppi->SearchAdvanced((HWND)lParam);
}

static INT_PTR stub19(PROTO_INTERFACE* ppi, WPARAM, LPARAM lParam)
{
	return (INT_PTR)ppi->CreateExtendedSearchUI((HWND)lParam);
}

static INT_PTR stub22(PROTO_INTERFACE* ppi, WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	ppi->RecvMsg(ccs->hContact, (PROTORECVEVENT*)ccs->lParam);
	return 0;
}

static INT_PTR stub29(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM)
{
	return (INT_PTR)ppi->SetStatus(wParam);
}

static INT_PTR stub33(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)ppi->SetAwayMsg(wParam, StrConvT((const char*)lParam));
}

static INT_PTR stub41(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char*)lParam, ppi->m_szModuleName, wParam);
	return 0;
}

static INT_PTR stub42(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	return ppi->m_iStatus;
}

static INT_PTR stub43(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION* p = (PROTO_AVATAR_INFORMATION*)lParam;

	PROTO_AVATAR_INFORMATIONW tmp = { 0 };
	tmp.cbSize = sizeof(tmp);
	tmp.hContact = p->hContact;
	int result = CallProtoServiceInt(NULL, ppi->m_szModuleName, PS_GETAVATARINFOW, wParam, (LPARAM)&tmp);

	p->format = tmp.format;

	wchar_t filename[MAX_PATH];
	wcscpy(filename, tmp.filename);
	GetShortPathNameW(tmp.filename, filename, SIZEOF(filename));

	WideCharToMultiByte(CP_ACP, 0, filename, -1, p->filename, MAX_PATH, 0, 0);
	return result;
}

static INT_PTR stub44(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	wchar_t* buf = (wchar_t*)_alloca(sizeof(wchar_t) * (lParam + 1));
	int result = CallProtoServiceInt(NULL, ppi->m_szModuleName, PS_GETMYAVATARW, WPARAM(buf), lParam);
	if (result == 0) {
		wchar_t* filename = (wchar_t*)_alloca(sizeof(wchar_t) * (lParam + 1));
		wcscpy(filename, buf);
		GetShortPathNameW(buf, filename, lParam + 1);

		WideCharToMultiByte(CP_ACP, 0, filename, -1, (char*)wParam, lParam, 0, 0);
	}

	return result;
}

static INT_PTR stub45(PROTO_INTERFACE* ppi, WPARAM wParam, LPARAM lParam)
{
	return CallProtoServiceInt(NULL, ppi->m_szModuleName, PS_SETMYAVATARW, wParam, (LPARAM)(LPCTSTR)StrConvT((char*)lParam));
}

static HANDLE CreateProtoServiceEx(const char* szModule, const char* szService, MIRANDASERVICEOBJ pFunc, void* param)
{
	char tmp[100];
	mir_snprintf(tmp, SIZEOF(tmp), "%s%s", szModule, szService);
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
	CreateProtoServiceEx(pa->szModuleName, PS_ADDTOLIST, (MIRANDASERVICEOBJ)stub1, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_ADDTOLISTBYEVENT, (MIRANDASERVICEOBJ)stub2, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_AUTHALLOW, (MIRANDASERVICEOBJ)stub3, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_AUTHDENY, (MIRANDASERVICEOBJ)stub4, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_FILERESUME, (MIRANDASERVICEOBJ)stub11, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_GETCAPS, (MIRANDASERVICEOBJ)stub12, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_LOADICON, (MIRANDASERVICEOBJ)stub13, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_BASICSEARCH, (MIRANDASERVICEOBJ)stub15, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_SEARCHBYEMAIL, (MIRANDASERVICEOBJ)stub16, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_SEARCHBYNAME, (MIRANDASERVICEOBJ)stub17, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_SEARCHBYADVANCED, (MIRANDASERVICEOBJ)stub18, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_CREATEADVSEARCHUI, (MIRANDASERVICEOBJ)stub19, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PSR_MESSAGE, (MIRANDASERVICEOBJ)stub22, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_SETSTATUS, (MIRANDASERVICEOBJ)stub29, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_SETAWAYMSG, (MIRANDASERVICEOBJ)stub33, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_GETNAME, (MIRANDASERVICEOBJ)stub41, pa->ppro);
	CreateProtoServiceEx(pa->szModuleName, PS_GETSTATUS, (MIRANDASERVICEOBJ)stub42, pa->ppro);

	if (!ProtoServiceExists(pa->szModuleName, PS_GETAVATARINFO))
		if (ProtoServiceExists(pa->szModuleName, PS_GETAVATARINFOW))
			CreateProtoServiceEx(pa->szModuleName, PS_GETAVATARINFO, (MIRANDASERVICEOBJ)stub43, pa->ppro);

	if (!ProtoServiceExists(pa->szModuleName, PS_GETMYAVATAR))
		if (ProtoServiceExists(pa->szModuleName, PS_GETMYAVATARW))
			CreateProtoServiceEx(pa->szModuleName, PS_GETMYAVATAR, (MIRANDASERVICEOBJ)stub44, pa->ppro);

	if (!ProtoServiceExists(pa->szModuleName, PS_SETMYAVATAR))
		if (ProtoServiceExists(pa->szModuleName, PS_SETMYAVATARW))
			CreateProtoServiceEx(pa->szModuleName, PS_SETMYAVATAR, (MIRANDASERVICEOBJ)stub45, pa->ppro);

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

	for (int i = 0; i < SIZEOF(hHooks); i++)
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

void RebuildProtoMenus(int iNewValue)
{
	db_set_b(NULL, "CList", "MoveProtoMenus", iNewValue);

	RebuildMenuOrder();
	BuildProtoMenus();
}
