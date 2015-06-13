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

int LoadProtoChains(void);
int LoadProtoOptions(void);

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
	{ PSR_URL,              23 },
	{ PSS_CONTACTS,         24 },
	{ PSS_FILE,             25 },
	{ PSS_MESSAGE,          26 },
	{ PSS_URL,              27 },
	{ PSS_SETAPPARENTMODE,  28 },
	{ PS_SETSTATUS,         29 },
	{ PSS_GETAWAYMSG,       30 },
	{ PSR_AWAYMSG,          31 },
	{ PS_SETAWAYMSG,        33 },
	{ PSS_USERISTYPING,     34 },
	{ PS_GETNAME,           35 },
	{ PS_GETSTATUS,         36 }
};

//------------------------------------------------------------------------------------

static int CompareProtos(const PROTOCOLDESCRIPTOR *p1, const PROTOCOLDESCRIPTOR *p2)
{
	if (p1->type != p2->type)
		return p1->type - p2->type;

	return mir_strcmp(p1->szName, p2->szName);
}

LIST<PROTOCOLDESCRIPTOR> filters(10, CompareProtos);

//------------------------------------------------------------------------------------

void FreeFilesMatrix(TCHAR ***files);

INT_PTR srvProto_IsLoaded(WPARAM, LPARAM lParam)
{
	return (INT_PTR)Proto_IsProtocolLoaded((char*)lParam);
}

static PROTO_INTERFACE* defInitProto(const char* szModule, const TCHAR*)
{
	return AddDefaultAccount(szModule);
}

static INT_PTR srvProto_RegisterModule(WPARAM, LPARAM lParam)
{
	PROTOCOLDESCRIPTOR *pd = (PROTOCOLDESCRIPTOR*)lParam;
	if (pd->cbSize != sizeof(PROTOCOLDESCRIPTOR) && pd->cbSize != PROTOCOLDESCRIPTOR_V3_SIZE)
		return 1;

	PROTOCOLDESCRIPTOR *p = Proto_RegisterModule(pd);
	if (p == NULL)
		return 2;

	if (p->fnInit == NULL && (p->type == PROTOTYPE_PROTOCOL || p->type == PROTOTYPE_VIRTUAL)) {
		// let's create a new container
		PROTO_INTERFACE* ppi = AddDefaultAccount(pd->szName);
		if (ppi) {
			ppi->m_iVersion = (pd->cbSize == PROTOCOLDESCRIPTOR_V3_SIZE) ? 1 : 2;
			PROTOACCOUNT *pa = Proto_GetAccount(pd->szName);
			if (pa == NULL) {
				pa = (PROTOACCOUNT*)mir_calloc(sizeof(PROTOACCOUNT));
				pa->cbSize = sizeof(PROTOACCOUNT);
				pa->szModuleName = mir_strdup(pd->szName);
				pa->szProtoName = mir_strdup(pd->szName);
				pa->tszAccountName = mir_a2t(pd->szName);
				pa->bIsVisible = pa->bIsEnabled = true;
				pa->iOrder = accounts.getCount();
				accounts.insert(pa);
			}
			pa->bOldProto = true;
			pa->bIsVirtual = (p->type == PROTOTYPE_VIRTUAL);
			pa->ppro = ppi;
			p->fnInit = defInitProto;
			p->fnUninit = FreeDefaultAccount;
		}
	}

	if (p->type != PROTOTYPE_PROTOCOL && p->type != PROTOTYPE_VIRTUAL)
		filters.insert(p);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Basic core services

static INT_PTR Proto_RecvMessage(WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT*)ccs->lParam;
	if (pre->szMessage == NULL)
		return NULL;

	ptrA pszTemp;
	mir_ptr<BYTE> pszBlob;

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof(dbei);
	dbei.flags = DBEF_UTF;
	dbei.szModule = GetContactProto(ccs->hContact);
	dbei.timestamp = pre->timestamp;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (DWORD)mir_strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;

	if (pre->cbCustomDataSize != 0) {
		pszBlob = (PBYTE)mir_alloc(dbei.cbBlob + pre->cbCustomDataSize);
		memcpy(pszBlob, dbei.pBlob, dbei.cbBlob);
		memcpy((PBYTE)pszBlob + dbei.cbBlob, pre->pCustomData, pre->cbCustomDataSize);
		dbei.pBlob = pszBlob;
		dbei.cbBlob += pre->cbCustomDataSize;
	}

	if (pre->flags & PREF_CREATEREAD)
		dbei.flags |= DBEF_READ;
	if (pre->flags & PREF_SENT)
		dbei.flags |= DBEF_SENT;

	return (INT_PTR)db_event_add(ccs->hContact, &dbei);
}

static INT_PTR Proto_AuthRecv(WPARAM wParam, LPARAM lParam)
{
	PROTORECVEVENT* pre = (PROTORECVEVENT*)lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = (char*)wParam;
	dbei.timestamp = pre->timestamp;
	dbei.flags = DBEF_UTF | pre->flags & (PREF_CREATEREAD ? DBEF_READ : 0);
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = pre->lParam;
	dbei.pBlob = (PBYTE)pre->szMessage;
	return (INT_PTR)db_event_add(NULL, &dbei);
}

/////////////////////////////////////////////////////////////////////////////////////////
// User Typing Notification services

static int Proto_ValidTypingContact(MCONTACT hContact, char *szProto)
{
	if (!hContact || !szProto)
		return 0;

	return (CallProtoServiceInt(NULL, szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_SUPPORTTYPING) ? 1 : 0;
}

static INT_PTR Proto_SelfIsTyping(WPARAM wParam, LPARAM lParam)
{
	if (lParam == PROTOTYPE_SELFTYPING_OFF || lParam == PROTOTYPE_SELFTYPING_ON) {
		char *szProto = GetContactProto(wParam);
		if (!szProto)
			return 0;

		if (Proto_ValidTypingContact(wParam, szProto))
			CallProtoServiceInt(NULL, szProto, PSS_USERISTYPING, wParam, lParam);
	}

	return 0;
}

static INT_PTR Proto_ContactIsTyping(WPARAM wParam, LPARAM lParam)
{
	int type = (int)lParam;
	char *szProto = GetContactProto(wParam);
	if (!szProto)
		return 0;

	if (CallService(MS_IGNORE_ISIGNORED, wParam, IGNOREEVENT_TYPINGNOTIFY))
		return 0;

	if (type < PROTOTYPE_CONTACTTYPING_OFF)
		return 0;

	if (Proto_ValidTypingContact(wParam, szProto))
		NotifyEventHooks(hTypeEvent, wParam, lParam);

	return 0;
}

void Proto_SetStatus(const char *szProto, unsigned status)
{
	if (CallProtoServiceInt(NULL, szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) {
		ptrT tszAwayMsg((TCHAR*)CallService(MS_AWAYMSG_GETSTATUSMSGT, status, (LPARAM)szProto));
		CallProtoServiceInt(NULL, szProto, PS_SETAWAYMSG, status, tszAwayMsg);
	}
	CallProtoServiceInt(NULL, szProto, PS_SETSTATUS, status, 0);
}

char** __fastcall Proto_FilesMatrixA(wchar_t **files)
{
	if (files == NULL) return NULL;

	int count = 0;
	while (files[count++]);

	char** filesA = (char**)mir_alloc(count * sizeof(char*));
	for (int i = 0; i < count; i++)
		filesA[i] = mir_u2a(files[i]);

	return filesA;
}

static wchar_t** __fastcall Proto_FilesMatrixU(char **files)
{
	if (files == NULL) return NULL;

	int count = 0;
	while (files[count++]);

	wchar_t** filesU = (wchar_t**)mir_alloc(count * sizeof(wchar_t*));
	for (int i = 0; i < count; i++)
		filesU[i] = mir_a2u(files[i]);

	return filesU;
}

HICON Proto_GetIcon(PROTO_INTERFACE *ppro, int iconIndex)
{
	if (LOWORD(iconIndex) != PLI_PROTOCOL)
		return NULL;

	if (iconIndex & PLIF_ICOLIBHANDLE)
		return (HICON)ppro->m_hProtoIcon;

	bool big = (iconIndex & PLIF_SMALL) == 0;
	HICON hIcon = Skin_GetIconByHandle(ppro->m_hProtoIcon, big);
	if (iconIndex & PLIF_ICOLIB)
		return hIcon;

	HICON hIcon2 = CopyIcon(hIcon);
	Skin_ReleaseIcon(hIcon);
	return hIcon2;
}

/////////////////////////////////////////////////////////////////////////////////////////
// 0.8.0+ - accounts

PROTOACCOUNT* __fastcall Proto_GetAccount(const char* accName)
{
	if (accName == NULL)
		return NULL;

	int idx;
	PROTOACCOUNT temp;
	temp.szModuleName = (char*)accName;
	if ((idx = accounts.getIndex(&temp)) == -1)
		return NULL;

	return accounts[idx];
}

static INT_PTR srvProto_CreateAccount(WPARAM, LPARAM lParam)
{
	ACC_CREATE *p = (ACC_CREATE*)lParam;
	if (p == NULL)
		return NULL;

	PROTOACCOUNT *pa = Proto_CreateAccount(p->pszInternal, p->pszBaseProto, p->ptszAccountName);
	if (pa) {
		WriteDbAccounts();
		NotifyEventHooks(hAccListChanged, PRAC_ADDED, (LPARAM)pa);
	}
	return (INT_PTR)pa;
}

static INT_PTR srvProto_GetAccount(WPARAM, LPARAM lParam)
{
	return (INT_PTR)Proto_GetAccount((char*)lParam);
}

static INT_PTR Proto_EnumAccounts(WPARAM wParam, LPARAM lParam)
{
	*(int*)wParam = accounts.getCount();
	*(PROTOACCOUNT***)lParam = accounts.getArray();
	return 0;
}

bool __fastcall Proto_IsAccountEnabled(PROTOACCOUNT *pa)
{
	return pa && ((pa->bIsEnabled && !pa->bDynDisabled) || pa->bOldProto);
}

static INT_PTR srvProto_IsAccountEnabled(WPARAM, LPARAM lParam)
{
	return (INT_PTR)Proto_IsAccountEnabled((PROTOACCOUNT*)lParam);
}

bool __fastcall Proto_IsAccountLocked(PROTOACCOUNT *pa)
{
	return pa && db_get_b(NULL, pa->szModuleName, "LockMainStatus", 0) != 0;
}

static INT_PTR srvProto_IsAccountLocked(WPARAM, LPARAM lParam)
{
	return (INT_PTR)Proto_IsAccountLocked(Proto_GetAccount((char*)lParam));
}

static INT_PTR Proto_BroadcastAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	return ProtoBroadcastAck(ack->szModule, ack->hContact, ack->type, ack->result, ack->hProcess, ack->lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) ProtoServiceExists(const char *szModule, const char *szService)
{
	if (szModule == NULL || szService == NULL)
		return false;

	TServiceListItem *item = (TServiceListItem*)bsearch(&szService, serviceItems, _countof(serviceItems), sizeof(serviceItems[0]), CompareServiceItems);
	if (item != NULL)
		return true;

	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, szModule, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	return ServiceExists(str);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(INT_PTR) CallProtoService(const char* szModule, const char* szService, WPARAM wParam, LPARAM lParam)
{
	return CallProtoServiceInt(NULL, szModule, szService, wParam, lParam);
}

INT_PTR CallProtoServiceInt(MCONTACT hContact, const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *pa = Proto_GetAccount(szModule);
	if (pa && !pa->bOldProto) {
		PROTO_INTERFACE *ppi = pa->ppro;
		if (ppi != NULL && ppi->m_iVersion > 1) {
			TServiceListItem *item = (TServiceListItem*)bsearch(&szService, serviceItems, _countof(serviceItems), sizeof(serviceItems[0]), CompareServiceItems);
			if (item) {
				switch (item->id) {
				case  1: return (INT_PTR)ppi->AddToList(wParam, (PROTOSEARCHRESULT*)lParam);
				case  2: return (INT_PTR)ppi->AddToListByEvent(LOWORD(wParam), HIWORD(wParam), (MEVENT)lParam);
				case  3: return (INT_PTR)ppi->Authorize((MEVENT)wParam);
				case  4: return (INT_PTR)ppi->AuthDeny((MEVENT)wParam, (TCHAR*)lParam);
				case  5: return (INT_PTR)ppi->AuthRecv(hContact, (PROTORECVEVENT*)lParam);
				case  6: return (INT_PTR)ppi->AuthRequest(hContact, (TCHAR*)lParam);
				case  8: return (INT_PTR)ppi->FileAllow(hContact, (HANDLE)wParam, (TCHAR*)lParam);
				case  9: return (INT_PTR)ppi->FileCancel(hContact, (HANDLE)wParam);
				case 10: return (INT_PTR)ppi->FileDeny(hContact, (HANDLE)wParam, (TCHAR*)lParam);
				case 11: {
						PROTOFILERESUME *pfr = (PROTOFILERESUME*)lParam;
						return (INT_PTR)ppi->FileResume((HANDLE)wParam, &pfr->action, (const TCHAR**)&pfr->szFilename);
					}

				case 12: return (INT_PTR)ppi->GetCaps(wParam, lParam);
				case 13: return (INT_PTR)Proto_GetIcon(ppi, wParam);
				case 14: return (INT_PTR)ppi->GetInfo(hContact, wParam);
				case 15: return (INT_PTR)ppi->SearchBasic((TCHAR*)lParam);
				case 16:	return (INT_PTR)ppi->SearchByEmail((TCHAR*)lParam);
				case 17: {
						PROTOSEARCHBYNAME* psbn = (PROTOSEARCHBYNAME*)lParam;
						return (INT_PTR)ppi->SearchByName(psbn->pszNick, psbn->pszFirstName, psbn->pszLastName);
					}
				case 18: return (INT_PTR)ppi->SearchAdvanced((HWND)lParam);
				case 19: return (INT_PTR)ppi->CreateExtendedSearchUI((HWND)lParam);
				case 20: return (INT_PTR)ppi->RecvContacts(hContact, (PROTORECVEVENT*)lParam);
				case 21: return (INT_PTR)ppi->RecvFile(hContact, (PROTORECVFILET*)lParam);
				case 22: return (INT_PTR)ppi->RecvMsg(hContact, (PROTORECVEVENT*)lParam);
				case 23: return (INT_PTR)ppi->RecvUrl(hContact, (PROTORECVEVENT*)lParam);
				case 24: return (INT_PTR)ppi->SendContacts(hContact, LOWORD(wParam), HIWORD(wParam), (MCONTACT*)lParam);
				case 25: return (INT_PTR)ppi->SendFile(hContact, (TCHAR*)wParam, (TCHAR**)lParam);
				case 26: return (INT_PTR)ppi->SendMsg(hContact, wParam, (const char*)lParam);
				case 27: return (INT_PTR)ppi->SendUrl(hContact, wParam, (const char*)lParam);
				case 28: return (INT_PTR)ppi->SetApparentMode(hContact, wParam);
				case 29: return (INT_PTR)ppi->SetStatus(wParam);
				case 30: return (INT_PTR)ppi->GetAwayMsg(hContact);
				case 31: return (INT_PTR)ppi->RecvAwayMsg(hContact, wParam, (PROTORECVEVENT*)lParam);
				case 33: return (INT_PTR)ppi->SetAwayMsg(wParam, (TCHAR*)lParam);
				case 34: return (INT_PTR)ppi->UserIsTyping(wParam, lParam);
				case 35: mir_strncpy((char*)lParam, ppi->m_szModuleName, wParam); return 0;
				case 36:
					return ppi->m_iStatus;
				}
			}
		}
	}

	return ProtoCallService(szModule, szService, wParam, lParam);
}

INT_PTR ProtoCallService(const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam)
{
	if (szModule == NULL || szService == NULL)
		return false;

	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, szModule, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	return CallService(str, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadProtocolsModule(void)
{
	bModuleInitialized = TRUE;

	if (LoadProtoChains())
		return 1;

	qsort(serviceItems, _countof(serviceItems), sizeof(serviceItems[0]), CompareServiceItems);

	hTypeEvent = CreateHookableEvent(ME_PROTO_CONTACTISTYPING);
	hAccListChanged = CreateHookableEvent(ME_PROTO_ACCLISTCHANGED);

	CreateServiceFunction(MS_PROTO_BROADCASTACK, Proto_BroadcastAck);
	CreateServiceFunction(MS_PROTO_ISPROTOCOLLOADED, srvProto_IsLoaded);
	CreateServiceFunction(MS_PROTO_REGISTERMODULE, srvProto_RegisterModule);
	CreateServiceFunction(MS_PROTO_SELFISTYPING, Proto_SelfIsTyping);
	CreateServiceFunction(MS_PROTO_CONTACTISTYPING, Proto_ContactIsTyping);

	CreateServiceFunction(MS_PROTO_RECVMSG, Proto_RecvMessage);
	CreateServiceFunction(MS_PROTO_AUTHRECV, Proto_AuthRecv);

	CreateServiceFunction("Proto/EnumProtocols", Proto_EnumAccounts);
	CreateServiceFunction(MS_PROTO_ENUMACCOUNTS, Proto_EnumAccounts);
	CreateServiceFunction(MS_PROTO_CREATEACCOUNT, srvProto_CreateAccount);
	CreateServiceFunction(MS_PROTO_GETACCOUNT, srvProto_GetAccount);

	CreateServiceFunction(MS_PROTO_ISACCOUNTENABLED, srvProto_IsAccountEnabled);
	CreateServiceFunction(MS_PROTO_ISACCOUNTLOCKED, srvProto_IsAccountLocked);

	return LoadProtoOptions();
}

void UnloadProtocolsModule()
{
	if (!bModuleInitialized) return;

	if (hAccListChanged) {
		DestroyHookableEvent(hAccListChanged);
		hAccListChanged = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

pfnUninitProto GetProtocolDestructor(char *szProto)
{
	PROTOCOLDESCRIPTOR *p = Proto_IsProtocolLoaded(szProto);
	return (p == NULL) ? NULL : p->fnUninit;
}
