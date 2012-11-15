/*

Jabber Protocol Plugin for Miranda IM
Copyright (C) 2002-04  Santithorn Bunchua
Copyright (C) 2005-12  George Hazan

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

#include "jabber.h"

void CJabberProto::JCreateService(const char* szService, JServiceFunc serviceProc)
{
	char str[ MAXMODULELABELLENGTH ];
	strcpy(str, m_szModuleName);
	strcat(str, szService);
	::CreateServiceFunctionObj(str, (MIRANDASERVICEOBJ)*(void**)&serviceProc, this);
}

void CJabberProto::JCreateServiceParam(const char* szService, JServiceFuncParam serviceProc, LPARAM lParam)
{
	char str[ MAXMODULELABELLENGTH ];
	strcpy(str, m_szModuleName);
	strcat(str, szService);
	::CreateServiceFunctionObjParam(str, (MIRANDASERVICEOBJPARAM)*(void**)&serviceProc, this, lParam);
}

void CJabberProto::JHookEvent(const char* szEvent, JEventFunc handler)
{
	::HookEventObj(szEvent, (MIRANDAHOOKOBJ)*(void**)&handler, this);
}

HANDLE CJabberProto::JCreateHookableEvent(const char* szService)
{
	char str[ MAXMODULELABELLENGTH ];
	strcpy(str, m_szModuleName);
	strcat(str, szService);
	return CreateHookableEvent(str);
}

void CJabberProto::JForkThread(JThreadFunc pFunc, void *param)
{
	UINT threadID;
	CloseHandle((HANDLE)::mir_forkthreadowner((pThreadFuncOwner) *(void**)&pFunc, this, param, &threadID));
}

HANDLE CJabberProto::JForkThreadEx(JThreadFunc pFunc, void *param, UINT* threadID)
{
	UINT lthreadID;
	return (HANDLE)::mir_forkthreadowner((pThreadFuncOwner) *(void**)&pFunc, this, param, threadID ? threadID : &lthreadID);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::JDeleteSetting(HANDLE hContact, const char* valueName)
{
   DBDeleteContactSetting(hContact, m_szModuleName, valueName);
}
/*
DWORD CJabberProto::JGetByte(const char* valueName, int parDefltValue)
{
	return db_get_b(NULL, m_szModuleName, valueName, parDefltValue);
}
*/
DWORD CJabberProto::JGetByte(HANDLE hContact, const char* valueName, int parDefltValue)
{
	return db_get_b(hContact, m_szModuleName, valueName, parDefltValue);
}

char* __stdcall JGetContactName(HANDLE hContact)
{
	return (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, WPARAM(hContact), 0);
}

DWORD CJabberProto::JGetDword(HANDLE hContact, const char* valueName, DWORD parDefltValue)
{
	return db_get_dw(hContact, m_szModuleName, valueName, parDefltValue);
}

int CJabberProto::JGetStaticString(const char* valueName, HANDLE hContact, char* dest, int dest_len)
{
	DBVARIANT dbv;
	dbv.pszVal = dest;
	dbv.cchVal = dest_len;
	dbv.type = DBVT_ASCIIZ;

	DBCONTACTGETSETTING sVal;
	sVal.pValue = &dbv;
	sVal.szModule = m_szModuleName;
	sVal.szSetting = valueName;
	if (CallService(MS_DB_CONTACT_GETSETTINGSTATIC, (WPARAM)hContact, (LPARAM)&sVal) != 0)
		return 1;

	return (dbv.type != DBVT_ASCIIZ);
}

int CJabberProto::JGetStringUtf(HANDLE hContact, char* valueName, DBVARIANT* dbv)
{
	return DBGetContactSettingStringUtf(hContact, m_szModuleName, valueName, dbv);
}

int CJabberProto::JGetStringT(HANDLE hContact, char* valueName, DBVARIANT* dbv)
{
	return DBGetContactSettingTString(hContact, m_szModuleName, valueName, dbv);
}

TCHAR *CJabberProto::JGetStringT(HANDLE hContact, char* valueName)
{
	DBVARIANT dbv = {0};
	if (JGetStringT(hContact, valueName, &dbv))
		return NULL;

	TCHAR *res = mir_tstrdup(dbv.ptszVal);
	db_free(&dbv);
	return res;
}

TCHAR *CJabberProto::JGetStringT(HANDLE hContact, char* valueName, TCHAR *&out)
{
	return out = JGetStringT(hContact, valueName);
}

TCHAR *CJabberProto::JGetStringT(HANDLE hContact, char* valueName, TCHAR *buf, int size)
{
	DBVARIANT dbv = {0};
	if (JGetStringT(hContact, valueName, &dbv))
		return NULL;

	lstrcpyn(buf, dbv.ptszVal, size);
	db_free(&dbv);
	return buf;
}

WORD CJabberProto::JGetWord(HANDLE hContact, const char* valueName, int parDefltValue)
{
	return db_get_w(hContact, m_szModuleName, valueName, parDefltValue);
}

int CJabberProto::JSendBroadcast(HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	// clear saved passowrd on login error. ugly hack, but at least this is centralized
	if (type == ACKTYPE_LOGIN && (lParam == LOGINERR_WRONGPASSWORD || lParam == LOGINERR_BADUSERID))
		*m_savedPassword = 0;

	ACKDATA ack = {0};
	ack.cbSize = sizeof(ACKDATA);
	ack.szModule = m_szModuleName;
	ack.hContact = hContact;
	ack.type = type;
	ack.result = result;
	ack.hProcess = hProcess;
	ack.lParam = lParam;
	return CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)&ack);
}
/*
DWORD CJabberProto::JSetByte(const char* valueName, int parValue)
{
	return db_set_b(NULL, m_szModuleName, valueName, parValue);
}
*/
DWORD CJabberProto::JSetByte(HANDLE hContact, const char* valueName, int parValue)
{
	return db_set_b(hContact, m_szModuleName, valueName, parValue);
}

DWORD CJabberProto::JSetDword(HANDLE hContact, const char* valueName, DWORD parValue)
{
	return db_set_dw(hContact, m_szModuleName, valueName, parValue);
}

DWORD CJabberProto::JSetString(HANDLE hContact, const char* valueName, const char* parValue)
{
	return db_set_s(hContact, m_szModuleName, valueName, parValue);
}

DWORD CJabberProto::JSetStringT(HANDLE hContact, const char* valueName, const TCHAR *parValue)
{
	return db_set_ts(hContact, m_szModuleName, valueName, parValue);
}

DWORD CJabberProto::JSetStringUtf(HANDLE hContact, const char* valueName, const char* parValue)
{
	return db_set_utf(hContact, m_szModuleName, valueName, parValue);
}

DWORD CJabberProto::JSetWord(HANDLE hContact, const char* valueName, int parValue)
{
	return db_set_w(hContact, m_szModuleName, valueName, parValue);
}

char* __fastcall JTranslate(const char* str)
{
	return Translate(str);
}

// save/load crypted strings
void __forceinline sttCryptString(char *str)
{
	for (;*str; ++str) 
	{
		const char c = *str ^ 0xc3;
		if (c) *str = c;
	}
}

TCHAR* CJabberProto::JGetStringCrypt(HANDLE hContact, char *valueName)
{
	DBVARIANT dbv;

	if (DBGetContactSettingString(hContact, m_szModuleName, valueName, &dbv))
		return NULL;

	sttCryptString(dbv.pszVal);
	WCHAR *res = mir_utf8decodeW(dbv.pszVal);


	DBFreeVariant(&dbv);
	return res;
}

DWORD CJabberProto::JSetStringCrypt(HANDLE hContact, char *valueName, const TCHAR *parValue)
{
	char *tmp = mir_utf8encodeT(parValue);
	sttCryptString(tmp);
	DWORD res = JSetString(hContact, valueName, tmp);
	mir_free(tmp);
	return res;
}
