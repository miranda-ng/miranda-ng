/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Standard functions

void  CMsnProto::deleteSetting(HANDLE hContact, const char* valueName)
{   DBDeleteContactSetting(hContact, m_szModuleName, valueName);
}

int CMsnProto::getByte(const char* name, BYTE defaultValue)
{	return DBGetContactSettingByte(NULL, m_szModuleName, name, defaultValue);
}

int CMsnProto::getByte(HANDLE hContact, const char* name, BYTE defaultValue)
{	return DBGetContactSettingByte(hContact, m_szModuleName, name, defaultValue);
}

int CMsnProto::getDword(const char* name, DWORD defaultValue)
{	return DBGetContactSettingDword(NULL, m_szModuleName, name, defaultValue);
}

int CMsnProto::getDword(HANDLE hContact, const char* name, DWORD defaultValue)
{	return DBGetContactSettingDword(hContact, m_szModuleName, name, defaultValue);
}

int CMsnProto::getString(const char* name, DBVARIANT* result)
{	return DBGetContactSettingString(NULL, m_szModuleName, name, result);
}

int CMsnProto::getString(HANDLE hContact, const char* name, DBVARIANT* result)
{	return DBGetContactSettingString(hContact, m_szModuleName, name, result);
}

int CMsnProto::getTString(const char* name, DBVARIANT* result)
{	return DBGetContactSettingTString(NULL, m_szModuleName, name, result);
}

int CMsnProto::getTString(HANDLE hContact, const char* name, DBVARIANT* result)
{	return DBGetContactSettingTString(hContact, m_szModuleName, name, result);
}

int CMsnProto::getStringUtf(HANDLE hContact, const char* name, DBVARIANT* result)
{	return DBGetContactSettingStringUtf(hContact, m_szModuleName, name, result);
}

int CMsnProto::getStringUtf(const char* name, DBVARIANT* result)
{	return DBGetContactSettingStringUtf(NULL, m_szModuleName, name, result);
}

int  CMsnProto::getStaticString(HANDLE hContact, const char* valueName, char* dest, unsigned dest_len)
{
	DBVARIANT dbv;
	dbv.pszVal = dest;
	dbv.cchVal = (WORD)dest_len;
	dbv.type = DBVT_ASCIIZ;

	DBCONTACTGETSETTING sVal;
	sVal.pValue = &dbv;
	sVal.szModule = m_szModuleName;
	sVal.szSetting = valueName;
	if (MSN_CallService(MS_DB_CONTACT_GETSETTINGSTATIC, (WPARAM)hContact, (LPARAM)&sVal) != 0)
		return 1;

	return (dbv.type != DBVT_ASCIIZ);
}

WORD CMsnProto::getWord(const char* name, WORD defaultValue)
{	return DBGetContactSettingWord(NULL, m_szModuleName, name, defaultValue);
}

WORD CMsnProto::getWord(HANDLE hContact, const char* name, WORD defaultValue)
{	return DBGetContactSettingWord(hContact, m_szModuleName, name, defaultValue);
}

void CMsnProto::setByte(const char* name, BYTE value)
{	DBWriteContactSettingByte(NULL, m_szModuleName, name, value);
}

void CMsnProto::setByte(HANDLE hContact, const char* name, BYTE value)
{	DBWriteContactSettingByte(hContact, m_szModuleName, name, value);
}

void CMsnProto::setDword(const char* name, DWORD value)
{	DBWriteContactSettingDword(NULL, m_szModuleName, name, value);
}

void CMsnProto::setDword(HANDLE hContact, const char* name, DWORD value)
{	DBWriteContactSettingDword(hContact, m_szModuleName, name, value);
}

void CMsnProto::setString(const char* name, const char* value)
{	DBWriteContactSettingString(NULL, m_szModuleName, name, value);
}

void CMsnProto::setString(HANDLE hContact, const char* name, const char* value)
{	DBWriteContactSettingString(hContact, m_szModuleName, name, value);
}

void CMsnProto::setStringUtf(HANDLE hContact, const char* name, const char* value)
{	DBWriteContactSettingStringUtf(hContact, m_szModuleName, name, value);
}

void CMsnProto::setTString(const char* name, const TCHAR* value)
{	DBWriteContactSettingTString(NULL, m_szModuleName, name, value);
}

void CMsnProto::setTString(HANDLE hContact, const char* name, const TCHAR* value)
{	DBWriteContactSettingTString(hContact, m_szModuleName, name, value);
}

void CMsnProto::setWord(const char* name, WORD value)
{	DBWriteContactSettingWord(NULL, m_szModuleName, name, value);
}

void CMsnProto::setWord(HANDLE hContact, const char* name, WORD value)
{	DBWriteContactSettingWord(hContact, m_szModuleName, name, value);
}
 
/////////////////////////////////////////////////////////////////////////////////////////

void CMsnProto::CreateProtoService(const char* szService, MsnServiceFunc serviceProc)
{
	char str[MAXMODULELABELLENGTH];

	mir_snprintf(str, sizeof(str), "%s%s", m_szModuleName, szService);
	::CreateServiceFunctionObj(str, (MIRANDASERVICEOBJ)*(void**)&serviceProc, this);
}

void CMsnProto::CreateProtoServiceParam(const char* szService, MsnServiceFuncParam serviceProc, LPARAM lParam)
{
	char str[MAXMODULELABELLENGTH];
	mir_snprintf(str, sizeof(str), "%s%s", m_szModuleName, szService);
	::CreateServiceFunctionObjParam(str, (MIRANDASERVICEOBJPARAM)*(void**)&serviceProc, this, lParam);
}

HANDLE CMsnProto::CreateProtoEvent(const char* szService)
{
	char str[MAXMODULELABELLENGTH];
	mir_snprintf(str, sizeof(str), "%s%s", m_szModuleName, szService);
	return ::CreateHookableEvent(str);
}

void CMsnProto::HookProtoEvent(const char* szEvent, MsnEventFunc pFunc)
{  ::HookEventObj(szEvent, (MIRANDAHOOKOBJ)*(void**)&pFunc, this);
}

void CMsnProto::ForkThread(MsnThreadFunc pFunc, void* param)
{
	UINT threadID;
	CloseHandle((HANDLE)mir_forkthreadowner((pThreadFuncOwner)*(void**)&pFunc, this, param, &threadID));
}

int  CMsnProto::SendBroadcast(HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	ACKDATA ack = {0};
	ack.cbSize = sizeof(ACKDATA);
	ack.szModule = m_szModuleName;
	ack.hContact = hContact;
	ack.type = type;
	ack.result = result;
	ack.hProcess = hProcess;
	ack.lParam = lParam;
	return MSN_CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)&ack);
}


#if !defined(_DEBUG)
INT_PTR MSN_CallService(const char* szSvcName, WPARAM wParam, LPARAM lParam)
{
	return CallService(szSvcName, wParam, lParam);
}
#endif

TCHAR*  CMsnProto::GetContactNameT(HANDLE hContact)
{
	if (hContact)
		return (TCHAR*)MSN_CallService(MS_CLIST_GETCONTACTDISPLAYNAME, WPARAM(hContact), GCDNF_TCHAR);
	else
	{
		CONTACTINFO ci = {0};
		ci.cbSize = sizeof(ci);
		ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
		ci.szProto = m_szModuleName;
		if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci))
			return (TCHAR*)ci.pszVal;
		else
			return _T("Me");
	}
}

void MSN_FreeVariant(DBVARIANT* dbv)
{
	DBFreeVariant(dbv);
}

char*  MSN_Translate(const char* str)
{
	return Translate(str);
}

unsigned MSN_GenRandom(void)
{
	unsigned rndnum;
	MSN_CallService(MS_UTILS_GETRANDOM, sizeof(rndnum), (LPARAM)&rndnum);
	return rndnum & 0x7FFFFFFF;
}
