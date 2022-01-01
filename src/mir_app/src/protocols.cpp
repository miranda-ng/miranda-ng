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

int LoadProtoOptions(void);

HANDLE hAckEvent;
HANDLE hAccListChanged;
static HANDLE hTypeEvent;
static BOOL bModuleInitialized = FALSE;

struct TServiceListItem
{
	const char *name;
	int id;
};

static int __cdecl CompareServiceItems(const void *p1, const void *p2)
{	return strcmp(((TServiceListItem*)p1)->name, ((TServiceListItem*)p2)->name);
}

static TServiceListItem serviceItems[] = 
{
	{ PS_ADDTOLIST,          1 },
	{ PS_ADDTOLISTBYEVENT,   2 },
	{ PS_AUTHALLOW,          3 },
	{ PS_AUTHDENY,           4 },
	{ PSR_AUTH,              5 },
	{ PSS_AUTHREQUEST,       6 },
	{ PSS_FILEALLOW,         8 },
	{ PSS_FILECANCEL,        9 },
	{ PSS_FILEDENY,         10 },
	{ PS_FILERESUME,        11 },
	{ PS_GETCAPS,           12 },
	{ PS_LOADICON,          13 },
	{ PSS_GETINFO,          14 },
	{ PS_BASICSEARCH,       15 },
	{ PS_SEARCHBYEMAIL,     16 },
	{ PS_SEARCHBYNAME,      17 },
	{ PS_SEARCHBYADVANCED,  18 },
	{ PS_CREATEADVSEARCHUI, 19 },
	{ PSR_CONTACTS,         20 },
	{ PSR_FILE,             21 },
	{ PSR_MESSAGE,          22 },
	{ PSS_CONTACTS,         23 },
	{ PSS_FILE,             24 },
	{ PSS_MESSAGE,          25 },
	{ PSS_SETAPPARENTMODE,  26 },
	{ PS_SETSTATUS,         27 },
	{ PSS_GETAWAYMSG,       28 },
	{ PSR_AWAYMSG,          29 },
	{ PS_SETAWAYMSG,        30 },
	{ PSS_USERISTYPING,     31 },
	{ PS_GETNAME,           32 },
	{ PS_GETSTATUS,         33 }
};

//------------------------------------------------------------------------------------

static int CompareProtos(const MBaseProto *p1, const MBaseProto *p2)
{
	if (p1->type != p2->type)
		return p1->type - p2->type;

	return mir_strcmp(p1->szName, p2->szName);
}

LIST<MBaseProto> g_arFilters(10, CompareProtos);

//------------------------------------------------------------------------------------

static PROTO_INTERFACE* defInitProto(const char *szModule, const wchar_t*)
{
	return AddDefaultAccount(szModule);
}

MIR_APP_DLL(PROTOCOLDESCRIPTOR*) Proto_RegisterModule(int type, const char *szName)
{
	if (szName == nullptr)
		return nullptr;

	bool bTryActivate = false;
	MBaseProto *pd = Proto_GetProto(szName);
	if (pd == nullptr) {
		pd = new MBaseProto(szName);
		g_arProtos.insert(pd);
	}
	else bTryActivate = true;

	pd->type = type;

	if (pd->type == PROTOTYPE_PROTOCOL || pd->type == PROTOTYPE_VIRTUAL) {
		// let's create a new container
		PROTO_INTERFACE *ppi = AddDefaultAccount(szName);
		if (ppi) {
			ppi->m_iVersion = 1;
			PROTOACCOUNT *pa = Proto_GetAccount(pd->szName);
			if (pa == nullptr) {
				pa = new PROTOACCOUNT(szName);
				pa->szProtoName = mir_strdup(szName);
				pa->tszAccountName = mir_a2u(szName);
				pa->bIsVisible = pa->bIsEnabled = true;
				pa->iOrder = g_arAccounts.getCount();
				g_arAccounts.insert(pa);
			}
			pa->bOldProto = true;
			pa->bIsVirtual = (pd->type == PROTOTYPE_VIRTUAL);
			pa->ppro = ppi;
			pd->fnInit = defInitProto;
			pd->fnUninit = FreeDefaultAccount;
		}
	}

	if (pd->type != PROTOTYPE_PROTOCOL && pd->type != PROTOTYPE_VIRTUAL && pd->type != PROTOTYPE_PROTOWITHACCS)
		g_arFilters.insert(pd);
	return pd;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Proto_SetUniqueId(const char *szModuleName, const char *pszUniqueId)
{
	if (pszUniqueId == nullptr)
		return;

	PROTOACCOUNT *pa = Proto_GetAccount(szModuleName);
	if (pa != nullptr)
		pa->szUniqueId = mir_strdup(pszUniqueId);
}

MIR_APP_DLL(const char*) Proto_GetUniqueId(const char *szModuleName)
{
	if (szModuleName == nullptr)
		return nullptr;

	const char *szProto;
	PROTOACCOUNT *pa = Proto_GetAccount(szModuleName);
	if (pa != nullptr) {
		if (pa->szUniqueId != nullptr)
			return pa->szUniqueId;

		szProto = pa->szProtoName;
	}
	else szProto = szModuleName;

	MBaseProto *pd = g_arProtos.find((MBaseProto*)&szProto);
	return (pd != nullptr) ? pd->szUniqueId : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Basic core services

MIR_APP_DLL(MEVENT) Proto_AuthRecv(const char *szProtoName, PROTORECVEVENT *pcre)
{
	if (szProtoName == nullptr || pcre == nullptr)
		return 0;

	DBEVENTINFO dbei = {};
	dbei.szModule = (char*)szProtoName;
	dbei.timestamp = pcre->timestamp;
	dbei.flags = DBEF_UTF | pcre->flags & (PREF_CREATEREAD ? DBEF_READ : 0);
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = pcre->lParam;
	dbei.pBlob = (uint8_t*)pcre->szMessage;
	return db_event_add(0, &dbei);
}

/////////////////////////////////////////////////////////////////////////////////////////
// User Typing Notification services

static int Proto_ValidTypingContact(MCONTACT hContact, char *szProto)
{
	if (!hContact || !szProto)
		return 0;

	return (CallProtoServiceInt(0, szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_SUPPORTTYPING) ? 1 : 0;
}

static INT_PTR Proto_SelfIsTyping(WPARAM wParam, LPARAM lParam)
{
	if (lParam == PROTOTYPE_SELFTYPING_OFF || lParam == PROTOTYPE_SELFTYPING_ON) {
		char *szProto = Proto_GetBaseAccountName(wParam);
		if (!szProto)
			return 0;

		if (Proto_ValidTypingContact(wParam, szProto))
			CallProtoServiceInt(0, szProto, PSS_USERISTYPING, wParam, lParam);
	}

	return 0;
}

static INT_PTR Proto_ContactIsTyping(WPARAM wParam, LPARAM lParam)
{
	int type = (int)lParam;
	char *szProto = Proto_GetBaseAccountName(wParam);
	if (!szProto)
		return 0;

	if (Ignore_IsIgnored(wParam, IGNOREEVENT_TYPINGNOTIFY))
		return 0;

	if (type < PROTOTYPE_CONTACTTYPING_OFF)
		return 0;

	if (Proto_ValidTypingContact(wParam, szProto))
		NotifyEventHooks(hTypeEvent, wParam, lParam);

	return 0;
}

void Proto_SetStatus(const char *szProto, unsigned status)
{
	if (CallProtoServiceInt(0, szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) {
		ptrW tszAwayMsg((wchar_t*)CallService(MS_AWAYMSG_GETSTATUSMSGW, status, (LPARAM)szProto));
		CallProtoServiceInt(0, szProto, PS_SETAWAYMSG, status, tszAwayMsg);
	}
	CallProtoServiceInt(0, szProto, PS_SETSTATUS, status, 0);
}

char** __fastcall Proto_FilesMatrixA(wchar_t **files)
{
	if (files == nullptr) return nullptr;

	int count = 0;
	while (files[count++]);

	char** filesA = (char**)mir_alloc(count * sizeof(char*));
	for (int i = 0; i < count; i++)
		filesA[i] = mir_u2a(files[i]);

	return filesA;
}

static wchar_t** __fastcall Proto_FilesMatrixU(char **files)
{
	if (files == nullptr)
		return nullptr;

	int count = 0;
	while (files[count++])
		;

	wchar_t **filesU = (wchar_t**)mir_alloc(count * sizeof(wchar_t*));
	for (int i = 0; i < count; i++)
		filesU[i] = mir_a2u(files[i]);
	return filesU;
}

HICON Proto_GetIcon(PROTO_INTERFACE *ppro, int iconIndex)
{
	if (LOWORD(iconIndex) != PLI_PROTOCOL)
		return nullptr;

	if (iconIndex & PLIF_ICOLIBHANDLE)
		return (HICON)ppro->m_hProtoIcon;

	bool big = (iconIndex & PLIF_SMALL) == 0;
	HICON hIcon = IcoLib_GetIconByHandle(ppro->m_hProtoIcon, big);
	if (iconIndex & PLIF_ICOLIB)
		return hIcon;

	HICON hIcon2 = CopyIcon(hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return hIcon2;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(PROTOACCOUNT*) Proto_GetAccount(const char *accName)
{
	if (accName == nullptr)
		return nullptr;

	return g_arAccounts.find((PROTOACCOUNT*)&accName);
}

MIR_APP_DLL(int) Proto_GetStatus(const char *accName)
{
	if (accName == nullptr)
		return ID_STATUS_OFFLINE;

	PROTOACCOUNT *pa = g_arAccounts.find((PROTOACCOUNT*)&accName);
	return (pa) ? pa->iRealStatus : ID_STATUS_OFFLINE;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Proto_EnumAccounts(int *nAccs, PROTOACCOUNT ***pAccs)
{
	if (nAccs) *nAccs = g_arAccounts.getCount();
	if (pAccs) *pAccs = g_arAccounts.getArray();
}

MIR_APP_DLL(LIST<PROTOACCOUNT>&) Accounts(void)
{
	return g_arAccounts;
}

/////////////////////////////////////////////////////////////////////////////////////////

PROTOACCOUNT::PROTOACCOUNT(const char *szProto) :
	szModuleName(mir_strdup(szProto)),
	iIconBase(-1),
	iRealStatus(ID_STATUS_OFFLINE)
{
	bIsLocked = db_get_b(0, szProto, "LockMainStatus", 0) != 0;
}

PROTOACCOUNT::~PROTOACCOUNT()
{
	mir_free(szModuleName);
	mir_free(szProtoName);
	mir_free(szUniqueId);
	mir_free(tszAccountName);
}

bool PROTOACCOUNT::IsEnabled() const
{
	return (this != nullptr) && ((bIsEnabled && !bDynDisabled) || bOldProto);
}

bool PROTOACCOUNT::IsLocked() const
{
	return (this != nullptr) && bIsLocked;
}

bool PROTOACCOUNT::IsVisible() const
{
	if (this != nullptr && bIsVisible && IsEnabled() && ppro) {
		PROTOCOLDESCRIPTOR *pd = Proto_IsProtocolLoaded(szProtoName);
		if (pd == nullptr || (pd->type != PROTOTYPE_PROTOCOL && pd->type != PROTOTYPE_PROTOWITHACCS))
			return false;

		return (ppro->GetCaps(PFLAGNUM_2, 0) & ~ppro->GetCaps(PFLAGNUM_5, 0));
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) ProtoServiceExists(const char *szModule, const char *szService)
{
	if (szModule == nullptr || szService == nullptr)
		return false;

	PROTOACCOUNT *pa = Proto_GetAccount(szModule);
	if (pa && !pa->bOldProto) {
		TServiceListItem *item = (TServiceListItem*)bsearch(&szService, serviceItems, _countof(serviceItems), sizeof(serviceItems[0]), CompareServiceItems);
		if (item != nullptr)
			return true;
	}

	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, szModule, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	return ServiceExists(str);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(INT_PTR) CallProtoService(const char* szModule, const char* szService, WPARAM wParam, LPARAM lParam)
{
	return CallProtoServiceInt(0, szModule, szService, wParam, lParam);
}

INT_PTR CallProtoServiceInt(MCONTACT hContact, const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam)
{
	auto *ppi = Proto_GetInstance(szModule);
	if (ppi != nullptr) {
		TServiceListItem *item = (TServiceListItem*)bsearch(&szService, serviceItems, _countof(serviceItems), sizeof(serviceItems[0]), CompareServiceItems);
		if (item) {
			switch (item->id) {
			case  1: return (INT_PTR)ppi->AddToList(wParam, (PROTOSEARCHRESULT *)lParam);
			case  2: return (INT_PTR)ppi->AddToListByEvent(LOWORD(wParam), HIWORD(wParam), (MEVENT)lParam);
			case  3: return (INT_PTR)ppi->Authorize((MEVENT)wParam);
			case  4: return (INT_PTR)ppi->AuthDeny((MEVENT)wParam, (wchar_t *)lParam);
			case  5: return (INT_PTR)ppi->AuthRecv(hContact, (PROTORECVEVENT *)lParam);
			case  6: return (INT_PTR)ppi->AuthRequest(hContact, (wchar_t *)lParam);
			case  8: return (INT_PTR)ppi->FileAllow(hContact, (HANDLE)wParam, (wchar_t *)lParam);
			case  9: return (INT_PTR)ppi->FileCancel(hContact, (HANDLE)wParam);
			case 10: return (INT_PTR)ppi->FileDeny(hContact, (HANDLE)wParam, (wchar_t *)lParam);
			case 11: {
					PROTOFILERESUME *pfr = (PROTOFILERESUME *)lParam;
					return (INT_PTR)ppi->FileResume((HANDLE)wParam, pfr->action, (const wchar_t *)pfr->szFilename);
				}

			case 12: return (INT_PTR)ppi->GetCaps(wParam, lParam);
			case 13: return (INT_PTR)Proto_GetIcon(ppi, wParam);
			case 14: return (INT_PTR)ppi->GetInfo(hContact, wParam);
			case 15: return (INT_PTR)ppi->SearchBasic((wchar_t *)lParam);
			case 16:	return (INT_PTR)ppi->SearchByEmail((wchar_t *)lParam);
			case 17: {
					PROTOSEARCHBYNAME *psbn = (PROTOSEARCHBYNAME *)lParam;
					return (INT_PTR)ppi->SearchByName(psbn->pszNick, psbn->pszFirstName, psbn->pszLastName);
				}
			case 18: return (INT_PTR)ppi->SearchAdvanced((HWND)lParam);
			case 19: return (INT_PTR)ppi->CreateExtendedSearchUI((HWND)lParam);
			case 20: return (INT_PTR)ppi->RecvContacts(hContact, (PROTORECVEVENT *)lParam);
			case 21: return (INT_PTR)ppi->RecvFile(hContact, (PROTORECVFILE *)lParam);
			case 22: return (INT_PTR)ppi->RecvMsg(hContact, (PROTORECVEVENT *)lParam);
			case 23: return (INT_PTR)ppi->SendContacts(hContact, LOWORD(wParam), HIWORD(wParam), (MCONTACT *)lParam);
			case 24: return (INT_PTR)ppi->SendFile(hContact, (wchar_t *)wParam, (wchar_t **)lParam);
			case 25: return (INT_PTR)ppi->SendMsg(hContact, wParam, (const char *)lParam);
			case 26: return (INT_PTR)ppi->SetApparentMode(hContact, wParam);
			case 27: return (INT_PTR)ppi->SetStatus(wParam);
			case 28: return (INT_PTR)ppi->GetAwayMsg(hContact);
			case 29: return (INT_PTR)ppi->RecvAwayMsg(hContact, wParam, (PROTORECVEVENT *)lParam);
			case 30: return (INT_PTR)ppi->SetAwayMsg(wParam, (wchar_t *)lParam);
			case 31: return (INT_PTR)ppi->UserIsTyping(wParam, lParam);
			case 32: mir_strncpy((char *)lParam, ppi->m_szModuleName, wParam); return 0;
			case 33:
				return ppi->m_iStatus;
			}
		}
	}

	return ProtoCallService(szModule, szService, wParam, lParam);
}

INT_PTR ProtoCallService(const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam)
{
	if (szModule == nullptr || szService == nullptr)
		return false;

	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, szModule, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	return CallService(str, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvProto_GetContactBaseAccount(WPARAM wParam, LPARAM)
{
	return (INT_PTR)Proto_GetBaseAccountName(wParam);
}

int LoadProtocolsModule(void)
{
	bModuleInitialized = TRUE;

	qsort(serviceItems, _countof(serviceItems), sizeof(serviceItems[0]), CompareServiceItems);

	hAckEvent = CreateHookableEvent(ME_PROTO_ACK);
	hTypeEvent = CreateHookableEvent(ME_PROTO_CONTACTISTYPING);
	hAccListChanged = CreateHookableEvent(ME_PROTO_ACCLISTCHANGED);

	CreateServiceFunction(MS_PROTO_SELFISTYPING, Proto_SelfIsTyping);
	CreateServiceFunction(MS_PROTO_CONTACTISTYPING, Proto_ContactIsTyping);

	// just to make QuickSearch happy
	CreateServiceFunction("Proto/GetContactBaseAccount", srvProto_GetContactBaseAccount);
	
	CreateServiceFunction(MS_PROTO_HIDDENSTUB, stubChainRecv);

	return LoadProtoOptions();
}

void UnloadProtocolsModule()
{
	if (!bModuleInitialized)
		return;

	g_arProtos.destroy();

	if (hAckEvent) {
		DestroyHookableEvent(hAckEvent);
		hAckEvent = nullptr;
	}

	if (hAccListChanged) {
		DestroyHookableEvent(hAccListChanged);
		hAccListChanged = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

pfnUninitProto GetProtocolDestructor(char *szProto)
{
	MBaseProto *p = Proto_GetProto(szProto);
	return (p == nullptr) ? nullptr : p->fnUninit;
}
