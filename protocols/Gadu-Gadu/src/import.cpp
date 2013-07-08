////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2006 Adam Strzelecki <ono+miranda@java.pl>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"

////////////////////////////////////////////////////////////////////////////////
// Checks if a group already exists in Miranda with
// the specified name.
// Returns 1 if a group with the name exists, returns 0 otherwise.
int GroupNameExists(const char *name)
{
	char idstr[33];
	DBVARIANT dbv;
	int i;

	for (i = 0; ; i++) {
		_itoa(i, idstr, 10);
		if (db_get_s(NULL, "CListGroups", idstr, &dbv, DBVT_ASCIIZ)) break;
		if (!strcmp(dbv.pszVal + 1, name)) {
			db_free(&dbv);
			return 1;
		}
		db_free(&dbv);
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Creates a group with a specified name in the
// Miranda contact list.
// Returns proper group name

char *CreateGroup(char *groupName)
{
	int groupId;
	char groupIdStr[11];
	char groupName2[127];
	char *p;
	DBVARIANT dbv;

	// Cleanup group name from weird characters

	// Skip first break
	while(*groupName && *groupName == '\\') groupName++;

	p = strrchr(groupName, '\\');
	// Cleanup end
	while(p && !(*(p + 1)))
	{
		*p = 0;
		p = strrchr(groupName, '\\');
	}
	// Create upper groups
	if (p)
	{
		*p = 0;
		CreateGroup(groupName);
		*p = '\\';
	}

	// Is this a duplicate?
	if (!GroupNameExists(groupName))
	{
		lstrcpynA(groupName2 + 1, groupName, (int)strlen(groupName) + 1);

		// Find an unused id
		for (groupId = 0; ; groupId++) {
				_itoa(groupId, groupIdStr,10);
				if (db_get_s(NULL, "CListGroups", groupIdStr, &dbv, DBVT_ASCIIZ))
						break;
				db_free(&dbv);
		}

		groupName2[0] = 1|GROUPF_EXPANDED;	// 1 is required so we never get '\0'
		db_set_s(NULL, "CListGroups", groupIdStr, groupName2);
	}
	return groupName;
}

char *gg_makecontacts(GGPROTO *gg, int cr)
{
	string_t s = string_init(NULL);
	char *contacts;

	// Readup contacts
	for (HANDLE hContact = db_find_first(gg->m_szModuleName); hContact; hContact = db_find_next(hContact, gg->m_szModuleName)) {
		if (db_get_b(hContact, gg->m_szModuleName, "ChatRoom", 0))
			continue;

		// Readup FirstName
		DBVARIANT dbv;
		if (!db_get_s(hContact, gg->m_szModuleName, GG_KEY_PD_FIRSTNAME, &dbv, DBVT_WCHAR))
		{
			char* pszValA = mir_t2a(dbv.ptszVal);
			string_append(s, dbv.pszVal);
			mir_free(pszValA);
			db_free(&dbv);
		}
		string_append_c(s, ';');
		// Readup LastName
		if (!db_get_s(hContact, gg->m_szModuleName, GG_KEY_PD_LASTNAME, &dbv, DBVT_WCHAR))
		{
			char* pszValA = mir_t2a(dbv.ptszVal);
			string_append(s, dbv.pszVal);
			mir_free(pszValA);
			db_free(&dbv);
		}
		string_append_c(s, ';');

		// Readup Nick
		if (!db_get_s(hContact, "CList", "MyHandle", &dbv, DBVT_TCHAR) || !db_get_s(hContact, gg->m_szModuleName, GG_KEY_NICK, &dbv, DBVT_TCHAR))
		{
			char* dbvA = mir_t2a(dbv.ptszVal);
			DBVARIANT dbv2;
			if (!db_get_s(hContact, gg->m_szModuleName, GG_KEY_PD_NICKNAME, &dbv2, DBVT_WCHAR))
			{
				char* pszValA = mir_t2a(dbv2.ptszVal);
				string_append(s, pszValA);
				mir_free(pszValA);
				db_free(&dbv2);
			}
			else string_append(s, dbvA);

			string_append_c(s, ';');
			string_append(s, dbvA);
			mir_free(dbvA);
			db_free(&dbv);
		}
		else
			string_append_c(s, ';');
		string_append_c(s, ';');

		// Readup Phone (fixed: uses stored editable phones)
		if (!db_get_s(hContact, "UserInfo", "MyPhone0", &dbv, DBVT_ASCIIZ))
		{
			// Remove SMS postfix
			char *sms = strstr(dbv.pszVal, " SMS");
			if (sms) *sms = 0;

			string_append(s, dbv.pszVal);
			db_free(&dbv);
		}
		string_append_c(s, ';');
		// Readup Group
		if (!db_get_s(hContact, "CList", "Group", &dbv, DBVT_ASCIIZ))
		{
			string_append(s, dbv.pszVal);
			db_free(&dbv);
		}
		string_append_c(s, ';');
		// Readup Uin
		string_append(s, ditoa(db_get_dw(hContact, gg->m_szModuleName, GG_KEY_UIN, 0)));
		string_append_c(s, ';');
		// Readup Mail (fixed: uses stored editable mails)
		if (!db_get_s(hContact, "UserInfo", "Mye-mail0", &dbv, DBVT_ASCIIZ))
		{
			string_append(s, dbv.pszVal);
			db_free(&dbv);
		}
		if (cr)
			string_append(s, ";0;;0;\r\n");
		else
			string_append(s, ";0;;0;\n");
	}

	contacts = string_free(s, 0);

#ifdef DEBUGMODE
	gg->netlog("gg_makecontacts(): \n%s", contacts);
#endif

	return contacts;
}

char *strndup(char *str, int c)
{
	char *ret = (char*)malloc(c + 1);
	ret[c] = 0;
	strncpy(ret, str, c);
	return ret;
}

void GGPROTO::parsecontacts(char *contacts)
{
	char *p = strchr(contacts, ':'), *n;
	char *strFirstName, *strLastName, *strNickname, *strNick, *strPhone, *strGroup, *strUin, *strMail;
	uin_t uin;

	// Skip to proper data
	if (p && p < strchr(contacts, ';')) p++;
	else p = contacts;

	while(p)
	{
		// Processing line
		strFirstName = strLastName = strNickname = strNick = strPhone = strGroup = strUin = strMail = NULL;
		uin = 0;

		// FirstName
		if (p)
		{
			n = strchr(p, ';');
			if (n && n != p) strFirstName = strndup(p, (n - p));
			p = (n + 1);
		}
		// LastName
		if (n && p)
		{
			n = strchr(p, ';');
			if (n && n != p) strLastName = strndup(p, (n - p));
			p = (n + 1);
		}
		// Nickname
		if (n && p)
		{
			n = strchr(p, ';');
			if (n && n != p) strNickname = strndup(p, (n - p));
			p = (n + 1);
		}
		// Nick
		if (n && p)
		{
			n = strchr(p, ';');
			if (n && n != p) strNick = strndup(p, (n - p));
			p = (n + 1);
		}
		// Phone
		if (n && p)
		{
			n = strchr(p, ';');
			if (n && n != p)
			{
				strPhone = (char*)malloc((n - p) + 5);
				strncpy(strPhone, p, (n - p));
				strcpy((strPhone + (n - p)), " SMS"); // Add SMS postfix
			}
			p = (n + 1);
		}
		// Group
		if (n && p)
		{
			n = strchr(p, ';');
			if (n && n != p) strGroup = strndup(p, (n - p));
			p = (n + 1);
		}
		// Uin
		if (n && p)
		{
			n = strchr(p, ';');
			if (n && n != p)
			{
				strUin = strndup(p, (n - p));
				uin = atoi(strUin);
			}
			p = (n + 1);
		}
		// Mail
		if (n && p)
		{
			n = strchr(p, ';');
			if (n && n != p) strMail = strndup(p, (n - p));
			n = strchr(p, '\n');
			p = (n + 1);
		}
		if (!n) p = NULL;

		// Loadup contact
		if (uin && strNick)
		{
			HANDLE hContact = getcontact(uin, 1, 1, _A2T(strNick));
#ifdef DEBUGMODE
			netlog("parsecontacts(): Found contact %d with nickname \"%s\".", uin, strNick);
#endif
			// Write group
			if (hContact && strGroup)
				db_set_s(hContact, "CList", "Group", CreateGroup(strGroup));

			// Write misc data
			if (hContact && strFirstName){
				TCHAR *tstrFirstName = mir_a2t(strFirstName);
				db_set_ts(hContact, m_szModuleName, GG_KEY_PD_FIRSTNAME, tstrFirstName);
				mir_free(tstrFirstName);
			}
			if (hContact && strLastName){
				TCHAR *tstrLastName = mir_a2t(strLastName);
				db_set_ts(hContact, m_szModuleName, GG_KEY_PD_LASTNAME, tstrLastName);
				mir_free(tstrLastName);
			}
			if (hContact && strPhone) db_set_s(hContact, "UserInfo", "MyPhone0", strPhone); // Store now in User Info
			if (hContact && strMail) db_set_s(hContact, "UserInfo", "Mye-mail0", strMail); // Store now in User Info
		}

		// Release stuff
		if (strFirstName) free(strFirstName);
		if (strLastName) free(strLastName);
		if (strNickname) free(strNickname);
		if (strNick) free(strNick);
		if (strPhone) free(strPhone);
		if (strGroup) free(strGroup);
		if (strUin) free(strUin);
		if (strMail) free(strMail);
	}
}

//////////////////////////////////////////////////////////
// import from server

INT_PTR GGPROTO::import_server(WPARAM wParam, LPARAM lParam)
{
	char *password;
	uin_t uin;
	DBVARIANT dbv;

	// Check if connected
	if (!isonline())
	{
		MessageBox(NULL,
			TranslateT("You have to be connected before you can import/export contacts from/to server."),
			m_tszUserName, MB_OK | MB_ICONSTOP
		);
		return 0;
	}

	// Readup password
	if (!db_get_s(NULL, m_szModuleName, GG_KEY_PASSWORD, &dbv, DBVT_ASCIIZ))
	{
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
		password = _strdup(dbv.pszVal);
		db_free(&dbv);
	}
	else return 0;

	if (!(uin = db_get_dw(NULL, m_szModuleName, GG_KEY_UIN, 0)))
		return 0;

	// Making contacts list
	gg_EnterCriticalSection(&sess_mutex, "import_server", 65, "sess_mutex", 1);
	if (gg_userlist_request(sess, GG_USERLIST_GET, NULL) == -1)
	{
		TCHAR error[128];
		gg_LeaveCriticalSection(&sess_mutex, "import_server", 65, 1, "sess_mutex", 1);
		mir_sntprintf(error, SIZEOF(error), TranslateT("List cannot be imported because of error:\n\t%s (Error: %d)"), _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		netlog("import_server(): Cannot import list. errno:%d: %s", errno, strerror(errno));
	}
	gg_LeaveCriticalSection(&sess_mutex, "import_server", 65, 2, "sess_mutex", 1);
	free(password);

	return 0;
}

//////////////////////////////////////////////////////////
// remove from server

INT_PTR GGPROTO::remove_server(WPARAM wParam, LPARAM lParam)
{
	char *password;
	uin_t uin;
	DBVARIANT dbv;

	// Check if connected
	if (!isonline())
	{
		MessageBox(NULL,
			TranslateT("You have to be connected before you can import/export contacts from/to server."),
			m_tszUserName, MB_OK | MB_ICONSTOP
		);
		return 0;
	}

	// Readup password
	if (!db_get_s(NULL, m_szModuleName, GG_KEY_PASSWORD, &dbv, DBVT_ASCIIZ))
	{
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
		password = _strdup(dbv.pszVal);
		db_free(&dbv);
	}
	else return 0;

	if (!(uin = db_get_dw(NULL, m_szModuleName, GG_KEY_UIN, 0)))
		return 0;

	// Making contacts list
	gg_EnterCriticalSection(&sess_mutex, "remove_server", 66, "sess_mutex", 1);
	if (gg_userlist_request(sess, GG_USERLIST_PUT, NULL) == -1)
	{
		TCHAR error[128];
		gg_LeaveCriticalSection(&sess_mutex, "remove_server", 66, 1, "sess_mutex", 1);
		mir_sntprintf(error, SIZEOF(error), TranslateT("List cannot be removeed because of error: %s (Error: %d)"), _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		netlog("remove_server(): Cannot remove list. errno=%d: %s", errno, strerror(errno));
	}
	gg_LeaveCriticalSection(&sess_mutex, "remove_server", 66, 2, "sess_mutex", 1);

	// Set list removal
	is_list_remove = TRUE;
	free(password);

	return 0;
}

INT_PTR GGPROTO::import_text(WPARAM wParam, LPARAM lParam)
{
	TCHAR str[MAX_PATH];
	TCHAR filter[512], *pfilter;
	struct _stat st;
	FILE *f;

	OPENFILENAME ofn = {0};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	_tcsncpy(filter, TranslateT("Text files"), SIZEOF(filter));
	_tcsncat(filter, _T(" (*.txt)"), SIZEOF(filter) - _tcslen(filter));
	pfilter = filter + _tcslen(filter) + 1;
	if (pfilter >= filter + SIZEOF(filter))
		return 0;

	_tcsncpy(pfilter, _T("*.TXT"), SIZEOF(filter) - (pfilter - filter));
	pfilter = pfilter + _tcslen(pfilter) + 1;
	if (pfilter >= filter + SIZEOF(filter))
		return 0;
	_tcsncpy(pfilter, TranslateT("All Files"), SIZEOF(filter) - (pfilter - filter));
	_tcsncat(pfilter, _T(" (*)"), SIZEOF(filter) - (pfilter - filter) - _tcslen(pfilter));
	pfilter = pfilter + _tcslen(pfilter) + 1;

	if (pfilter >= filter + SIZEOF(filter))
		return 0;

	_tcsncpy(pfilter, _T("*"), SIZEOF(filter) - (pfilter - filter));
	pfilter = pfilter + _tcslen(pfilter) + 1;
	if (pfilter >= filter + SIZEOF(filter))
		return 0;

	*pfilter = '\0';
	*str = '\0';

	ofn.lpstrFilter = filter;
	ofn.lpstrFile = str;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nMaxFile = sizeof(str);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = _T("txt");

#ifdef DEBUGMODE
	netlog("import_text()");
#endif
	if (!GetOpenFileName(&ofn)) return 0;

	f = _tfopen(str, _T("r"));
	_tstat(str, &st);

	if (f && st.st_size)
	{
		char *contacts = (char*)malloc(st.st_size * sizeof(char));
		fread(contacts, sizeof(char), st.st_size, f);
		fclose(f);
		parsecontacts(contacts);
		free(contacts);

		MessageBox(NULL, TranslateT("List import successful."), m_tszUserName, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		TCHAR error[256];
		mir_sntprintf(error, SIZEOF(error), TranslateT("List cannot be imported from file \"%s\" because of error:\n\t%s (Error: %d)"), str, _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		netlog("import_text(): Cannot import list from file \"%S\". errno=%d: %s", str, errno, strerror(errno));
	}

	return 0;
}

INT_PTR GGPROTO::export_text(WPARAM wParam, LPARAM lParam)
{
	TCHAR str[MAX_PATH];
	OPENFILENAME ofn = {0};
	TCHAR filter[512], *pfilter;
	FILE *f;

	_tcsncpy(str, TranslateT("contacts"), SIZEOF(str));
	_tcsncat(str, _T(".txt"), SIZEOF(str) - _tcslen(str));

	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	_tcsncpy(filter, TranslateT("Text files"), SIZEOF(filter));
	_tcsncat(filter, _T(" (*.txt)"), SIZEOF(filter) - _tcslen(filter));
	pfilter = filter + _tcslen(filter) + 1;
	if (pfilter >= filter + SIZEOF(filter))
		return 0;
	_tcsncpy(pfilter, _T("*.TXT"), SIZEOF(filter) - (pfilter - filter));
	pfilter = pfilter + _tcslen(pfilter) + 1;
	if (pfilter >= filter + SIZEOF(filter))
		return 0;
	_tcsncpy(pfilter, TranslateT("All Files"), SIZEOF(filter) - (pfilter - filter));
	_tcsncat(pfilter, _T(" (*)"), SIZEOF(filter) - (pfilter - filter) - _tcslen(pfilter));
	pfilter = pfilter + _tcslen(pfilter) + 1;
	if (pfilter >= filter + SIZEOF(filter))
		return 0;
	_tcsncpy(pfilter, _T("*"), SIZEOF(filter) - (pfilter - filter));
	pfilter = pfilter + _tcslen(pfilter) + 1;
	if (pfilter >= filter + SIZEOF(filter))
		return 0;
	*pfilter = '\0';
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = str;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.nMaxFile = sizeof(str);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = _T("txt");

#ifdef DEBUGMODE
	netlog("export_text(%S).", str);
#endif
	if (!GetSaveFileName(&ofn)) return 0;

	if (f = _tfopen(str, _T("w"))) {
		char *contacts = gg_makecontacts(this, 0);
		fwrite(contacts, sizeof(char), strlen(contacts), f);
		fclose(f);
		free(contacts);

		MessageBox(NULL, TranslateT("List export successful."), m_tszUserName, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		TCHAR error[128];
		mir_sntprintf(error, SIZEOF(error), TranslateT("List cannot be exported to file \"%s\" because of error:\n\t%s (Error: %d)"), str, _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		netlog("export_text(): Cannot export list to file \"%s\". errno=%d: %s", str, errno, strerror(errno));
	}

	return 0;
}

//////////////////////////////////////////////////////////
// export to server

INT_PTR GGPROTO::export_server(WPARAM wParam, LPARAM lParam)
{
	char *password, *contacts;
	uin_t uin;
	DBVARIANT dbv;

	// Check if connected
	if (!isonline())
	{
		MessageBox(NULL,
			TranslateT("You have to be connected before you can import/export contacts from/to server."),
			m_tszUserName, MB_OK | MB_ICONSTOP
		);
		return 0;
	}

	// Readup password
	if (!db_get_s(NULL, m_szModuleName, GG_KEY_PASSWORD, &dbv, DBVT_ASCIIZ))
	{
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
		password = _strdup(dbv.pszVal);
		db_free(&dbv);
	}
	else return 0;

	if (!(uin = db_get_dw(NULL, m_szModuleName, GG_KEY_UIN, 0)))
		return 0;

	// Making contacts list
	contacts = gg_makecontacts(this, 1);

#ifdef DEBUGMODE
		netlog("export_server(): gg_userlist_request(%s).", contacts);
#endif

	gg_EnterCriticalSection(&sess_mutex, "export_server", 67, "sess_mutex", 1);
	if (gg_userlist_request(sess, GG_USERLIST_PUT, contacts) == -1)
	{
		TCHAR error[128];
		gg_LeaveCriticalSection(&sess_mutex, "export_server", 67, 1, "sess_mutex", 1);
		mir_sntprintf(error, SIZEOF(error), TranslateT("List cannot be exported because of error:\n\t%s (Error: %d)"), _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		netlog("export_server(): Cannot export list. errno=%d: %s", errno, strerror(errno));
	}
	gg_LeaveCriticalSection(&sess_mutex, "export_server", 67, 2, "sess_mutex", 1);

	// Set list removal
	is_list_remove = FALSE;
	free(contacts);
	free(password);

	return 0;
}

//////////////////////////////////////////////////////////
// Import menus and stuff

void GGPROTO::import_init(HGENMENU hRoot)
{
	// Import from server item
	char service[64];
	mir_snprintf(service, sizeof(service), "%s%s", m_szModuleName, GGS_IMPORT_SERVER);
	CreateService(GGS_IMPORT_SERVER, &GGPROTO::import_server);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;
	mi.hParentMenu = hRoot;
	mi.position = 2000500001;
	mi.icolibItem = iconList[1].hIcolib;
	mi.ptszName = LPGENT("Import List From &Server");
	mi.pszService = service;
 	hMainMenu[2] = Menu_AddProtoMenuItem(&mi);

	// Import from textfile
	mir_snprintf(service, sizeof(service), "%s%s", m_szModuleName, GGS_IMPORT_TEXT);
	CreateService(GGS_IMPORT_TEXT, &GGPROTO::import_text);

	mi.position = 2000500002;
	mi.icolibItem = iconList[2].hIcolib;
	mi.ptszName = LPGENT("Import List From &Text File...");
	mi.pszService = service;
	hMainMenu[3] = Menu_AddProtoMenuItem(&mi);

	// Remove from server
	mir_snprintf(service, sizeof(service), "%s%s", m_szModuleName, GGS_REMOVE_SERVER);
	CreateService(GGS_REMOVE_SERVER, &GGPROTO::remove_server);

	mi.position = 2000500003;
	mi.icolibItem = iconList[3].hIcolib;
	mi.ptszName = LPGENT("&Remove List From Server");
	mi.pszService = service;
	hMainMenu[4] = Menu_AddProtoMenuItem(&mi);

	// Export to server
	mir_snprintf(service, sizeof(service), "%s%s", m_szModuleName, GGS_EXPORT_SERVER);
	CreateService(GGS_EXPORT_SERVER, &GGPROTO::export_server);

	mi.position = 2005000001;
	mi.icolibItem = iconList[4].hIcolib;
	mi.ptszName = LPGENT("Export List To &Server");
	mi.pszService = service;
	hMainMenu[5] = Menu_AddProtoMenuItem(&mi);

	// Export to textfile
	mir_snprintf(service, sizeof(service), "%s%s", m_szModuleName, GGS_EXPORT_TEXT);
	CreateService(GGS_EXPORT_TEXT, &GGPROTO::export_text);

	mi.position = 2005000002;
	mi.icolibItem = iconList[5].hIcolib;
	mi.ptszName = LPGENT("Export List To &Text File...");
	mi.pszService = service;
	hMainMenu[6] = Menu_AddProtoMenuItem(&mi);
}
