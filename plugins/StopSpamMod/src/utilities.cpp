/* Copyright (C) Miklashevsky Roman, sss, elzor
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

wstring DBGetContactSettingStringPAN(MCONTACT hContact, char const * szModule, char const * szSetting, wstring errorValue)
{
	DBVARIANT dbv;
		if (db_get_ws(hContact, szModule, szSetting, &dbv))
		return errorValue;
	
	errorValue = dbv.pwszVal;
	db_free(&dbv);
	return errorValue;
}

std::string DBGetContactSettingStringPAN_A(MCONTACT hContact, char const * szModule, char const * szSetting, std::string errorValue)
{
	DBVARIANT dbv;
	if (db_get_s(hContact, szModule, szSetting, &dbv))
		return errorValue;

	errorValue = dbv.pszVal;
	db_free(&dbv);
	return errorValue;
}

wstring& GetDlgItemString(HWND hwnd, int id)
{
	HWND h = GetDlgItem(hwnd, id);
	int len = GetWindowTextLength(h);
	wchar_t * buf = new wchar_t[len + 1];
	GetWindowText(h, buf, len + 1);
	static wstring s;
	s = buf;
	delete[]buf;
	return s;
}

std::string &GetProtoList()
{
	static std::string s;
	return s = DBGetContactSettingStringPAN_A(0, MODULENAME, "protoList", "ICQ\r\n");
}

bool ProtoInList(const char *szProto)
{
	if (szProto == nullptr)
		return false;

	return std::string::npos != GetProtoList().find(std::string(szProto) + "\r\n");
}

void DeleteCListGroupsByName(wchar_t* szGroupName)
{
	uint8_t ConfirmDelete = Clist::ConfirmDelete;
	if (ConfirmDelete)
		db_set_b(0, "CList", "ConfirmDelete", 0);

	wchar_t *szGroup;
	for (int i = 1; (szGroup = Clist_GroupGetName(i, nullptr)) != nullptr; i++)
		if (!mir_wstrcmp(szGroupName, szGroup))
			Clist_GroupDelete(i);

	if (ConfirmDelete)
		db_set_b(0, "CList", "ConfirmDelete", ConfirmDelete);
}

int RemoveTmp(WPARAM, LPARAM)
{
	CleanThread(nullptr);
	return 0;
}

wstring variables_parse(wstring const &tstrFormat, MCONTACT hContact)
{
	if (gbVarsServiceExist) {
		FORMATINFO fi = {};
		fi.cbSize = sizeof(fi);
		fi.szFormat.w = wcsdup(tstrFormat.c_str());
		fi.hContact = hContact;
		fi.flags = FIF_UNICODE;
		wchar_t *tszParsed = (wchar_t*)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
		free(fi.szFormat.w);

		if (tszParsed) {
			wstring tstrResult = tszParsed;
			mir_free(tszParsed);
			return tstrResult;
		}
	}
	return tstrFormat;
}

// case-insensitive mir_wstrcmp
const int Stricmp(const wchar_t *str, const wchar_t *substr)
{
	int i = 0;

	wchar_t *str_up = mir_wstrdup(str);
	wchar_t *substr_up = mir_wstrdup(substr);

	CharUpperBuff(str_up, (int)mir_wstrlen(str_up));
	CharUpperBuff(substr_up, (int)mir_wstrlen(substr_up));

	i = mir_wstrcmp(str_up, substr_up);

	mir_free(str_up);
	mir_free(substr_up);

	return i;
}

wchar_t* ReqGetText(DBEVENTINFO* dbei)
{
	if (!dbei->pBlob)
		return nullptr;

	char * ptr = (char *)&dbei->pBlob[sizeof(uint32_t) * 2];
	int len = dbei->cbBlob - sizeof(uint32_t) * 2;
	int i = 0;

	while (len && (i < 4)) {
		if (!ptr[0]) i++;
		ptr++;
		len--;
	};

	if (len) {
		char * tstr = (char *)mir_alloc(len + 1);
		memcpy(tstr, ptr, len);
		tstr[len] = 0;
		wchar_t* msg = nullptr;
		msg = (dbei->flags&DBEF_UTF) ? mir_utf8decodeW(tstr) : mir_a2u(tstr);
		mir_free(tstr);
		return (wchar_t *)msg;
	};
	return nullptr;
}

BOOL IsUrlContains(wchar_t * Str)
{
	const int CountUrl = 11;
	const wchar_t  URL[CountUrl][5] =
	{
		L"http",
		L"www",
		L".ru",
		L".com",
		L".de",
		L".cz",
		L".org",
		L".net",
		L".su",
		L".ua",
		L".tv"
	};

	if (Str && mir_wstrlen(Str) > 0) {
		wchar_t *StrLower = mir_wstrdup(Str);
		CharLowerBuff(StrLower, (int)mir_wstrlen(StrLower));
		for (int i = 0; i < CountUrl; i++)
			if (wcsstr(StrLower, URL[i])) {
				mir_free(StrLower);
				return 1;
			}
		mir_free(StrLower);
	}
	return 0;
}

wstring GetContactUid(MCONTACT hContact, wstring Protocol)
{
	char *szProto = mir_utf8encodeW(Protocol.c_str());
	ptrW uid(Contact::GetInfo(CNF_DISPLAYUID, hContact, szProto));
	return (uid) ? uid : L"";
}

void LogSpamToFile(MCONTACT hContact, wstring message)
{
	if (!gbLogToFile) return;

	wstring LogStrW, LogTime, LogProtocol, LogContactId, LogContactName;
	std::fstream file;
	wchar_t pszName[MAX_PATH];

	if (hStopSpamLogDirH)
		FoldersGetCustomPathW(hStopSpamLogDirH, pszName, MAX_PATH, L"");
	else
		mir_wstrncpy(pszName, VARSW(L"%miranda_logpath%"), _countof(pszName));

	wstring filename = pszName;
	filename += L"\\stopspam_mod.log";
	file.open(filename.c_str(), std::ios::out | std::ios::app);

	// Time Log line
	time_t time_now;
	time(&time_now);

	struct tm *TimeNow = localtime(&time_now);
	LogTime = _wasctime(TimeNow);
	// Time Log line

	// Name, UID and Protocol Log line
	LogProtocol = DBGetContactSettingStringPAN(hContact, "Protocol", "p", L"");
	LogContactName = (wchar_t*)Clist_GetContactDisplayName(hContact);
	LogContactId = (LogProtocol == L"") ? L"" : GetContactUid(hContact, LogProtocol);
	// Name, UID  and Protocol Log line

	LogStrW = L"[" + LogTime.substr(0, LogTime.length() - 1) + L"] " +
		LogContactId + L" - " +
		LogContactName + L" (" +
		LogProtocol + L"): " +
		message + L"\n";

	char *buf = mir_u2a(LogStrW.c_str());
	file.write(buf, LogStrW.length());
	mir_free(buf);

	file.close();
}

mir_cs clean_mutex;

void __cdecl CleanProtocolTmpThread(void *param)
{
	const char *szProto = (const char*)param;

	while (true) {
		int status = Proto_GetStatus(szProto);
		if (status > ID_STATUS_OFFLINE)
			break;
		Sleep(2000);
	}

	std::list<MCONTACT> contacts;
	for (auto &hContact : Contacts(szProto))
		if (!Contact::OnList(hContact) || (L"Not In List" == DBGetContactSettingStringPAN(hContact, "CList", "Group", L"")))
			contacts.push_back(hContact);

	Sleep(5000);
	{
		mir_cslock lck(clean_mutex);

		std::list<MCONTACT>::iterator end = contacts.end();
		for (std::list<MCONTACT>::iterator i = contacts.begin(); i != end; ++i) {
			LogSpamToFile(*i, L"Deleted");
			HistoryLogFunc(*i, "Deleted");
			db_delete_contact(*i);
		}
	}
	mir_free(param);
}

void __cdecl CleanProtocolExclThread(void *param)
{
	const char *szProto = (const char*)param;

	while (true) {
		int status = Proto_GetStatus(szProto);
		if (status > ID_STATUS_OFFLINE)
			break;
		Sleep(2000);
	}

	std::list<MCONTACT> contacts;
	for (auto &hContact : Contacts(szProto))
		if (!Contact::OnList(hContact) && g_plugin.getByte(hContact, "Excluded"))
			contacts.push_back(hContact);

	Sleep(5000);
	{
		mir_cslock lck(clean_mutex);
		std::list<MCONTACT>::iterator end = contacts.end();
		for (std::list<MCONTACT>::iterator i = contacts.begin(); i != end; ++i) {
			LogSpamToFile(*i, L"Deleted");
			HistoryLogFunc(*i, "Deleted");
			db_delete_contact(*i);
		}
	}
	mir_free(param);
}

void __cdecl CleanThread(void*)
{
	std::list<std::string> protocols;
	for (auto &pa : Accounts())
		if (!strstr(pa->szModuleName, "MetaContacts") && !strstr(pa->szModuleName, "Weather")) //not real protocols
			protocols.push_back(pa->szModuleName);

	std::list<std::string>::iterator end = protocols.end();
	for (std::list<std::string>::iterator i = protocols.begin(); i != end; ++i) {
		if (gbDelAllTempory)
			mir_forkthread(CleanProtocolTmpThread, mir_strdup((*i).c_str()));
		if (gbDelExcluded)
			mir_forkthread(CleanProtocolExclThread, mir_strdup((*i).c_str()));
	}
}

void HistoryLog(MCONTACT hContact, char *data, int event_type, int flags)
{
	DBEVENTINFO Event = {};
	Event.szModule = MODULENAME;
	Event.eventType = event_type;
	Event.flags = flags | DBEF_UTF;
	Event.timestamp = (uint32_t)time(0);
	Event.cbBlob = (uint32_t)mir_strlen(data) + 1;
	Event.pBlob = (uint8_t*)_strdup(data);
	db_event_add(hContact, &Event);
}

void HistoryLogFunc(MCONTACT hContact, std::string message)
{
	if (gbHistoryLog) {
		if (hContact == INVALID_CONTACT_ID)
			return;

		std::string msg = message;
		msg.append("\n");
		msg.append("Protocol: ").append(Proto_GetBaseAccountName(hContact)).append(" Contact: ");
		msg.append(toUTF8(Clist_GetContactDisplayName(hContact))).append(" ID: ");
		msg.append(toUTF8(GetContactUid(hContact, toUTF16(Proto_GetBaseAccountName(hContact)))));
		HistoryLog(NULL, (char*)msg.c_str(), EVENTTYPE_MESSAGE, DBEF_READ);
	}
}

std::wstring toUTF16(std::string str) //convert as much as possible
{
	return std::wstring(ptrW(mir_utf8decodeW(str.c_str())));
}

std::string toUTF8(std::wstring str)
{
	return std::string(ptrA(mir_utf8encodeW(str.c_str())));
}

std::string toUTF8(std::string str)
{
	return toUTF8(toUTF16(str));
}

std::string get_random_num(int length)
{
	std::string chars("123456789");
	std::string data;
	boost::random_device rng;
	boost::variate_generator<boost::random_device&, boost::uniform_int<>> gen(rng, boost::uniform_int<>(0, (int)chars.length() - 1));
	for (int i = 0; i < length; ++i)
		data += chars[gen()];
	return data;
}
