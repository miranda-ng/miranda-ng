/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-13 Miranda NG project,
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

MIR_CORE_DLL(INT_PTR) ProtoBroadcastAck(const char *szModule, HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	if (type == ACKTYPE_AVATAR && hProcess) {
		PROTO_AVATAR_INFORMATION* ai = (PROTO_AVATAR_INFORMATION*)hProcess;
		if (ai->cbSize == sizeof(PROTO_AVATAR_INFORMATION)) {
			PROTO_AVATAR_INFORMATIONW aiw = { sizeof(aiw), ai->hContact, ai->format };
			MultiByteToWideChar(CP_ACP, 0, ai->filename, -1, aiw.filename, SIZEOF(aiw.filename));

			hProcess = &aiw;
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
	strncpy_s(str,szModule,strlen(szModule));
	strncat_s(str,szService,strlen(szService));
	return CallService(str, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) ProtoServiceExists(const char *szModule, const char *szService)
{
	if (szModule == NULL || szService == NULL)
		return false;

	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, szModule, strlen(szModule));
	strncat_s(str, szService, strlen(szService));
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
	strncpy_s(str, pThis->m_szModuleName, strlen(pThis->m_szModuleName));
	strncat_s(str, szService, strlen(szService));
	::CreateServiceFunctionObj(str, (MIRANDASERVICEOBJ)*(void**)&serviceProc, pThis);
}

MIR_CORE_DLL(void) ProtoCreateServiceParam(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFuncParam serviceProc, LPARAM lParam)
{
	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, pThis->m_szModuleName, strlen(pThis->m_szModuleName));
	strncat_s(str, szService, strlen(szService));
	::CreateServiceFunctionObjParam(str, (MIRANDASERVICEOBJPARAM)*(void**)&serviceProc, pThis, lParam);
}

MIR_CORE_DLL(void) ProtoHookEvent(PROTO_INTERFACE *pThis, const char* szEvent, ProtoEventFunc handler)
{
	::HookEventObj(szEvent, (MIRANDAHOOKOBJ)*(void**)&handler, pThis);
}

MIR_CORE_DLL(HANDLE) ProtoCreateHookableEvent(PROTO_INTERFACE *pThis, const char* szName)
{
	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, pThis->m_szModuleName, strlen(pThis->m_szModuleName));
	strncat_s(str, szName, strlen(szName));
	return CreateHookableEvent(str);
}

MIR_CORE_DLL(void) ProtoForkThread(PROTO_INTERFACE *pThis, ProtoThreadFunc pFunc, void *param)
{
	UINT threadID;
	CloseHandle((HANDLE)::mir_forkthreadowner((pThreadFuncOwner) *(void**)&pFunc, pThis, param, &threadID));
}

MIR_CORE_DLL(HANDLE) ProtoForkThreadEx(PROTO_INTERFACE *pThis, ProtoThreadFunc pFunc, void *param, UINT* threadID)
{
	UINT lthreadID;
	return (HANDLE)::mir_forkthreadowner((pThreadFuncOwner) *(void**)&pFunc, pThis, param, threadID ? threadID : &lthreadID);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(bool) ProtoGetBool0(PROTO_INTERFACE *pThis, const char* name, bool defaultValue)
{	return db_get_b(NULL, pThis->m_szModuleName, name, defaultValue) != 0;
}

MIR_CORE_DLL(bool) ProtoGetBool(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, bool defaultValue)
{	return db_get_b(hContact, pThis->m_szModuleName, name, defaultValue) != 0;
}

MIR_CORE_DLL(int) ProtoGetByte0(PROTO_INTERFACE *pThis, const char* name, BYTE defaultValue)
{	return db_get_b(NULL, pThis->m_szModuleName, name, defaultValue);
}

MIR_CORE_DLL(int) ProtoGetByte(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, BYTE defaultValue)
{	return db_get_b(hContact, pThis->m_szModuleName, name, defaultValue);
}

MIR_CORE_DLL(int) ProtoGetDword0(PROTO_INTERFACE *pThis, const char* name, DWORD defaultValue)
{	return db_get_dw(NULL, pThis->m_szModuleName, name, defaultValue);
}

MIR_CORE_DLL(int) ProtoGetDword(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, DWORD defaultValue)
{	return db_get_dw(hContact, pThis->m_szModuleName, name, defaultValue);
}

MIR_CORE_DLL(int) ProtoGetString0(PROTO_INTERFACE *pThis, const char* name, DBVARIANT* result)
{	return db_get_s(NULL, pThis->m_szModuleName, name, result);
}

MIR_CORE_DLL(int) ProtoGetString(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, DBVARIANT* result)
{	return db_get_s(hContact, pThis->m_szModuleName, name, result);
}

MIR_CORE_DLL(int) ProtoGetTString0(PROTO_INTERFACE *pThis, const char* name, DBVARIANT* result)
{	return db_get_ts(NULL, pThis->m_szModuleName, name, result);
}

MIR_CORE_DLL(int) ProtoGetTString(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, DBVARIANT* result)
{	return db_get_ts(hContact, pThis->m_szModuleName, name, result);
}

MIR_CORE_DLL(WORD) ProtoGetWord0(PROTO_INTERFACE *pThis, const char* name, WORD defaultValue)
{	return (WORD)db_get_w(NULL, pThis->m_szModuleName, name, defaultValue);
}

MIR_CORE_DLL(WORD) ProtoGetWord(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, WORD defaultValue)
{	return (WORD)db_get_w(hContact, pThis->m_szModuleName, name, defaultValue);
}

MIR_CORE_DLL(char*) ProtoGetStringA0(PROTO_INTERFACE *pThis, const char* setting)
{	return db_get_sa(NULL, pThis->m_szModuleName, setting);
}

MIR_CORE_DLL(char*) ProtoGetStringA(PROTO_INTERFACE *pThis, HANDLE hContact, const char* setting)
{	return db_get_sa(hContact, pThis->m_szModuleName, setting);
}

MIR_CORE_DLL(void) ProtoSetByte0(PROTO_INTERFACE *pThis, const char* name, BYTE value)
{	db_set_b(NULL, pThis->m_szModuleName, name, value);
}

MIR_CORE_DLL(void) ProtoSetByte(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, BYTE value)
{	db_set_b(hContact, pThis->m_szModuleName, name, value);
}

MIR_CORE_DLL(void) ProtoSetWord0(PROTO_INTERFACE *pThis, const char* name, WORD value)
{	db_set_w(NULL, pThis->m_szModuleName, name, value);
}

MIR_CORE_DLL(void) ProtoSetWord(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, WORD value)
{	db_set_w(hContact, pThis->m_szModuleName, name, value);
}

MIR_CORE_DLL(void) ProtoSetDword0(PROTO_INTERFACE *pThis, const char* name, DWORD value)
{	db_set_dw(NULL, pThis->m_szModuleName, name, value);
}

MIR_CORE_DLL(void) ProtoSetDword(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, DWORD value)
{	db_set_dw(hContact, pThis->m_szModuleName, name, value);
}

MIR_CORE_DLL(void) ProtoSetString0(PROTO_INTERFACE *pThis, const char* name, const char* value)
{	db_set_s(NULL, pThis->m_szModuleName, name, value);
}

MIR_CORE_DLL(void) ProtoSetString(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, const char* value)
{	db_set_s(hContact, pThis->m_szModuleName, name, value);
}

MIR_CORE_DLL(void) ProtoSetTString0(PROTO_INTERFACE *pThis, const char* name, const TCHAR* value)
{	db_set_ts(NULL, pThis->m_szModuleName, name, value);
}

MIR_CORE_DLL(void) ProtoSetTString(PROTO_INTERFACE *pThis, HANDLE hContact, const char* name, const TCHAR* value)
{	db_set_ts(hContact, pThis->m_szModuleName, name, value);
}
