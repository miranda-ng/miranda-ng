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

int LoadProtoChains(void);
int LoadProtoOptions(void);

HANDLE hAccListChanged;
static HANDLE hTypeEvent;
static BOOL bModuleInitialized = FALSE;

typedef struct
{
	const char* name;
	int id;
}
TServiceListItem;

static int CompareServiceItems(const TServiceListItem* p1, const TServiceListItem* p2)
{
	return strcmp(p1->name, p2->name);
}

static LIST<TServiceListItem> serviceItems(10, CompareServiceItems);

//------------------------------------------------------------------------------------

static int CompareProtos(const PROTOCOLDESCRIPTOR *p1, const PROTOCOLDESCRIPTOR *p2)
{
	if (p1->type != p2->type)
		return p1->type - p2->type;

	return strcmp(p1->szName, p2->szName);
}

LIST<PROTOCOLDESCRIPTOR> filters(10, CompareProtos);

//------------------------------------------------------------------------------------

void FreeFilesMatrix(TCHAR ***files);

INT_PTR srvProto_IsLoaded(WPARAM, LPARAM lParam)
{
	return (INT_PTR)Proto_IsProtocolLoaded((char*)lParam);
}

static PROTO_INTERFACE* defInitProto(const char* szModuleName, const TCHAR*)
{
	return AddDefaultAccount(szModuleName);
}

static INT_PTR srvProto_RegisterModule(WPARAM, LPARAM lParam)
{
	PROTOCOLDESCRIPTOR *pd = (PROTOCOLDESCRIPTOR*)lParam;
	if (pd->cbSize != sizeof(PROTOCOLDESCRIPTOR) && pd->cbSize != PROTOCOLDESCRIPTOR_V3_SIZE)
		return 1;

	PROTOCOLDESCRIPTOR *p = Proto_RegisterModule(pd);
	if (p == NULL)
		return 2;

	if (pd->cbSize == PROTOCOLDESCRIPTOR_V3_SIZE) {
		if (p->type == PROTOTYPE_PROTOCOL || p->type == PROTOTYPE_VIRTUAL) {
			// let's create a new container
			PROTO_INTERFACE* ppi = AddDefaultAccount(pd->szName);
			if (ppi) {
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

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = GetContactProto(ccs->hContact);
	dbei.timestamp = pre->timestamp;
	dbei.eventType = EVENTTYPE_MESSAGE;
	if (pre->flags & PREF_UNICODE) {
		pszTemp = mir_utf8encodeT(pre->tszMessage);
		dbei.pBlob = (PBYTE)(char*)pszTemp;
		dbei.cbBlob = (DWORD)strlen(pszTemp) + 1;
		dbei.flags |= DBEF_UTF;
	}
	else {
		dbei.cbBlob = (DWORD)strlen(pre->szMessage) + 1;
		dbei.pBlob = (PBYTE)pre->szMessage;
	}

	if (pre->cbCustomDataSize != 0) {
		pszBlob = (PBYTE)mir_alloc(dbei.cbBlob + pre->cbCustomDataSize);
		memcpy(pszBlob, dbei.pBlob, dbei.cbBlob);
		memcpy((PBYTE)pszBlob + dbei.cbBlob, pre->pCustomData, pre->cbCustomDataSize);
		dbei.pBlob = pszBlob;
		dbei.cbBlob += pre->cbCustomDataSize;
	}

	if (pre->flags & PREF_CREATEREAD)
		dbei.flags |= DBEF_READ;
	if (pre->flags & PREF_UTF)
		dbei.flags |= DBEF_UTF;
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
	dbei.flags = pre->flags & (PREF_CREATEREAD ? DBEF_READ : 0);
	dbei.flags |= (pre->flags & PREF_UTF) ? DBEF_UTF : 0;
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
		TCHAR *awayMsg = (TCHAR*)CallService(MS_AWAYMSG_GETSTATUSMSGT, status, (LPARAM)szProto);
		CallProtoServiceInt(NULL, szProto, PS_SETAWAYMSGT, status, (LPARAM)awayMsg);
		mir_free(awayMsg);
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
	if (LOWORD(iconIndex) == PLI_PROTOCOL) {
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
	return NULL;
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

INT_PTR CallProtoService(const char* szModule, const char* szService, WPARAM wParam, LPARAM lParam)
{
	return CallProtoServiceInt(NULL, szModule, szService, wParam, lParam);
}

INT_PTR CallProtoServiceInt(MCONTACT hContact, const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *pa = Proto_GetAccount(szModule);
	if (pa && !pa->bOldProto) {
		PROTO_INTERFACE* ppi;
		if ((ppi = pa->ppro) == NULL)
			return CALLSERVICE_NOTFOUND;

		TServiceListItem *item = serviceItems.find((TServiceListItem*)&szService);
		if (item) {
			switch (item->id) {
			case 1:
				if (ppi->m_iVersion > 1 || !(((PROTOSEARCHRESULT*)lParam)->flags & PSR_UNICODE))
					return (INT_PTR)ppi->AddToList(wParam, (PROTOSEARCHRESULT*)lParam);
				else {
					PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT*)lParam;
					PROTOSEARCHRESULT *psra = (PROTOSEARCHRESULT*)mir_alloc(psr->cbSize);
					memcpy(psra, psr, psr->cbSize);
					psra->nick = (PROTOCHAR*)mir_u2a(psr->nick);
					psra->firstName = (PROTOCHAR*)mir_u2a(psr->firstName);
					psra->lastName = (PROTOCHAR*)mir_u2a(psr->lastName);
					psra->email = (PROTOCHAR*)mir_u2a(psr->email);

					INT_PTR res = (INT_PTR)ppi->AddToList(wParam, psra);

					mir_free(psra->nick);
					mir_free(psra->firstName);
					mir_free(psra->lastName);
					mir_free(psra->email);
					mir_free(psra);

					return res;
				}

			case 2: return (INT_PTR)ppi->AddToListByEvent(LOWORD(wParam), HIWORD(wParam), (MEVENT)lParam);
			case 3: return (INT_PTR)ppi->Authorize((MEVENT)wParam);
			case 4:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->AuthDeny((MEVENT)wParam, _A2T((char*)lParam));
				return (INT_PTR)ppi->AuthDeny((MEVENT)wParam, (PROTOCHAR*)lParam);
			case 5: return (INT_PTR)ppi->AuthRecv(hContact, (PROTORECVEVENT*)lParam);
			case 6:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->AuthRequest(hContact, _A2T((char*)lParam));
				return (INT_PTR)ppi->AuthRequest(hContact, (PROTOCHAR*)lParam);
			case 8:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->FileAllow(hContact, (HANDLE)wParam, _A2T((char*)lParam));
				return (INT_PTR)ppi->FileAllow(hContact, (HANDLE)wParam, (PROTOCHAR*)lParam);
			case 9: return (INT_PTR)ppi->FileCancel(hContact, (HANDLE)wParam);
			case 10:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->FileDeny(hContact, (HANDLE)wParam, _A2T((char*)lParam));
				return (INT_PTR)ppi->FileDeny(hContact, (HANDLE)wParam, (PROTOCHAR*)lParam);
			case 11: {
				PROTOFILERESUME* pfr = (PROTOFILERESUME*)lParam;
				if (ppi->m_iVersion > 1) {
					PROTOCHAR* szFname = mir_a2t((char*)pfr->szFilename);
					INT_PTR res = (INT_PTR)ppi->FileResume((HANDLE)wParam, &pfr->action, (const PROTOCHAR**)&szFname);
					mir_free((PROTOCHAR*)pfr->szFilename);
					pfr->szFilename = (PROTOCHAR*)mir_t2a(szFname); mir_free(szFname);
				}
				else
					return (INT_PTR)ppi->FileResume((HANDLE)wParam, &pfr->action, (const PROTOCHAR**)&pfr->szFilename);
			}
			case 12: return (INT_PTR)ppi->GetCaps(wParam, lParam);
			case 13: return (INT_PTR)Proto_GetIcon(ppi, wParam);
			case 14: return (INT_PTR)ppi->GetInfo(hContact, wParam);
			case 15:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->SearchBasic(_A2T((char*)lParam));
				return (INT_PTR)ppi->SearchBasic((TCHAR*)lParam);
			case 16:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->SearchByEmail(_A2T((char*)lParam));
				return (INT_PTR)ppi->SearchByEmail((TCHAR*)lParam);
			case 17: {
				PROTOSEARCHBYNAME* psbn = (PROTOSEARCHBYNAME*)lParam;
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->SearchByName(_A2T((char*)psbn->pszNick), _A2T((char*)psbn->pszFirstName), _A2T((char*)psbn->pszLastName));
				else
					return (INT_PTR)ppi->SearchByName(psbn->pszNick, psbn->pszFirstName, psbn->pszLastName);
			}
			case 18: return (INT_PTR)ppi->SearchAdvanced((HWND)lParam);
			case 19: return (INT_PTR)ppi->CreateExtendedSearchUI((HWND)lParam);
			case 20: return (INT_PTR)ppi->RecvContacts(hContact, (PROTORECVEVENT*)lParam);
			case 21: return (INT_PTR)ppi->RecvFile(hContact, (PROTOFILEEVENT*)lParam);
			case 22: return (INT_PTR)ppi->RecvMsg(hContact, (PROTORECVEVENT*)lParam);
			case 23: return (INT_PTR)ppi->RecvUrl(hContact, (PROTORECVEVENT*)lParam);
			case 24: return (INT_PTR)ppi->SendContacts(hContact, LOWORD(wParam), HIWORD(wParam), (MCONTACT*)lParam);
			case 25:
				if (ppi->m_iVersion > 1) {
					TCHAR** files = Proto_FilesMatrixU((char**)lParam);
					INT_PTR res = (INT_PTR)ppi->SendFile(hContact, _A2T((char*)wParam), (TCHAR**)files);
					if (res == 0) FreeFilesMatrix(&files);
					return res;
				}
				return (INT_PTR)ppi->SendFile(hContact, (TCHAR*)wParam, (TCHAR**)lParam);

			case 26: return (INT_PTR)ppi->SendMsg(hContact, wParam, (const char*)lParam);
			case 27: return (INT_PTR)ppi->SendUrl(hContact, wParam, (const char*)lParam);
			case 28: return (INT_PTR)ppi->SetApparentMode(hContact, wParam);
			case 29: return (INT_PTR)ppi->SetStatus(wParam);
			case 30: return (INT_PTR)ppi->GetAwayMsg(hContact);
			case 31: return (INT_PTR)ppi->RecvAwayMsg(hContact, wParam, (PROTORECVEVENT*)lParam);
			case 33:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->SetAwayMsg(wParam, _A2T((char*)lParam));
				return (INT_PTR)ppi->SetAwayMsg(wParam, (TCHAR*)lParam);
			case 34: return (INT_PTR)ppi->UserIsTyping(wParam, lParam);
			case 35: mir_strncpy((char*)lParam, ppi->m_szModuleName, wParam); return 0;
			case 36: return ppi->m_iStatus;

			case 100:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->SetAwayMsg(wParam, (TCHAR*)lParam);
				return (INT_PTR)ppi->SetAwayMsg(wParam, StrConvA((TCHAR*)lParam));
			case 102:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->SendFile(hContact, (TCHAR*)wParam, (TCHAR**)lParam);
				else {
					char** files = Proto_FilesMatrixA((TCHAR**)lParam);
					INT_PTR res = (INT_PTR)ppi->SendFile(hContact, StrConvA((TCHAR*)wParam), (TCHAR**)files);
					if (res == 0) FreeFilesMatrix((TCHAR***)&files);
					return res;
				}
			case 103:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->FileAllow(hContact, (HANDLE)wParam, (TCHAR*)lParam);
				return (INT_PTR)ppi->FileAllow(hContact, (HANDLE)wParam, StrConvA((TCHAR*)lParam));
			case 104:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->FileDeny(hContact, (HANDLE)wParam, (TCHAR*)lParam);
				return (INT_PTR)ppi->FileDeny(hContact, (HANDLE)wParam, StrConvA((TCHAR*)lParam));
			case 105: {
				PROTOFILERESUME* pfr = (PROTOFILERESUME*)lParam;
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->FileResume((HANDLE)wParam, &pfr->action, (const PROTOCHAR**)&pfr->szFilename);
				else {
					char* szFname = mir_t2a(pfr->szFilename);
					INT_PTR res = (INT_PTR)ppi->FileResume((HANDLE)wParam, &pfr->action,
						(const PROTOCHAR**)&szFname);
					mir_free(szFname);
					return (INT_PTR) res;
				}
			}
			case 106:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->AuthRequest(hContact, (const TCHAR*)lParam);
				return (INT_PTR)ppi->AuthRequest(hContact, StrConvA((const TCHAR*)lParam));
			case 107:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->AuthDeny((MEVENT)wParam, (const TCHAR*)lParam);
				return (INT_PTR)ppi->AuthDeny((MEVENT)wParam, StrConvA((const TCHAR*)lParam));
			case 108:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->SearchBasic((const TCHAR*)lParam);
				return (INT_PTR)ppi->SearchBasic(StrConvA((const TCHAR*)lParam));
			case 109: {
				PROTOSEARCHBYNAME* psbn = (PROTOSEARCHBYNAME*)lParam;
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->SearchByName(psbn->pszNick, psbn->pszFirstName, psbn->pszLastName);
				return (INT_PTR)ppi->SearchByName(StrConvA((TCHAR*)psbn->pszNick),
					StrConvA((TCHAR*)psbn->pszFirstName), StrConvA((TCHAR*)psbn->pszLastName));
			}
			case 110:
				if (ppi->m_iVersion > 1)
					return (INT_PTR)ppi->SearchByEmail((const TCHAR*)lParam);
				return (INT_PTR)ppi->SearchByEmail(StrConvA((const TCHAR*)lParam));
			}
		}
	}

	if (!strcmp(szService, PS_ADDTOLIST)) {
		PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT*)lParam;
		if (!(psr->flags & PSR_UNICODE)) {
			PROTOSEARCHRESULT *psra = (PROTOSEARCHRESULT*)mir_alloc(psr->cbSize);
			memcpy(psra, psr, psr->cbSize);
			psra->nick = (PROTOCHAR*)mir_u2a(psr->nick);
			psra->firstName = (PROTOCHAR*)mir_u2a(psr->firstName);
			psra->lastName = (PROTOCHAR*)mir_u2a(psr->lastName);
			psra->email = (PROTOCHAR*)mir_u2a(psr->email);

			INT_PTR res = ProtoCallService(szModule, szService, wParam, (LPARAM)psra);

			mir_free(psra->nick);
			mir_free(psra->firstName);
			mir_free(psra->lastName);
			mir_free(psra->email);
			mir_free(psra);
			return res;
		}
	}

	INT_PTR res = ProtoCallService(szModule, szService, wParam, lParam);

	if (res == CALLSERVICE_NOTFOUND && pa && pa->bOldProto && pa->ppro && strchr(szService, 'W')) {
		TServiceListItem *item = serviceItems.find((TServiceListItem*)&szService);
		if (!item) return res;

		CCSDATA *ccs = (CCSDATA*)lParam;
		switch (item->id) {
		case 100:
			return (INT_PTR)pa->ppro->SetAwayMsg(wParam, (TCHAR*)lParam);
		case 102:
			return (INT_PTR)pa->ppro->SendFile(ccs->hContact, (TCHAR*)ccs->wParam, (TCHAR**)ccs->lParam);
		case 103:
			return (INT_PTR)pa->ppro->FileAllow(ccs->hContact, (HANDLE)ccs->wParam, (TCHAR*)ccs->lParam);
		case 104:
			return (INT_PTR)pa->ppro->FileDeny(ccs->hContact, (HANDLE)ccs->wParam, (TCHAR*)ccs->lParam);
		case 105:
			{
				PROTOFILERESUME* pfr = (PROTOFILERESUME*)lParam;
				return (INT_PTR)pa->ppro->FileResume((HANDLE)wParam, &pfr->action, &pfr->szFilename);
			}
		case 106:
			return (INT_PTR)pa->ppro->AuthRequest(ccs->hContact, (const TCHAR*)ccs->lParam);
		case 107:
			return (INT_PTR)pa->ppro->AuthDeny((MEVENT)wParam, (const TCHAR*)lParam);
		case 108:
			return (INT_PTR)pa->ppro->SearchBasic((const TCHAR*)lParam);
		case 109:
			{
				PROTOSEARCHBYNAME* psbn = (PROTOSEARCHBYNAME*)lParam;
				return (INT_PTR)pa->ppro->SearchByName(psbn->pszNick, psbn->pszFirstName, psbn->pszLastName);
			}
		case 110:
			return (INT_PTR)pa->ppro->SearchByEmail((const TCHAR*)lParam);
		}
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void InsertServiceListItem(int id, const char* szName)
{
	TServiceListItem* p = (TServiceListItem*)mir_alloc(sizeof(TServiceListItem));
	p->id = id;
	p->name = szName;
	serviceItems.insert(p);
}

int LoadProtocolsModule(void)
{
	bModuleInitialized = TRUE;

	if (LoadProtoChains())
		return 1;

	InsertServiceListItem(1, PS_ADDTOLIST);
	InsertServiceListItem(2, PS_ADDTOLISTBYEVENT);
	InsertServiceListItem(3, PS_AUTHALLOW);
	InsertServiceListItem(4, PS_AUTHDENY);
	InsertServiceListItem(5, PSR_AUTH);
	InsertServiceListItem(6, PSS_AUTHREQUEST);
	InsertServiceListItem(8, PSS_FILEALLOW);
	InsertServiceListItem(9, PSS_FILECANCEL);
	InsertServiceListItem(10, PSS_FILEDENY);
	InsertServiceListItem(11, PS_FILERESUME);
	InsertServiceListItem(12, PS_GETCAPS);
	InsertServiceListItem(13, PS_LOADICON);
	InsertServiceListItem(14, PSS_GETINFO);
	InsertServiceListItem(15, PS_BASICSEARCH);
	InsertServiceListItem(16, PS_SEARCHBYEMAIL);
	InsertServiceListItem(17, PS_SEARCHBYNAME);
	InsertServiceListItem(18, PS_SEARCHBYADVANCED);
	InsertServiceListItem(19, PS_CREATEADVSEARCHUI);
	InsertServiceListItem(20, PSR_CONTACTS);
	InsertServiceListItem(21, PSR_FILE);
	InsertServiceListItem(22, PSR_MESSAGE);
	InsertServiceListItem(23, PSR_URL);
	InsertServiceListItem(24, PSS_CONTACTS);
	InsertServiceListItem(25, PSS_FILE);
	InsertServiceListItem(26, PSS_MESSAGE);
	InsertServiceListItem(27, PSS_URL);
	InsertServiceListItem(28, PSS_SETAPPARENTMODE);
	InsertServiceListItem(29, PS_SETSTATUS);
	InsertServiceListItem(30, PSS_GETAWAYMSG);
	InsertServiceListItem(31, PSR_AWAYMSG);
	InsertServiceListItem(33, PS_SETAWAYMSG);
	InsertServiceListItem(34, PSS_USERISTYPING);
	InsertServiceListItem(35, PS_GETNAME);
	InsertServiceListItem(36, PS_GETSTATUS);

	InsertServiceListItem(100, PS_SETAWAYMSGW);
	InsertServiceListItem(102, PSS_FILEW);
	InsertServiceListItem(103, PSS_FILEALLOWW);
	InsertServiceListItem(104, PSS_FILEDENYW);
	InsertServiceListItem(105, PS_FILERESUMEW);
	InsertServiceListItem(106, PSS_AUTHREQUESTW);
	InsertServiceListItem(107, PS_AUTHDENYW);
	InsertServiceListItem(108, PS_BASICSEARCHW);
	InsertServiceListItem(109, PS_SEARCHBYNAMEW);
	InsertServiceListItem(110, PS_SEARCHBYEMAILW);

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

	for (int i = 0; i < serviceItems.getCount(); i++)
		mir_free(serviceItems[i]);
	serviceItems.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

pfnUninitProto GetProtocolDestructor(char *szProto)
{
	PROTOCOLDESCRIPTOR *p = Proto_IsProtocolLoaded(szProto);
	return (p == NULL) ? NULL : p->fnUninit;
}
