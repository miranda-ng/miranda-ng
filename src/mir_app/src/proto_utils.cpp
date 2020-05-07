/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team,
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

static int CompareProtos(const MBaseProto *p1, const MBaseProto *p2)
{
	return strcmp(p1->szName, p2->szName);
}

OBJLIST<MBaseProto> g_arProtos(10, CompareProtos);

extern HANDLE hAckEvent;

/////////////////////////////////////////////////////////////////////////////////////////

MBaseProto* Proto_GetProto(const char *szProtoName)
{
	if (szProtoName == nullptr)
		return nullptr;

	return g_arProtos.find((MBaseProto*)&szProtoName);
}

MIR_APP_DLL(PROTOCOLDESCRIPTOR*) Proto_IsProtocolLoaded(const char *szProtoName)
{
	if (szProtoName == nullptr)
		return nullptr;
	
	return g_arProtos.find((MBaseProto*)&szProtoName);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Proto_EnumProtocols(int *nProtos, PROTOCOLDESCRIPTOR ***pProtos)
{
	if (nProtos) *nProtos = g_arProtos.getCount();
	if (pProtos) *pProtos = (PROTOCOLDESCRIPTOR **)g_arProtos.getArray();
}

/////////////////////////////////////////////////////////////////////////////////////////

static void __cdecl sttFakeAckThread(ACKDATA *ack)
{
	Sleep(100);
	NotifyEventHooks(hAckEvent, 0, (LPARAM)ack);
	delete ack;
}

MIR_APP_DLL(void) ProtoBroadcastAsync(const char *szModule, MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	ACKDATA ack = { szModule, hContact, type, result, hProcess, lParam };
	mir_forkThread<ACKDATA>(sttFakeAckThread, new ACKDATA(ack));
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(INT_PTR) ProtoBroadcastAck(const char *szModule, MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	ACKDATA ack = { szModule, hContact, type, result, hProcess, lParam };
	return NotifyEventHooks(hAckEvent, 0, (LPARAM)&ack);
}

/////////////////////////////////////////////////////////////////////////////////////////

void PROTO_INTERFACE::setAllContactStatuses(int iStatus, bool bSkipChats)
{
	for (auto &hContact : AccContacts()) {
		if (isChatRoom(hContact)) {
			if (!bSkipChats && iStatus == ID_STATUS_OFFLINE) {
				ptrW wszRoom(getWStringA(hContact, "ChatRoomID"));
				if (wszRoom != nullptr)
					Chat_Control(m_szModuleName, wszRoom, SESSION_OFFLINE);
			}
		}
		else setWord(hContact, "Status", iStatus);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// protocol menus

static HGENMENU hReqAuth = nullptr, hGrantAuth = nullptr, hRevokeAuth = nullptr;

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

static int __cdecl ProtoPrebuildContactMenu(WPARAM, LPARAM)
{
	Menu_ShowItem(hReqAuth, false);
	Menu_ShowItem(hGrantAuth, false);
	Menu_ShowItem(hRevokeAuth, false);
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

	// "Revoke authorization"
	SET_UID(mi, 0x619efdcb, 0x99c0, 0x44a8, 0xbf, 0x28, 0xc3, 0xe0, 0x2f, 0xb3, 0x7e, 0x77);
	mi.pszService = "Proto/Menu/RevokeAuth";
	mi.name.a = LPGEN("Revoke authorization");
	mi.position = -2000001000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
	hRevokeAuth = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, ProtoPrebuildContactMenu);
}

/////////////////////////////////////////////////////////////////////////////////////////
// protocol constructor & destructor

PROTO_INTERFACE::PROTO_INTERFACE(const char *pszModuleName, const wchar_t *ptszUserName)
{
	m_iVersion = 2;
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	m_szModuleName = mir_strdup(pszModuleName);
	m_tszUserName = mir_wstrdup(ptszUserName);
	db_set_resident(m_szModuleName, "Status");

	m_hmiReqAuth = hReqAuth;
	m_hmiGrantAuth = hGrantAuth;
	m_hmiRevokeAuth = hRevokeAuth;
}

PROTO_INTERFACE::~PROTO_INTERFACE()
{
	if (m_hNetlibUser)
		Netlib_CloseHandle(m_hNetlibUser);

	mir_free(m_szModuleName);
	mir_free(m_tszUserName);

	WindowList_Destroy(m_hWindowList);
}

void PROTO_INTERFACE::OnBuildProtoMenu()
{}

void PROTO_INTERFACE::OnContactDeleted(MCONTACT)
{}

void PROTO_INTERFACE::OnErase()
{}

void PROTO_INTERFACE::OnModulesLoaded()
{}

bool PROTO_INTERFACE::IsReadyToExit()
{
	return true;
}

void PROTO_INTERFACE::OnShutdown()
{}

/////////////////////////////////////////////////////////////////////////////////////////
// protocol services

MIR_APP_DLL(void) ProtoCreateService(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFunc serviceProc)
{
	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, pThis->m_szModuleName, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	::CreateServiceFunctionObj(str, (MIRANDASERVICEOBJ)*(void**)&serviceProc, pThis);
}

MIR_APP_DLL(void) ProtoCreateServiceParam(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFuncParam serviceProc, LPARAM lParam)
{
	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, pThis->m_szModuleName, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	::CreateServiceFunctionObjParam(str, (MIRANDASERVICEOBJPARAM)*(void**)&serviceProc, pThis, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// protocol events

MIR_APP_DLL(void) ProtoHookEvent(PROTO_INTERFACE *pThis, const char* szEvent, ProtoEventFunc handler)
{
	::HookEventObj(szEvent, (MIRANDAHOOKOBJ)*(void**)&handler, pThis);
}

MIR_APP_DLL(HANDLE) ProtoCreateHookableEvent(PROTO_INTERFACE *pThis, const char* szName)
{
	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, pThis->m_szModuleName, _TRUNCATE);
	strncat_s(str, szName, _TRUNCATE);
	return CreateHookableEvent(str);
}

/////////////////////////////////////////////////////////////////////////////////////////
// protocol threads

MIR_APP_DLL(void) ProtoForkThread(PROTO_INTERFACE *pThis, ProtoThreadFunc pFunc, void *param)
{
	UINT threadID;
	CloseHandle((HANDLE)::mir_forkthreadowner((pThreadFuncOwner)*(void**)&pFunc, pThis, param, &threadID));
}

MIR_APP_DLL(HANDLE) ProtoForkThreadEx(PROTO_INTERFACE *pThis, ProtoThreadFunc pFunc, void *param, UINT* threadID)
{
	UINT lthreadID;
	return (HANDLE)::mir_forkthreadowner((pThreadFuncOwner)*(void**)&pFunc, pThis, param, threadID ? threadID : &lthreadID);
}

/////////////////////////////////////////////////////////////////////////////////////////
// protocol windows

void PROTO_INTERFACE::WindowSubscribe(HWND hwnd)
{
	if (m_hWindowList == nullptr)
		m_hWindowList = WindowList_Create();

	WindowList_Add(m_hWindowList, hwnd);
}

void PROTO_INTERFACE::WindowUnsubscribe(HWND hwnd)
{
	WindowList_Remove(m_hWindowList, hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// avatar support

void PROTO_INTERFACE::ReportSelfAvatarChanged()
{
	CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName);
}

MIR_APP_DLL(LPCTSTR) ProtoGetAvatarExtension(int format)
{
	if (format == PA_FORMAT_PNG)
		return L".png";
	if (format == PA_FORMAT_JPEG)
		return L".jpg";
	if (format == PA_FORMAT_ICON)
		return L".ico";
	if (format == PA_FORMAT_BMP)
		return L".bmp";
	if (format == PA_FORMAT_GIF)
		return L".gif";
	if (format == PA_FORMAT_SWF)
		return L".swf";
	if (format == PA_FORMAT_XML)
		return L".xml";

	return L"";
}

MIR_APP_DLL(int) ProtoGetAvatarFormat(const wchar_t *ptszFileName)
{
	if (ptszFileName == nullptr)
		return PA_FORMAT_UNKNOWN;

	const wchar_t *ptszExt = wcsrchr(ptszFileName, '.');
	if (ptszExt == nullptr)
		return PA_FORMAT_UNKNOWN;

	if (!wcsicmp(ptszExt, L".png"))
		return PA_FORMAT_PNG;

	if (!wcsicmp(ptszExt, L".jpg") || !wcsicmp(ptszExt, L".jpeg"))
		return PA_FORMAT_JPEG;

	if (!wcsicmp(ptszExt, L".ico"))
		return PA_FORMAT_ICON;

	if (!wcsicmp(ptszExt, L".bmp") || !wcsicmp(ptszExt, L".rle"))
		return PA_FORMAT_BMP;

	if (!wcsicmp(ptszExt, L".gif"))
		return PA_FORMAT_GIF;

	if (!wcsicmp(ptszExt, L".swf"))
		return PA_FORMAT_SWF;

	if (!wcsicmp(ptszExt, L".xml"))
		return PA_FORMAT_XML;

	return PA_FORMAT_UNKNOWN;
}

MIR_APP_DLL(int) ProtoGetBufferFormat(const void *pBuffer, const wchar_t **ptszExtension)
{
	if (pBuffer != nullptr) {
		if (!memcmp(pBuffer, "\x89PNG", 4)) {
			if (ptszExtension) *ptszExtension = L".png";
			return PA_FORMAT_PNG;
		}

		if (!memcmp(pBuffer, "GIF8", 4)) {
			if (ptszExtension) *ptszExtension = L".gif";
			return PA_FORMAT_GIF;
		}

		if (!memicmp(pBuffer, "<?xml", 5)) {
			if (ptszExtension) *ptszExtension = L".xml";
			return PA_FORMAT_XML;
		}

		if (!memcmp(pBuffer, "\xFF\xD8\xFF\xE0", 4) || !memcmp(pBuffer, "\xFF\xD8\xFF\xE1", 4)) {
			if (ptszExtension) *ptszExtension = L".jpg";
			return PA_FORMAT_JPEG;
		}

		if (!memcmp(pBuffer, "BM", 2)) {
			if (ptszExtension) *ptszExtension = L".bmp";
			return PA_FORMAT_BMP;
		}
	}

	if (ptszExtension) *ptszExtension = L"";
	return PA_FORMAT_UNKNOWN;
}

MIR_APP_DLL(int) ProtoGetAvatarFileFormat(const wchar_t *ptszFileName)
{
	HANDLE hFile = CreateFile(ptszFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return PA_FORMAT_UNKNOWN;

	DWORD dwBytes;
	char buf[32];
	BOOL res = ReadFile(hFile, buf, _countof(buf), &dwBytes, nullptr);
	CloseHandle(hFile);

	return (res && dwBytes == _countof(buf)) ? ProtoGetBufferFormat(buf) : PA_FORMAT_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////////////////
// mime type functions

static char *wszMimeTypes[] =
{
	"application/octet-stream", // PA_FORMAT_UNKNOWN
	"image/png",                // PA_FORMAT_PNG
	"image/jpeg",               // PA_FORMAT_JPEG
	"image/icon",               // PA_FORMAT_ICON
	"image/bmp",                // PA_FORMAT_BMP
	"image/gif",                // PA_FORMAT_GIF
	"image/swf",                // PA_FORMAT_SWF
	"application/xml"           // PA_FORMAT_XML
};

MIR_APP_DLL(const char*) ProtoGetAvatarMimeType(int iFileType)
{
	if (iFileType >= 0 && iFileType < _countof(wszMimeTypes))
		return wszMimeTypes[iFileType];
	return nullptr;
}

MIR_APP_DLL(int) ProtoGetAvatarFormatByMimeType(const char *pwszMimeType)
{
	for (int i = 0; i < _countof(wszMimeTypes); i++)
		if (!mir_strcmp(pwszMimeType, wszMimeTypes[i]))
			return i;
	
	return PA_FORMAT_UNKNOWN;
}

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

int PROTO_INTERFACE::AuthRecv(MCONTACT, PROTORECVEVENT*)
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

HWND PROTO_INTERFACE::SearchAdvanced(HWND)
{
	return nullptr; // error
}

HWND PROTO_INTERFACE::CreateExtendedSearchUI(HWND)
{
	return nullptr; // error
}

int PROTO_INTERFACE::RecvContacts(MCONTACT, PROTORECVEVENT*)
{
	return 1; // error
}

int PROTO_INTERFACE::RecvFile(MCONTACT hContact, PROTORECVFILE *pcre)
{
	CCSDATA ccs = { hContact, PSR_FILE, 0, (LPARAM)pcre };
	return CallService(MS_PROTO_RECVFILET, 0, (LPARAM)&ccs);
}

MEVENT PROTO_INTERFACE::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	if (pre->szMessage == nullptr)
		return 0;

	ptrA pszTemp;
	mir_ptr<BYTE> pszBlob;

	DBEVENTINFO dbei = {};
	dbei.flags = DBEF_UTF;
	dbei.szModule = Proto_GetBaseAccountName(hContact);
	dbei.timestamp = pre->timestamp;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (DWORD)mir_strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;

	if (pre->flags & PREF_CREATEREAD)
		dbei.flags |= DBEF_READ;
	if (pre->flags & PREF_SENT)
		dbei.flags |= DBEF_SENT;

	// if it's possible to find an existing event by its id, do that
	if ((GetCaps(PFLAGNUM_4) & PF4_SERVERMSGID) && pre->szMsgId != nullptr) {
		MEVENT hDbEvent = db_event_getById(m_szModuleName, pre->szMsgId);
		if (hDbEvent == 0 || db_event_edit(hContact, hDbEvent, &dbei)) {
			hDbEvent = db_event_add(hContact, &dbei);
			if (hDbEvent)
				db_event_setId(m_szModuleName, hDbEvent, pre->szMsgId);
		}

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

int PROTO_INTERFACE::SendMsg(MCONTACT, int, const char*)
{
	return 0; // error
}

int PROTO_INTERFACE::SetApparentMode(MCONTACT, int)
{
	return 1; // error
}

int PROTO_INTERFACE::SetStatus(int)
{
	return 1; // you better declare it
}

HANDLE PROTO_INTERFACE::GetAwayMsg(MCONTACT)
{
	return nullptr; // no away message
}

int PROTO_INTERFACE::RecvAwayMsg(MCONTACT, int, PROTORECVEVENT*)
{
	return 1; // error
}

int PROTO_INTERFACE::SetAwayMsg(int, const wchar_t*)
{
	return 1; // error
}

int PROTO_INTERFACE::UserIsTyping(MCONTACT, int)
{
	return 1; // error
}
