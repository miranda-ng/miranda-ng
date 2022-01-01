/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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

MIR_APP_DLL(PROTO_INTERFACE *) Proto_GetInstance(MCONTACT hContact)
{
	PROTOACCOUNT *pa = Proto_GetContactAccount(hContact);
	if (pa == nullptr || pa->bOldProto)
		return nullptr;

	PROTO_INTERFACE *ppi = pa->ppro;
	return (ppi != nullptr && ppi->m_iVersion > 1) ? ppi : nullptr;
}

MIR_APP_DLL(PROTO_INTERFACE *) Proto_GetInstance(const char *szModule)
{
	PROTOACCOUNT *pa = Proto_GetAccount(szModule);
	if (pa == nullptr || pa->bOldProto)
		return nullptr;

	PROTO_INTERFACE *ppi = pa->ppro;
	return (ppi != nullptr && ppi->m_iVersion > 1) ? ppi : nullptr;
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
	if (format == PA_FORMAT_WEBP)
		return L".webp";

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

	if (!wcsicmp(ptszExt, L".webp"))
		return PA_FORMAT_WEBP;

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

		if (!memcmp(pBuffer, "RIFF", 4) && !memcmp((char*)pBuffer+8, "WEBP", 4)) {
			if (ptszExtension) *ptszExtension = L".webp";
			return PA_FORMAT_WEBP;
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
	"application/xml",          // PA_FORMAT_XML
	"image/webp",               // PA_FORMAT_WEBP
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
