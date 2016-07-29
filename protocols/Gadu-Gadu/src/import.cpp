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

char *gg_makecontacts(GGPROTO *gg, int cr)
{
	string_t s = string_init(NULL);
	char *contacts;

	// Readup contacts
	for (MCONTACT hContact = db_find_first(gg->m_szModuleName); hContact; hContact = db_find_next(hContact, gg->m_szModuleName)) {
		if (gg->isChatRoom(hContact))
			continue;

		// Readup FirstName
		DBVARIANT dbv;
		if (!gg->getWString(hContact, GG_KEY_PD_FIRSTNAME, &dbv))
		{
			char* pszValA = mir_u2a(dbv.ptszVal);
			string_append(s, dbv.pszVal);
			mir_free(pszValA);
			db_free(&dbv);
		}
		string_append_c(s, ';');
		// Readup LastName
		if (!gg->getWString(hContact, GG_KEY_PD_LASTNAME, &dbv))
		{
			char* pszValA = mir_u2a(dbv.ptszVal);
			string_append(s, dbv.pszVal);
			mir_free(pszValA);
			db_free(&dbv);
		}
		string_append_c(s, ';');

		// Readup Nick
		if (!db_get_ws(hContact, "CList", "MyHandle", &dbv) || !gg->getWString(hContact, GG_KEY_NICK, &dbv))
		{
			char* dbvA = mir_u2a(dbv.ptszVal);
			DBVARIANT dbv2;
			if (!gg->getWString(hContact, GG_KEY_PD_NICKNAME, &dbv2))
			{
				char* pszValA = mir_u2a(dbv2.ptszVal);
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
		string_append(s, ditoa(gg->getDword(hContact, GG_KEY_UIN, 0)));
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
	gg->debugLogA("gg_makecontacts(): \n%s", contacts);
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
	char *p = strchr(contacts, ':'), *n = NULL;
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
				mir_strcpy((strPhone + (n - p)), " SMS"); // Add SMS postfix
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
			MCONTACT hContact = getcontact(uin, 1, 1, _A2T(strNick));
#ifdef DEBUGMODE
			debugLogA("parsecontacts(): Found contact %d with nickname \"%s\".", uin, strNick);
#endif
			// Write group
			if (hContact && strGroup) {
				ptrW tszGrpName( mir_a2u(strGroup));
				Clist_GroupCreate(0, tszGrpName);
				db_set_ws(hContact, "CList", "Group", tszGrpName);
			}

			// Write misc data
			if (hContact && strFirstName){
				wchar_t *tstrFirstName = mir_a2u(strFirstName);
				setWString(hContact, GG_KEY_PD_FIRSTNAME, tstrFirstName);
				mir_free(tstrFirstName);
			}
			if (hContact && strLastName){
				wchar_t *tstrLastName = mir_a2u(strLastName);
				setWString(hContact, GG_KEY_PD_LASTNAME, tstrLastName);
				mir_free(tstrLastName);
			}
			if (hContact && strPhone) db_set_s(hContact, "UserInfo", "MyPhone0", strPhone); // Store now in User Info
			if (hContact && strMail) db_set_s(hContact, "UserInfo", "Mye-mail0", strMail); // Store now in User Info
		}

		// Release stuff
		free(strFirstName);
		free(strLastName);
		free(strNickname);
		free(strNick);
		free(strPhone);
		free(strGroup);
		free(strUin);
		free(strMail);
	}
}

//////////////////////////////////////////////////////////
// import from server
//
INT_PTR GGPROTO::import_server(WPARAM, LPARAM)
{
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
	ptrA password(getStringA(GG_KEY_PASSWORD));
	if (password == NULL)
		return 0;

	uin_t uin;
	if (!(uin = getDword(GG_KEY_UIN, 0)))
		return 0;

	// Making contacts list
	gg_EnterCriticalSection(&sess_mutex, "import_server", 65, "sess_mutex", 1);
	if (gg_userlist_request(sess, GG_USERLIST_GET, NULL) == -1)
	{
		wchar_t error[128];
		gg_LeaveCriticalSection(&sess_mutex, "import_server", 65, 1, "sess_mutex", 1);
		mir_snwprintf(error, TranslateT("List cannot be imported because of error:\n\t%s (Error: %d)"), ws_strerror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		debugLogW(L"import_server(): Cannot import list. errno:%d: %s", errno, ws_strerror(errno));
	}
	gg_LeaveCriticalSection(&sess_mutex, "import_server", 65, 2, "sess_mutex", 1);

	return 0;
}

//////////////////////////////////////////////////////////
// remove from server
//
INT_PTR GGPROTO::remove_server(WPARAM, LPARAM)
{
	// Check if connected
	if (!isonline()) {
		MessageBox(NULL,
			TranslateT("You have to be connected before you can import/export contacts from/to server."),
			m_tszUserName, MB_OK | MB_ICONSTOP
		);
		return 0;
	}

	// Readup password
	ptrA password(getStringA(GG_KEY_PASSWORD));
	if (password == NULL)
		return 0;

	uin_t uin;
	if (!(uin = getDword(GG_KEY_UIN, 0)))
		return 0;

	// Making contacts list
	gg_EnterCriticalSection(&sess_mutex, "remove_server", 66, "sess_mutex", 1);
	if (gg_userlist_request(sess, GG_USERLIST_PUT, NULL) == -1)
	{
		wchar_t error[128];
		gg_LeaveCriticalSection(&sess_mutex, "remove_server", 66, 1, "sess_mutex", 1);
		mir_snwprintf(error, TranslateT("List cannot be removed because of error: %s (Error: %d)"), ws_strerror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		debugLogW(L"remove_server(): Cannot remove list. errno=%d: %s", errno, ws_strerror(errno));
	}
	gg_LeaveCriticalSection(&sess_mutex, "remove_server", 66, 2, "sess_mutex", 1);

	// Set list removal
	is_list_remove = TRUE;

	return 0;
}

INT_PTR GGPROTO::import_text(WPARAM, LPARAM)
{
	wchar_t str[MAX_PATH];
	wchar_t filter[512], *pfilter;
	struct _stat st;

	OPENFILENAME ofn = {0};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	wcsncpy(filter, TranslateT("Text files"), _countof(filter));
	mir_wstrncat(filter, L" (*.txt)", _countof(filter) - mir_wstrlen(filter));
	pfilter = filter + mir_wstrlen(filter) + 1;
	if (pfilter >= filter + _countof(filter))
		return 0;

	wcsncpy(pfilter, L"*.TXT", _countof(filter) - (pfilter - filter));
	pfilter = pfilter + mir_wstrlen(pfilter) + 1;
	if (pfilter >= filter + _countof(filter))
		return 0;
	wcsncpy(pfilter, TranslateT("All Files"), _countof(filter) - (pfilter - filter));
	mir_wstrncat(pfilter, L" (*)", _countof(filter) - (pfilter - filter) - mir_wstrlen(pfilter));
	pfilter = pfilter + mir_wstrlen(pfilter) + 1;

	if (pfilter >= filter + _countof(filter))
		return 0;

	wcsncpy(pfilter, L"*", _countof(filter) - (pfilter - filter));
	pfilter = pfilter + mir_wstrlen(pfilter) + 1;
	if (pfilter >= filter + _countof(filter))
		return 0;

	*pfilter = '\0';
	*str = '\0';

	ofn.lpstrFilter = filter;
	ofn.lpstrFile = str;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nMaxFile = _countof(str);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = L"txt";

#ifdef DEBUGMODE
	debugLogA("import_text()");
#endif
	if (!GetOpenFileName(&ofn)) return 0;

	FILE *f = _wfopen(str, L"r");
	_wstat(str, &st);

	if (f && st.st_size)
	{
		char *contacts = (char*)mir_calloc((st.st_size * sizeof(char)) + 1); // zero-terminate it
		fread(contacts, sizeof(char), st.st_size, f);
		fclose(f);
		parsecontacts(contacts);
		mir_free(contacts);

		MessageBox(NULL, TranslateT("List import successful."), m_tszUserName, MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	else
	{
		wchar_t error[256];
		mir_snwprintf(error, TranslateT("List cannot be imported from file \"%s\" because of error:\n\t%s (Error: %d)"), str, _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		debugLogW(L"import_text(): Cannot import list from file \"%s\". errno=%d: %s", str, errno, _tcserror(errno));
		if (f)
			fclose(f);
		return 0;
	}
}

INT_PTR GGPROTO::export_text(WPARAM, LPARAM)
{
	wchar_t str[MAX_PATH];
	OPENFILENAME ofn = {0};
	wchar_t filter[512], *pfilter;

	wcsncpy(str, TranslateT("contacts"), _countof(str));
	mir_wstrncat(str, L".txt", _countof(str) - mir_wstrlen(str));

	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	wcsncpy(filter, TranslateT("Text files"), _countof(filter));
	mir_wstrncat(filter, L" (*.txt)", _countof(filter) - mir_wstrlen(filter));
	pfilter = filter + mir_wstrlen(filter) + 1;
	if (pfilter >= filter + _countof(filter))
		return 0;
	wcsncpy(pfilter, L"*.TXT", _countof(filter) - (pfilter - filter));
	pfilter = pfilter + mir_wstrlen(pfilter) + 1;
	if (pfilter >= filter + _countof(filter))
		return 0;
	wcsncpy(pfilter, TranslateT("All Files"), _countof(filter) - (pfilter - filter));
	mir_wstrncat(pfilter, L" (*)", _countof(filter) - (pfilter - filter) - mir_wstrlen(pfilter));
	pfilter = pfilter + mir_wstrlen(pfilter) + 1;
	if (pfilter >= filter + _countof(filter))
		return 0;
	wcsncpy(pfilter, L"*", _countof(filter) - (pfilter - filter));
	pfilter = pfilter + mir_wstrlen(pfilter) + 1;
	if (pfilter >= filter + _countof(filter))
		return 0;
	*pfilter = '\0';
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = str;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.nMaxFile = _countof(str);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = L"txt";

#ifdef DEBUGMODE
	debugLogW(L"export_text(%s).", str);
#endif
	if (!GetSaveFileName(&ofn)) return 0;

	FILE *f = _wfopen(str, L"w");
	if (f) {
		char *contacts = gg_makecontacts(this, 0);
		fwrite(contacts, sizeof(char), mir_strlen(contacts), f);
		fclose(f);
		free(contacts);

		MessageBox(NULL, TranslateT("List export successful."), m_tszUserName, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		wchar_t error[128];
		mir_snwprintf(error, TranslateT("List cannot be exported to file \"%s\" because of error:\n\t%s (Error: %d)"), str, _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		debugLogW(L"export_text(): Cannot export list to file \"%s\". errno=%d: %s", str, errno, _tcserror(errno));
	}

	return 0;
}

//////////////////////////////////////////////////////////
// export to server
//
INT_PTR GGPROTO::export_server(WPARAM, LPARAM)
{
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
	ptrA password(getStringA(GG_KEY_PASSWORD));
	if (password == NULL)
		return 0;

	uin_t uin = getDword(GG_KEY_UIN, 0);
	if (!uin)
		return 0;

	// Making contacts list
	char *contacts = gg_makecontacts(this, 1);

#ifdef DEBUGMODE
		debugLogA("export_server(): gg_userlist_request(%s).", contacts);
#endif

	gg_EnterCriticalSection(&sess_mutex, "export_server", 67, "sess_mutex", 1);
	if (gg_userlist_request(sess, GG_USERLIST_PUT, contacts) == -1)
	{
		wchar_t error[128];
		gg_LeaveCriticalSection(&sess_mutex, "export_server", 67, 1, "sess_mutex", 1);
		mir_snwprintf(error, TranslateT("List cannot be exported because of error:\n\t%s (Error: %d)"), ws_strerror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		debugLogW(L"export_server(): Cannot export list. errno=%d: %s", errno, ws_strerror(errno));
	}
	gg_LeaveCriticalSection(&sess_mutex, "export_server", 67, 2, "sess_mutex", 1);

	// Set list removal
	is_list_remove = FALSE;
	free(contacts);

	return 0;
}

//////////////////////////////////////////////////////////
// Import menus and stuff
//
void GGPROTO::import_init(HGENMENU hRoot)
{
	CMenuItem mi;
	mi.flags = CMIF_UNICODE;
	mi.root = hRoot;

	// Import from server item
	SET_UID(mi, 0x50ea554c, 0x6371, 0x495c, 0x86, 0x7f, 0x28, 0x7, 0x47, 0x59, 0x20, 0x20);
	mi.pszService = GGS_IMPORT_SERVER;
	CreateProtoService(mi.pszService, &GGPROTO::import_server);
	mi.position = 2000500001;
	mi.hIcolibItem = iconList[1].hIcolib;
	mi.name.w = LPGENW("Import List From &Server");
 	hMainMenu[2] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Import from textfile
	SET_UID(mi, 0x39c036d5, 0x6eb5, 0x49ae, 0xa8, 0x78, 0x8a, 0x72, 0xeb, 0xf, 0x15, 0x3d);
	mi.pszService = GGS_IMPORT_TEXT;
	CreateProtoService(mi.pszService, &GGPROTO::import_text);
	mi.position = 2000500002;
	mi.hIcolibItem = iconList[2].hIcolib;
	mi.name.w = LPGENW("Import List From &Text File...");
	hMainMenu[3] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Remove from server
	SET_UID(mi, 0x6bdbe700, 0xf240, 0x4ed8, 0x86, 0x26, 0x31, 0x85, 0xf8, 0x5, 0xe5, 0xb1);
	mi.pszService = GGS_REMOVE_SERVER;
	CreateProtoService(mi.pszService, &GGPROTO::remove_server);
	mi.position = 2000500003;
	mi.hIcolibItem = iconList[3].hIcolib;
	mi.name.w = LPGENW("&Remove List From Server");
	hMainMenu[4] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Export to server
	SET_UID(mi, 0xddb8e1f0, 0x7282, 0x4105, 0x90, 0x83, 0x92, 0x25, 0x21, 0x7d, 0xe2, 0x6a);
	mi.pszService = GGS_EXPORT_SERVER;
	CreateProtoService(mi.pszService, &GGPROTO::export_server);
	mi.position = 2005000001;
	mi.hIcolibItem = iconList[4].hIcolib;
	mi.name.w = LPGENW("Export List To &Server");
	hMainMenu[5] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Export to textfile
	SET_UID(mi, 0x7ee0000b, 0xb202, 0x4fe4, 0xb3, 0x42, 0x3b, 0x39, 0xfb, 0x9d, 0x80, 0x5d);
	mi.pszService = GGS_EXPORT_TEXT;
	CreateProtoService(mi.pszService, &GGPROTO::export_text);
	mi.position = 2005000002;
	mi.hIcolibItem = iconList[5].hIcolib;
	mi.name.w = LPGENW("Export List To &Text File...");
	hMainMenu[6] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
}
