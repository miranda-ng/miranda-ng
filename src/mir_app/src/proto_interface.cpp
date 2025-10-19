/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team,
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
#include "file.h"

static wchar_t wszPreviewFolder[MAX_PATH];
static HANDLE hPreviewFolder, hHistoryLoaded;

static HGENMENU hReqAuth = nullptr, hGrantAuth = nullptr, hRevokeAuth = nullptr, hServerHist = nullptr;

MIR_APP_DLL(void) History::FinishLoad(MCONTACT hContact)
{
	NotifyEventHooks(hHistoryLoaded, hContact, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// protocol constructor & destructor

PROTO_INTERFACE::PROTO_INTERFACE(const char *pszModuleName, const wchar_t *ptszUserName) :
	m_iVersion(2),
	m_bCacheInited(false)
{
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	m_szModuleName = mir_strdup(pszModuleName);
	m_tszUserName = mir_wstrdup(ptszUserName);
	db_set_resident(m_szModuleName, "Status");
}

PROTO_INTERFACE::~PROTO_INTERFACE()
{
	if (m_hNetlibUser)
		Netlib_CloseHandle(m_hNetlibUser);

	mir_free(m_szModuleName);
	mir_free(m_tszUserName);

	WindowList_Destroy(m_hWindowList);
}

HGENMENU PROTO_INTERFACE::GetMenuItem(ProtoMenuItemType aType)
{
	switch (aType) {
	case PROTO_MENU_REQ_AUTH: return hReqAuth;
	case PROTO_MENU_GRANT_AUTH: return hGrantAuth;
	case PROTO_MENU_REVOKE_AUTH: return hRevokeAuth;
	case PROTO_MENU_LOAD_HISTORY: return hServerHist;
	}

	return nullptr;
}

CMStringW PROTO_INTERFACE::GetAvatarPath() const
{
	return CMStringW(FORMAT, L"%s\\%S", VARSW(L"%miranda_avatarcache%").get(), m_szModuleName);
}

CMStringW PROTO_INTERFACE::GetPreviewPath() const
{
	return CMStringW(FORMAT, L"%s\\%S", wszPreviewFolder, m_szModuleName);
}

void PROTO_INTERFACE::OnBuildProtoMenu()
{}

void PROTO_INTERFACE::OnCacheInit()
{}

void PROTO_INTERFACE::OnContactAdded(MCONTACT)
{}

bool PROTO_INTERFACE::OnContactDeleted(MCONTACT, uint32_t)
{
	return true;
}

MWindow PROTO_INTERFACE::OnCreateAccMgrUI(MWindow)
{
	return nullptr; // error
}

void PROTO_INTERFACE::OnEventDeleted(MCONTACT, MEVENT, int)
{}

void PROTO_INTERFACE::OnEventEdited(MCONTACT, MEVENT, const DBEVENTINFO &)
{}

void PROTO_INTERFACE::OnErase()
{}

void PROTO_INTERFACE::OnMarkRead(MCONTACT, MEVENT)
{}

void PROTO_INTERFACE::OnModulesLoaded()
{}

bool PROTO_INTERFACE::IsReadyToExit()
{
	return true;
}

void PROTO_INTERFACE::OnReceiveOfflineFile(DB::EventInfo&, DB::FILE_BLOB &)
{}

void PROTO_INTERFACE::OnSendOfflineFile(DB::EventInfo &, DB::FILE_BLOB &, void *)
{}

void PROTO_INTERFACE::OnShutdown()
{}

/////////////////////////////////////////////////////////////////////////////////////////
// default PROTO_INTERFACE method implementations

MCONTACT PROTO_INTERFACE::AddToList(int, PROTOSEARCHRESULT*)
{
	return 0; // error
}

MCONTACT PROTO_INTERFACE::AddToListByEvent(int, int, MEVENT)
{
	return 0; // error
}

int PROTO_INTERFACE::Authorize(MEVENT)
{
	return 1; // error
}

int PROTO_INTERFACE::AuthDeny(MEVENT, const wchar_t*)
{
	return 1; // error
}

int PROTO_INTERFACE::AuthRecv(MCONTACT, DB::EventInfo&)
{
	return 1; // error
}

int PROTO_INTERFACE::AuthRequest(MCONTACT, const wchar_t*)
{
	return 1; // error
}

HANDLE PROTO_INTERFACE::FileAllow(MCONTACT, HANDLE, const wchar_t*)
{
	return nullptr; // error
}

int PROTO_INTERFACE::FileCancel(MCONTACT, HANDLE)
{
	return 1; // error
}

int PROTO_INTERFACE::FileDeny(MCONTACT, HANDLE, const wchar_t*)
{
	return 1; // error
}

int PROTO_INTERFACE::FileResume(HANDLE, int, const wchar_t*)
{
	return 1; // error
}

INT_PTR PROTO_INTERFACE::GetCaps(int, MCONTACT)
{
	return 0; // empty value
}

int PROTO_INTERFACE::GetInfo(MCONTACT, int)
{
	return 1; // error
}

HANDLE PROTO_INTERFACE::SearchBasic(const wchar_t*)
{
	return nullptr; // error
}

HANDLE PROTO_INTERFACE::SearchByEmail(const wchar_t*)
{
	return nullptr; // error
}

HANDLE PROTO_INTERFACE::SearchByName(const wchar_t*, const wchar_t*, const wchar_t*)
{
	return nullptr; // error
}

HANDLE PROTO_INTERFACE::SearchAdvanced(MWindow)
{
	return nullptr; // error
}

MWindow PROTO_INTERFACE::CreateExtendedSearchUI(MWindow)
{
	return nullptr; // error
}

int PROTO_INTERFACE::RecvContacts(MCONTACT, DB::EventInfo &)
{
	return 1; // error
}

MEVENT PROTO_INTERFACE::RecvFile(MCONTACT hContact, DB::FILE_BLOB &blob, DB::EventInfo &dbei)
{
	return Proto_RecvFile(hContact, blob, dbei);
}

MEVENT PROTO_INTERFACE::RecvMsg(MCONTACT hContact, DB::EventInfo &dbei)
{
	dbei.flags |= DBEF_UTF;
	dbei.szModule = Proto_GetBaseAccountName(hContact);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (uint32_t)mir_strlen(dbei.pBlob) + 1;

	// if it's possible to find an existing event by its id, do that
	if ((GetCaps(PFLAGNUM_4) & PF4_SERVERMSGID) && dbei.szId != nullptr) {
		MEVENT hDbEvent = db_event_getById(m_szModuleName, dbei.szId);
		if (hDbEvent == 0 || db_event_edit(hDbEvent, &dbei))
			hDbEvent = db_event_add(hContact, &dbei);

		return hDbEvent;
	}

	// event is new? add it
	return (INT_PTR)db_event_add(hContact, &dbei);
}

int PROTO_INTERFACE::SendContacts(MCONTACT, int, int, MCONTACT*)
{
	return 1; // error
}

HANDLE PROTO_INTERFACE::SendFile(MCONTACT, const wchar_t*, wchar_t**)
{
	return nullptr; // error
}

int PROTO_INTERFACE::SendMsg(MCONTACT, MEVENT, const char*)
{
	return 0; // error
}

int PROTO_INTERFACE::GetStatus()
{
	return m_iStatus;
}

int PROTO_INTERFACE::SetStatus(int)
{
	return 1; // you better declare it
}

HANDLE PROTO_INTERFACE::GetAwayMsg(MCONTACT)
{
	return nullptr; // no away message
}

int PROTO_INTERFACE::SetAwayMsg(int, const wchar_t*)
{
	return 1; // error
}

int PROTO_INTERFACE::UserIsTyping(MCONTACT, int)
{
	return 1; // error
}

/////////////////////////////////////////////////////////////////////////////////////////
// folders support

static int FoldersPathChanged(WPARAM, LPARAM)
{
	FoldersGetCustomPathW(hPreviewFolder, wszPreviewFolder, _countof(wszPreviewFolder), L"");
	return 0;
}

static int ProtoModulesLoaded(WPARAM, LPARAM)
{
	wcsncpy_s(wszPreviewFolder, VARSW(L"%miranda_userdata%\\Preview"), _TRUNCATE);
	if (hPreviewFolder = FoldersRegisterCustomPathW("Preview", LPGEN("Preview folder"), wszPreviewFolder)) {
		HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersPathChanged);
		FoldersPathChanged(0, 0);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// protocol menus

static INT_PTR __cdecl stubRequestAuth(WPARAM hContact, LPARAM)
{
	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto)
		ProtoCallService(szProto, PS_MENU_REQAUTH, hContact, 0);
	return 0;
}

static INT_PTR __cdecl stubGrantAuth(WPARAM hContact, LPARAM)
{
	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto)
		ProtoCallService(szProto, PS_MENU_GRANTAUTH, hContact, 0);
	return 0;
}

static INT_PTR __cdecl stubRevokeAuth(WPARAM hContact, LPARAM)
{
	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto)
		ProtoCallService(szProto, PS_MENU_REVOKEAUTH, hContact, 0);
	return 0;
}

static INT_PTR __cdecl stubLoadHistory(WPARAM hContact, LPARAM)
{
	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto)
		ProtoCallService(szProto, PS_MENU_LOADHISTORY, hContact, 0);

	return 0;
}

static int OnHistoryLoaded(WPARAM hContact, LPARAM)
{
	POPUPDATA2 pd = {};
	pd.cbSize = sizeof(pd);
	pd.flags = PU2_UNICODE;
	pd.lchContact = hContact;
	pd.szTitle.w = Clist_GetContactDisplayName(hContact);
	pd.szText.w = TranslateT("History loaded successfully");
	Popup_Add(&pd);
	return 0;
}

static int __cdecl ProtoPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(hReqAuth, false);
	Menu_ShowItem(hGrantAuth, false);
	Menu_ShowItem(hRevokeAuth, false);
	
	const char *szProto = Proto_GetBaseAccountName(hContact);
	Menu_ShowItem(hServerHist, ProtoServiceExists(szProto, PS_MENU_LOADHISTORY));
	return 0;
}

void InitProtoMenus(void)
{
	// "Request authorization"
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x36375a1f, 0xc142, 0x4d6e, 0xa6, 0x57, 0xe4, 0x76, 0x5d, 0xbc, 0x59, 0x8e);
	mi.pszService = "Proto/Menu/ReqAuth";
	mi.name.a = LPGEN("Request authorization");
	mi.position = -2000001002;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	hReqAuth = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, stubRequestAuth);

	// "Grant authorization"
	SET_UID(mi, 0x4c90452a, 0x869a, 0x4a81, 0xaf, 0xa8, 0x28, 0x34, 0xaf, 0x2b, 0x6b, 0x30);
	mi.pszService = "Proto/Menu/GrantAuth";
	mi.name.a = LPGEN("Grant authorization");
	mi.position = -2000001001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_GRANT);
	hGrantAuth = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, stubGrantAuth);

	// "Revoke authorization"
	SET_UID(mi, 0x619efdcb, 0x99c0, 0x44a8, 0xbf, 0x28, 0xc3, 0xe0, 0x2f, 0xb3, 0x7e, 0x77);
	mi.pszService = "Proto/Menu/RevokeAuth";
	mi.name.a = LPGEN("Revoke authorization");
	mi.position = -2000001000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
	hRevokeAuth = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, stubRevokeAuth);

	SET_UID(mi, 0xd15b841d, 0xb0fc, 0x4ab5, 0x96, 0x94, 0xcf, 0x6c, 0x6e, 0x99, 0x4b, 0x3c); // {D15B841D-B0FC-4AB5-9694-CF6C6E994B3C}
	mi.pszService = "Proto/Menu/LoadHistory";
	mi.name.a = LPGEN("Load server history");
	mi.position = -200001004;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
	hServerHist = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, stubLoadHistory);

	hHistoryLoaded = CreateHookableEvent(ME_HISTORY_LOADED);

	HookEvent(ME_HISTORY_LOADED, OnHistoryLoaded);
	HookEvent(ME_SYSTEM_MODULESLOADED, ProtoModulesLoaded);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, ProtoPrebuildContactMenu);
}
