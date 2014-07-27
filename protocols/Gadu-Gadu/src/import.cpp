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
		if (!gg->getTString(hContact, GG_KEY_PD_FIRSTNAME, &dbv))
		{
			char* pszValA = mir_t2a(dbv.ptszVal);
			string_append(s, dbv.pszVal);
			mir_free(pszValA);
			db_free(&dbv);
		}
		string_append_c(s, ';');
		// Readup LastName
		if (!gg->getTString(hContact, GG_KEY_PD_LASTNAME, &dbv))
		{
			char* pszValA = mir_t2a(dbv.ptszVal);
			string_append(s, dbv.pszVal);
			mir_free(pszValA);
			db_free(&dbv);
		}
		string_append_c(s, ';');

		// Readup Nick
		if (!db_get_ts(hContact, "CList", "MyHandle", &dbv) || !gg->getTString(hContact, GG_KEY_NICK, &dbv))
		{
			char* dbvA = mir_t2a(dbv.ptszVal);
			DBVARIANT dbv2;
			if (!gg->getTString(hContact, GG_KEY_PD_NICKNAME, &dbv2))
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
			MCONTACT hContact = getcontact(uin, 1, 1, _A2T(strNick));
#ifdef DEBUGMODE
			debugLogA("parsecontacts(): Found contact %d with nickname \"%s\".", uin, strNick);
#endif
			// Write group
			if (hContact && strGroup) {
				ptrT tszGrpName( mir_a2t(strGroup));
				Clist_CreateGroup(0, tszGrpName);
				db_set_ts(hContact, "CList", "Group", tszGrpName);
			}

			// Write misc data
			if (hContact && strFirstName){
				TCHAR *tstrFirstName = mir_a2t(strFirstName);
				setTString(hContact, GG_KEY_PD_FIRSTNAME, tstrFirstName);
				mir_free(tstrFirstName);
			}
			if (hContact && strLastName){
				TCHAR *tstrLastName = mir_a2t(strLastName);
				setTString(hContact, GG_KEY_PD_LASTNAME, tstrLastName);
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

INT_PTR GGPROTO::import_server(WPARAM wParam, LPARAM lParam)
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
		TCHAR error[128];
		gg_LeaveCriticalSection(&sess_mutex, "import_server", 65, 1, "sess_mutex", 1);
		mir_sntprintf(error, SIZEOF(error), TranslateT("List cannot be imported because of error:\n\t%s (Error: %d)"), _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		debugLogA("import_server(): Cannot import list. errno:%d: %s", errno, strerror(errno));
	}
	gg_LeaveCriticalSection(&sess_mutex, "import_server", 65, 2, "sess_mutex", 1);

	return 0;
}

//////////////////////////////////////////////////////////
// remove from server

INT_PTR GGPROTO::remove_server(WPARAM wParam, LPARAM lParam)
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
		TCHAR error[128];
		gg_LeaveCriticalSection(&sess_mutex, "remove_server", 66, 1, "sess_mutex", 1);
		mir_sntprintf(error, SIZEOF(error), TranslateT("List cannot be removed because of error: %s (Error: %d)"), _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		debugLogA("remove_server(): Cannot remove list. errno=%d: %s", errno, strerror(errno));
	}
	gg_LeaveCriticalSection(&sess_mutex, "remove_server", 66, 2, "sess_mutex", 1);

	// Set list removal
	is_list_remove = TRUE;

	return 0;
}

INT_PTR GGPROTO::import_text(WPARAM wParam, LPARAM lParam)
{
	TCHAR str[MAX_PATH];
	TCHAR filter[512], *pfilter;
	struct _stat st;

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
	debugLogA("import_text()");
#endif
	if (!GetOpenFileName(&ofn)) return 0;

	FILE *f = _tfopen(str, _T("r"));
	_tstat(str, &st);

	if (f && st.st_size)
	{
		char *contacts = (char*)mir_calloc((st.st_size * sizeof(char)) + 1); // zero-terminate it
		fread(contacts, sizeof(char), st.st_size, f);
		fclose(f);
		parsecontacts(contacts);
		mir_free(contacts);

		MessageBox(NULL, TranslateT("List import successful."), m_tszUserName, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		TCHAR error[256];
		mir_sntprintf(error, SIZEOF(error), TranslateT("List cannot be imported from file \"%s\" because of error:\n\t%s (Error: %d)"), str, _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		debugLog(_T("import_text(): Cannot import list from file \"%s\". errno=%d: %s"), str, errno, strerror(errno));
	}

	return 0;
}

INT_PTR GGPROTO::export_text(WPARAM wParam, LPARAM lParam)
{
	TCHAR str[MAX_PATH];
	OPENFILENAME ofn = {0};
	TCHAR filter[512], *pfilter;

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
	debugLog(_T("export_text(%s)."), str);
#endif
	if (!GetSaveFileName(&ofn)) return 0;

	FILE *f = _tfopen(str, _T("w"));
	if (f) {
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
		debugLogA("export_text(): Cannot export list to file \"%s\". errno=%d: %s", str, errno, strerror(errno));
	}

	return 0;
}

//////////////////////////////////////////////////////////
// export to server

INT_PTR GGPROTO::export_server(WPARAM wParam, LPARAM lParam)
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
		TCHAR error[128];
		gg_LeaveCriticalSection(&sess_mutex, "export_server", 67, 1, "sess_mutex", 1);
		mir_sntprintf(error, SIZEOF(error), TranslateT("List cannot be exported because of error:\n\t%s (Error: %d)"), _tcserror(errno), errno);
		MessageBox(NULL, error, m_tszUserName, MB_OK | MB_ICONSTOP);
		debugLogA("export_server(): Cannot export list. errno=%d: %s", errno, strerror(errno));
	}
	gg_LeaveCriticalSection(&sess_mutex, "export_server", 67, 2, "sess_mutex", 1);

	// Set list removal
	is_list_remove = FALSE;
	free(contacts);

	return 0;
}

//////////////////////////////////////////////////////////
// Import menus and stuff

void GGPROTO::import_init(HGENMENU hRoot)
{
	// Import from server item
	char service[64];
	mir_snprintf(service, sizeof(service), "%s%s", m_szModuleName, GGS_IMPORT_SERVER);
	CreateProtoService(GGS_IMPORT_SERVER, &GGPROTO::import_server);

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
	CreateProtoService(GGS_IMPORT_TEXT, &GGPROTO::import_text);

	mi.position = 2000500002;
	mi.icolibItem = iconList[2].hIcolib;
	mi.ptszName = LPGENT("Import List From &Text File...");
	mi.pszService = service;
	hMainMenu[3] = Menu_AddProtoMenuItem(&mi);

	// Remove from server
	mir_snprintf(service, sizeof(service), "%s%s", m_szModuleName, GGS_REMOVE_SERVER);
	CreateProtoService(GGS_REMOVE_SERVER, &GGPROTO::remove_server);

	mi.position = 2000500003;
	mi.icolibItem = iconList[3].hIcolib;
	mi.ptszName = LPGENT("&Remove List From Server");
	mi.pszService = service;
	hMainMenu[4] = Menu_AddProtoMenuItem(&mi);

	// Export to server
	mir_snprintf(service, sizeof(service), "%s%s", m_szModuleName, GGS_EXPORT_SERVER);
	CreateProtoService(GGS_EXPORT_SERVER, &GGPROTO::export_server);

	mi.position = 2005000001;
	mi.icolibItem = iconList[4].hIcolib;
	mi.ptszName = LPGENT("Export List To &Server");
	mi.pszService = service;
	hMainMenu[5] = Menu_AddProtoMenuItem(&mi);

	// Export to textfile
	mir_snprintf(service, sizeof(service), "%s%s", m_szModuleName, GGS_EXPORT_TEXT);
	CreateProtoService(GGS_EXPORT_TEXT, &GGPROTO::export_text);

	mi.position = 2005000002;
	mi.icolibItem = iconList[5].hIcolib;
	mi.ptszName = LPGENT("Export List To &Text File...");
	mi.pszService = service;
	hMainMenu[6] = Menu_AddProtoMenuItem(&mi);
}
