/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-14 Miranda NG project,
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

#include "commonheaders.h"

#include <m_protomod.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_netlib.h>

static HANDLE hAckEvent;

void InitProtocols()
{
	hAckEvent = CreateHookableEvent(ME_PROTO_ACK);
}

void UninitProtocols()
{
	if (hAckEvent) {
		DestroyHookableEvent(hAckEvent);
		hAckEvent = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) ProtoLogA(struct PROTO_INTERFACE *pThis, LPCSTR szFormat, va_list args)
{
	char buf[4096];
	int res = _vsnprintf(buf, sizeof(buf), szFormat, args);
	CallService(MS_NETLIB_LOG, (WPARAM)pThis->m_hNetlibUser, (LPARAM)((res != -1) ? buf : CMStringA().FormatV(szFormat, args)));
}

MIR_CORE_DLL(void) ProtoLogW(struct PROTO_INTERFACE *pThis, LPCWSTR wszFormat, va_list args)
{
	WCHAR buf[4096];
	int res = _vsnwprintf(buf, SIZEOF(buf), wszFormat, args);
	CallService(MS_NETLIB_LOGW, (WPARAM)pThis->m_hNetlibUser, (LPARAM)((res != -1) ? buf : CMStringW().FormatV(wszFormat, args)));
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(INT_PTR) ProtoBroadcastAck(const char *szModule, MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	if (type == ACKTYPE_AVATAR && hProcess) {
		PROTO_AVATAR_INFORMATION* ai = (PROTO_AVATAR_INFORMATION*)hProcess;
		if (ai->cbSize == sizeof(PROTO_AVATAR_INFORMATION)) {
			PROTO_AVATAR_INFORMATIONW aiw = { sizeof(aiw), ai->hContact, ai->format };
			MultiByteToWideChar(CP_ACP, 0, ai->filename, -1, aiw.filename, SIZEOF(aiw.filename));

			hProcess = &aiw;
			ACKDATA ack = { sizeof(ACKDATA), szModule, hContact, type, result, hProcess, lParam };
			return NotifyEventHooks(hAckEvent, 0, (LPARAM)&ack);
		}
	}

	ACKDATA ack = { sizeof(ACKDATA), szModule, hContact, type, result, hProcess, lParam };
	return NotifyEventHooks(hAckEvent, 0, (LPARAM)&ack);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(INT_PTR) ProtoCallService(const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam)
{
	if (szModule == NULL || szService == NULL)
		return false;

	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, szModule, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	return CallService(str, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) ProtoServiceExists(const char *szModule, const char *szService)
{
	if (szModule == NULL || szService == NULL)
		return false;

	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, szModule, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	return ServiceExists(str);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) ProtoConstructor(PROTO_INTERFACE *pThis, LPCSTR pszModuleName, LPCTSTR ptszUserName)
{
	pThis->m_iVersion = 2;
	pThis->m_iStatus = pThis->m_iDesiredStatus = ID_STATUS_OFFLINE;
	pThis->m_szModuleName = mir_strdup(pszModuleName);
	pThis->m_hProtoIcon = (HANDLE)CallService("Skin2/Icons/IsManaged", (WPARAM)LoadSkinnedProtoIcon(pszModuleName, ID_STATUS_ONLINE), 0);
	pThis->m_tszUserName = mir_tstrdup(ptszUserName);
}

MIR_CORE_DLL(void) ProtoDestructor(PROTO_INTERFACE *pThis)
{
	mir_free(pThis->m_szModuleName);
	mir_free(pThis->m_tszUserName);
}

MIR_CORE_DLL(void) ProtoCreateService(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFunc serviceProc)
{
	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, pThis->m_szModuleName, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	::CreateServiceFunctionObj(str, (MIRANDASERVICEOBJ)*(void**)&serviceProc, pThis);
}

MIR_CORE_DLL(void) ProtoCreateServiceParam(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFuncParam serviceProc, LPARAM lParam)
{
	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, pThis->m_szModuleName, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	::CreateServiceFunctionObjParam(str, (MIRANDASERVICEOBJPARAM)*(void**)&serviceProc, pThis, lParam);
}

MIR_CORE_DLL(void) ProtoHookEvent(PROTO_INTERFACE *pThis, const char* szEvent, ProtoEventFunc handler)
{
	::HookEventObj(szEvent, (MIRANDAHOOKOBJ)*(void**)&handler, pThis);
}

MIR_CORE_DLL(HANDLE) ProtoCreateHookableEvent(PROTO_INTERFACE *pThis, const char* szName)
{
	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, pThis->m_szModuleName, _TRUNCATE);
	strncat_s(str, szName, _TRUNCATE);
	return CreateHookableEvent(str);
}

MIR_CORE_DLL(void) ProtoForkThread(PROTO_INTERFACE *pThis, ProtoThreadFunc pFunc, void *param)
{
	UINT threadID;
	CloseHandle((HANDLE)::mir_forkthreadowner((pThreadFuncOwner)*(void**)&pFunc, pThis, param, &threadID));
}

MIR_CORE_DLL(HANDLE) ProtoForkThreadEx(PROTO_INTERFACE *pThis, ProtoThreadFunc pFunc, void *param, UINT* threadID)
{
	UINT lthreadID;
	return (HANDLE)::mir_forkthreadowner((pThreadFuncOwner)*(void**)&pFunc, pThis, param, threadID ? threadID : &lthreadID);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(LPCTSTR) ProtoGetAvatarExtension(int format)
{
	if (format == PA_FORMAT_PNG)
		return _T(".png");
	if (format == PA_FORMAT_JPEG)
		return _T(".jpg");
	if (format == PA_FORMAT_ICON)
		return _T(".ico");
	if (format == PA_FORMAT_BMP)
		return _T(".bmp");
	if (format == PA_FORMAT_GIF)
		return _T(".gif");
	if (format == PA_FORMAT_SWF)
		return _T(".swf");
	if (format == PA_FORMAT_XML)
		return _T(".xml");

	return _T("");
}

MIR_CORE_DLL(int) ProtoGetAvatarFormat(const TCHAR *ptszFileName)
{
	if (ptszFileName == NULL)
		return PA_FORMAT_UNKNOWN;

	const TCHAR *ptszExt = _tcsrchr(ptszFileName, '.');
	if (ptszExt == NULL)
		return PA_FORMAT_UNKNOWN;

	if (!_tcsicmp(ptszExt, _T(".png")))
		return PA_FORMAT_PNG;

	if (!_tcsicmp(ptszExt, _T(".jpg")) || !_tcsicmp(ptszExt, _T(".jpeg")))
		return PA_FORMAT_JPEG;

	if (!_tcsicmp(ptszExt, _T(".ico")))
		return PA_FORMAT_ICON;

	if (!_tcsicmp(ptszExt, _T(".bmp")) || !_tcsicmp(ptszExt, _T(".rle")))
		return PA_FORMAT_BMP;

	if (!_tcsicmp(ptszExt, _T(".gif")))
		return PA_FORMAT_GIF;

	if (!_tcsicmp(ptszExt, _T(".swf")))
		return PA_FORMAT_SWF;

	if (!_tcsicmp(ptszExt, _T(".xml")))
		return PA_FORMAT_XML;

	return PA_FORMAT_UNKNOWN;
}

MIR_CORE_DLL(int) ProtoGetBufferFormat(const void *pBuffer, const TCHAR **ptszExtension)
{
	if (!memcmp(pBuffer, "%PNG", 4)) {
		if (ptszExtension) *ptszExtension = _T(".png");
		return PA_FORMAT_PNG;
	}

	if (!memcmp(pBuffer, "GIF8", 4)) {
		if (ptszExtension) *ptszExtension = _T(".gif");
		return PA_FORMAT_GIF;
	}

	if (!memicmp(pBuffer, "<?xml", 5)) {
		if (ptszExtension) *ptszExtension = _T(".xml");
		return PA_FORMAT_XML;
	}

	if (!memcmp(pBuffer, "\xFF\xD8\xFF\xE0", 4) || !memcmp(pBuffer, "\xFF\xD8\xFF\xE1", 4)) {
		if (ptszExtension) *ptszExtension = _T(".jpg");
		return PA_FORMAT_JPEG;
	}

	if (!memcmp(pBuffer, "BM", 2)) {
		if (ptszExtension) *ptszExtension = _T(".bmp");
		return PA_FORMAT_BMP;
	}

	if (ptszExtension) *ptszExtension = _T("");
	return PA_FORMAT_UNKNOWN;
}

MIR_CORE_DLL(int) ProtoGetAvatarFileFormat(const TCHAR *ptszFileName)
{
	HANDLE hFile = CreateFile(ptszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return PA_FORMAT_UNKNOWN;

	DWORD dwBytes;
	char buf[32];
	BOOL res = ReadFile(hFile, buf, SIZEOF(buf), &dwBytes, NULL);
	CloseHandle(hFile);

	return (res && dwBytes == SIZEOF(buf)) ? ProtoGetBufferFormat(buf) : PA_FORMAT_UNKNOWN;
}
