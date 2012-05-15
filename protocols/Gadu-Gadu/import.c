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
		if (DBGetContactSettingString(NULL, "CListGroups", idstr, &dbv)) break;
		if (!strcmp(dbv.pszVal + 1, name)) {
			DBFreeVariant(&dbv);
			return 1;
		}
		DBFreeVariant(&dbv);
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
	if(p)
	{
		*p = 0;
		CreateGroup(groupName);
		*p = '\\';
	}

	// Is this a duplicate?
	if (!GroupNameExists(groupName))
	{
		lstrcpyn(groupName2 + 1, groupName, (int)strlen(groupName) + 1);

		// Find an unused id
		for (groupId = 0; ; groupId++) {
				_itoa(groupId, groupIdStr,10);
				if (DBGetContactSettingString(NULL, "CListGroups", groupIdStr, &dbv))
						break;
				DBFreeVariant(&dbv);
		}

		groupName2[0] = 1|GROUPF_EXPANDED;	// 1 is required so we never get '\0'
		DBWriteContactSettingString(NULL, "CListGroups", groupIdStr, groupName2);
	}
	return groupName;
}

char *gg_makecontacts(GGPROTO *gg, int cr)
{
	string_t s = string_init(NULL);
	char *contacts;

	// Readup contacts
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (szProto != NULL && !strcmp(szProto, GG_PROTO) && !DBGetContactSettingByte(hContact, GG_PROTO, "ChatRoom", 0))
		{
			DBVARIANT dbv;

			// Readup FirstName
			if (!DBGetContactSettingString(hContact, GG_PROTO, "FirstName", &dbv))
			{
				string_append(s, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			string_append_c(s, ';');
			// Readup LastName
			if (!DBGetContactSettingString(hContact, GG_PROTO, "LastName", &dbv))
			{
				string_append(s, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			string_append_c(s, ';');

			// Readup Nick
			if (!DBGetContactSettingString(hContact, "CList", "MyHandle", &dbv) || !DBGetContactSettingString(hContact, GG_PROTO, GG_KEY_NICK, &dbv))
			{
				DBVARIANT dbv2;
				if (!DBGetContactSettingString(hContact, GG_PROTO, "NickName", &dbv2))
				{
					string_append(s, dbv2.pszVal);
					DBFreeVariant(&dbv2);
				}
				else
					string_append(s, dbv.pszVal);
				string_append_c(s, ';');
				string_append(s, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			else
				string_append_c(s, ';');
			string_append_c(s, ';');

			// Readup Phone (fixed: uses stored editable phones)
			if (!DBGetContactSettingString(hContact, "UserInfo", "MyPhone0", &dbv))
			{
				// Remove SMS postfix
				char *sms = strstr(dbv.pszVal, " SMS");
				if(sms) *sms = 0;

				string_append(s, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			string_append_c(s, ';');
			// Readup Group
			if (!DBGetContactSettingString(hContact, "CList", "Group", &dbv))
			{
				string_append(s, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			string_append_c(s, ';');
			// Readup Uin
			string_append(s, ditoa(DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0)));
			string_append_c(s, ';');
			// Readup Mail (fixed: uses stored editable mails)
			if (!DBGetContactSettingString(hContact, "UserInfo", "Mye-mail0", &dbv))
			{
				string_append(s, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			if(cr)
				string_append(s, ";0;;0;\r\n");
			else
				string_append(s, ";0;;0;\n");
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}

	contacts = string_free(s, 0);

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_makecontacts(): \n%s", contacts);
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

void gg_parsecontacts(GGPROTO *gg, char *contacts)
{
	char *p = strchr(contacts, ':'), *n;
	char *strFirstName, *strLastName, *strNickname, *strNick, *strPhone, *strGroup, *strUin, *strMail;
	uin_t uin;

	// Skip to proper data
	if(p && p < strchr(contacts, ';')) p++;
	else p = contacts;

	while(p)
	{
		// Processing line
		strFirstName = strLastName = strNickname = strNick = strPhone = strGroup = strUin = strMail = NULL;
		uin = 0;

		// FirstName
		if(p)
		{
			n = strchr(p, ';');
			if(n && n != p) strFirstName = strndup(p, (n - p));
			p = (n + 1);
		}
		// LastName
		if(n && p)
		{
			n = strchr(p, ';');
			if(n && n != p) strLastName = strndup(p, (n - p));
			p = (n + 1);
		}
		// Nickname
		if(n && p)
		{
			n = strchr(p, ';');
			if(n && n != p) strNickname = strndup(p, (n - p));
			p = (n + 1);
		}
		// Nick
		if(n && p)
		{
			n = strchr(p, ';');
			if(n && n != p) strNick = strndup(p, (n - p));
			p = (n + 1);
		}
		// Phone
		if(n && p)
		{
			n = strchr(p, ';');
			if(n && n != p)
			{
				strPhone = malloc((n - p) + 5);
				strncpy(strPhone, p, (n - p));
				strcpy((strPhone + (n - p)), " SMS"); // Add SMS postfix
			}
			p = (n + 1);
		}
		// Group
		if(n && p)
		{
			n = strchr(p, ';');
			if(n && n != p) strGroup = strndup(p, (n - p));
			p = (n + 1);
		}
		// Uin
		if(n && p)
		{
			n = strchr(p, ';');
			if(n && n != p)
			{
				strUin = strndup(p, (n - p));
				uin = atoi(strUin);
			}
			p = (n + 1);
		}
		// Mail
		if(n && p)
		{
			n = strchr(p, ';');
			if(n && n != p) strMail = strndup(p, (n - p));
			n = strchr(p, '\n');
			p = (n + 1);
		}
		if(!n) p = NULL;

		// Loadup contact
		if(uin && strNick)
		{
			HANDLE hContact = gg_getcontact(gg, uin, 1, 1, strNick);
#ifdef DEBUGMODE
			gg_netlog(gg, "gg_parsecontacts(): Found contact %d with nickname \"%s\".", uin, strNick);
#endif
			// Write group
			if(hContact && strGroup)
				DBWriteContactSettingString(hContact, "CList", "Group", CreateGroup(strGroup));

			// Write misc data
			if(hContact && strFirstName) DBWriteContactSettingString(hContact, GG_PROTO, "FirstName", strFirstName);
			if(hContact && strLastName) DBWriteContactSettingString(hContact, GG_PROTO, "LastName", strLastName);
			if(hContact && strPhone) DBWriteContactSettingString(hContact, "UserInfo", "MyPhone0", strPhone); // Store now in User Info
			if(hContact && strMail) DBWriteContactSettingString(hContact, "UserInfo", "Mye-mail0", strMail); // Store now in User Info
		}

		// Release stuff
		if(strFirstName) free(strFirstName);
		if(strLastName) free(strLastName);
		if(strNickname) free(strNickname);
		if(strNick) free(strNick);
		if(strPhone) free(strPhone);
		if(strGroup) free(strGroup);
		if(strUin) free(strUin);
		if(strMail) free(strMail);
	}
}

//////////////////////////////////////////////////////////
// import from server
static INT_PTR gg_import_server(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	char *password;
	uin_t uin;
	DBVARIANT dbv;

	// Check if connected
	if (!gg_isonline(gg))
	{
		MessageBox(NULL,
			Translate("You have to be connected before you can import/export contacts from/to server."),
			GG_PROTONAME, MB_OK | MB_ICONSTOP
		);
		return 0;
	}

	// Readup password
	if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv))
	{
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
		password = _strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	else return 0;

	if (!(uin = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0)))
		return 0;

	// Making contacts list
	EnterCriticalSection(&gg->sess_mutex);
	if (gg_userlist_request(gg->sess, GG_USERLIST_GET, NULL) == -1)
	{
		char error[128];
		LeaveCriticalSection(&gg->sess_mutex);
		mir_snprintf(error, sizeof(error), Translate("List cannot be imported because of error:\n\t%s"), strerror(errno));
		MessageBox(
			NULL,
			error,
			GG_PROTONAME,
			MB_OK | MB_ICONSTOP
		);
		gg_netlog(gg, "gg_import_server(): Cannot import list because of \"%s\".", strerror(errno));
	}
	LeaveCriticalSection(&gg->sess_mutex);
	free(password);

	return 0;
}

//////////////////////////////////////////////////////////
// remove from server
static INT_PTR gg_remove_server(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	char *password;
	uin_t uin;
	DBVARIANT dbv;

	// Check if connected
	if (!gg_isonline(gg))
	{
		MessageBox(NULL,
			Translate("You have to be connected before you can import/export contacts from/to server."),
			GG_PROTONAME, MB_OK | MB_ICONSTOP
		);
		return 0;
	}

	// Readup password
	if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv))
	{
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
		password = _strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	else return 0;

	if (!(uin = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0)))
		return 0;

	// Making contacts list
	EnterCriticalSection(&gg->sess_mutex);
	if (gg_userlist_request(gg->sess, GG_USERLIST_PUT, NULL) == -1)
	{
		char error[128];
		LeaveCriticalSection(&gg->sess_mutex);
		mir_snprintf(error, sizeof(error), Translate("List cannot be removeed because of error:\n\t%s"), strerror(errno));
		MessageBox(
			NULL,
			error,
			GG_PROTONAME,
			MB_OK | MB_ICONSTOP
		);
		gg_netlog(gg, "gg_remove_server(): Cannot remove list because of \"%s\".", strerror(errno));
	}
	LeaveCriticalSection(&gg->sess_mutex);

	// Set list removal
	gg->list_remove = TRUE;
	free(password);

	return 0;
}

static INT_PTR gg_import_text(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	char str[MAX_PATH] = "\0";
	OPENFILENAME ofn = {0};
	char filter[512], *pfilter;
	struct _stat st;
	FILE *f;

	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	strncpy(filter, Translate("Text files"), sizeof(filter));
	strncat(filter, " (*.txt)", sizeof(filter) - strlen(filter));
	pfilter = filter + strlen(filter) + 1;
	if(pfilter >= filter + sizeof(filter)) return 0;
	strncpy(pfilter, "*.TXT", sizeof(filter) - (pfilter - filter));
	pfilter = pfilter + strlen(pfilter) + 1;
	if(pfilter >= filter + sizeof(filter)) return 0;
	strncpy(pfilter, Translate("All Files"), sizeof(filter) - (pfilter - filter));
	strncat(pfilter, " (*)", sizeof(filter) - (pfilter - filter) - strlen(pfilter));
	pfilter = pfilter + strlen(pfilter) + 1;
	if(pfilter >= filter + sizeof(filter)) return 0;
	strncpy(pfilter, "*", sizeof(filter) - (pfilter - filter));
	pfilter = pfilter + strlen(pfilter) + 1;
	if(pfilter >= filter + sizeof(filter)) return 0;
	*pfilter = '\0';
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = str;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nMaxFile = sizeof(str);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "txt";

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_import_text()");
#endif
	if(!GetOpenFileName(&ofn)) return 0;

	f = fopen(str, "r");
	_stat(str, &st);

	if(f && st.st_size)
	{
		char *contacts = malloc(st.st_size * sizeof(char));
		fread(contacts, sizeof(char), st.st_size, f);
		fclose(f);
		gg_parsecontacts(gg, contacts);
		free(contacts);

		MessageBox(
			NULL,
			Translate("List import successful."),
			GG_PROTONAME,
			MB_OK | MB_ICONINFORMATION
		);
	}
	else
	{
		char error[128];
		mir_snprintf(error, sizeof(error), Translate("List cannot be imported from file \"%s\" because of error:\n\t%s"), str, strerror(errno));
		MessageBox(
			NULL,
			error,
			GG_PROTONAME,
			MB_OK | MB_ICONSTOP
		);
		gg_netlog(gg, "gg_import_text(): Cannot import list from file \"%s\" because of \"%s\".", str, strerror(errno));
	}

	return 0;
}

static INT_PTR gg_export_text(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	char str[MAX_PATH];
	OPENFILENAME ofn = {0};
	char filter[512], *pfilter;
	FILE *f;

	strncpy(str, Translate("contacts"), sizeof(str));
	strncat(str, ".txt", sizeof(str) - strlen(str));

	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	strncpy(filter, Translate("Text files"), sizeof(filter));
	strncat(filter, " (*.txt)", sizeof(filter) - strlen(filter));
	pfilter = filter + strlen(filter) + 1;
	if(pfilter >= filter + sizeof(filter)) return 0;
	strncpy(pfilter, "*.TXT", sizeof(filter) - (pfilter - filter));
	pfilter = pfilter + strlen(pfilter) + 1;
	if(pfilter >= filter + sizeof(filter)) return 0;
	strncpy(pfilter, Translate("All Files"), sizeof(filter) - (pfilter - filter));
	strncat(pfilter, " (*)", sizeof(filter) - (pfilter - filter) - strlen(pfilter));
	pfilter = pfilter + strlen(pfilter) + 1;
	if(pfilter >= filter + sizeof(filter)) return 0;
	strncpy(pfilter, "*", sizeof(filter) - (pfilter - filter));
	pfilter = pfilter + strlen(pfilter) + 1;
	if(pfilter >= filter + sizeof(filter)) return 0;
	*pfilter = '\0';
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = str;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.nMaxFile = sizeof(str);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "txt";

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_export_text(%s).", str);
#endif
	if(!GetSaveFileName(&ofn)) return 0;

	if(f = fopen(str, "w"))
	{
		char *contacts = gg_makecontacts(gg, 0);
		fwrite(contacts, sizeof(char), strlen(contacts), f);
		fclose(f);
		free(contacts);

		MessageBox(
			NULL,
			Translate("List export successful."),
			GG_PROTONAME,
			MB_OK | MB_ICONINFORMATION
		);
	}
	else
	{
		char error[128];
		mir_snprintf(error, sizeof(error), Translate("List cannot be exported to file \"%s\" because of error:\n\t%s"), str, strerror(errno));
		MessageBox(
			NULL,
			error,
			GG_PROTONAME,
			MB_OK | MB_ICONSTOP
		);
		gg_netlog(gg, "gg_import_text(): Cannot export list to file \"%s\" because of \"%s\".", str, strerror(errno));
	}

	return 0;
}

//////////////////////////////////////////////////////////
// export to server
static INT_PTR gg_export_server(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	char *password, *contacts;
	uin_t uin;
	DBVARIANT dbv;

	// Check if connected
	if (!gg_isonline(gg))
	{
		MessageBox(NULL,
			Translate("You have to be connected before you can import/export contacts from/to server."),
			GG_PROTONAME, MB_OK | MB_ICONSTOP
		);
		return 0;
	}

	// Readup password
	if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv))
	{
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
		password = _strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	else return 0;

	if (!(uin = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0)))
		return 0;

	// Making contacts list
	contacts = gg_makecontacts(gg, 1);

#ifdef DEBUGMODE
		gg_netlog(gg, "gg_userlist_request(%s).", contacts);
#endif

	EnterCriticalSection(&gg->sess_mutex);
	if (gg_userlist_request(gg->sess, GG_USERLIST_PUT, contacts) == -1)
	{
		char error[128];
		LeaveCriticalSection(&gg->sess_mutex);
		mir_snprintf(error, sizeof(error), Translate("List cannot be exported because of error:\n\t%s"), strerror(errno));
		MessageBox(
			NULL,
			error,
			GG_PROTONAME,
			MB_OK | MB_ICONSTOP
		);
		gg_netlog(gg, "gg_export_server(): Cannot export list because of \"%s\".", strerror(errno));
	}
	LeaveCriticalSection(&gg->sess_mutex);

	// Set list removal
	gg->list_remove = FALSE;
	free(contacts);
	free(password);

	return 0;
}

//////////////////////////////////////////////////////////
// Import menus and stuff
void gg_import_init(GGPROTO *gg, HGENMENU hRoot)
{
	CLISTMENUITEM mi = {0};
	char service[64];

	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_ICONFROMICOLIB | CMIF_ROOTHANDLE;
	mi.hParentMenu = hRoot;

	// Import from server item
	mir_snprintf(service, sizeof(service), GGS_IMPORT_SERVER, GG_PROTO);
	CreateProtoServiceFunction(service, gg_import_server, gg);
	mi.position = 2000500001;
	mi.icolibItem = GetIconHandle(IDI_IMPORT_SERVER);
	mi.pszName = LPGEN("Import List From &Server");
	mi.pszService = service;
 	gg->hMainMenu[2] = (HANDLE)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) &mi);

	// Import from textfile
	mir_snprintf(service, sizeof(service), GGS_IMPORT_TEXT, GG_PROTO);
	CreateProtoServiceFunction(service, gg_import_text, gg);
	mi.position = 2000500002;
	mi.icolibItem = GetIconHandle(IDI_IMPORT_TEXT);
	mi.pszName = LPGEN("Import List From &Text File...");
	mi.pszService = service;
	gg->hMainMenu[3] = (HANDLE)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) &mi);

	// Remove from server
	mir_snprintf(service, sizeof(service), GGS_REMOVE_SERVER, GG_PROTO);
	CreateProtoServiceFunction(service, gg_remove_server, gg);
	mi.position = 2000500003;
	mi.icolibItem = GetIconHandle(IDI_REMOVE_SERVER);
	mi.pszName = LPGEN("&Remove List From Server");
	mi.pszService = service;
	gg->hMainMenu[4] = (HANDLE)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) &mi);

	// Export to server
	mir_snprintf(service, sizeof(service), GGS_EXPORT_SERVER, GG_PROTO);
	CreateProtoServiceFunction(service, gg_export_server, gg);
	mi.position = 2005000001;
	mi.icolibItem = GetIconHandle(IDI_EXPORT_SERVER);
	mi.pszName = LPGEN("Export List To &Server");
	mi.pszService = service;
	gg->hMainMenu[5] = (HANDLE)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) &mi);

	// Export to textfile
	mir_snprintf(service, sizeof(service), GGS_EXPORT_TEXT, GG_PROTO);
	CreateProtoServiceFunction(service, gg_export_text, gg);
	mi.position = 2005000002;
	mi.icolibItem = GetIconHandle(IDI_EXPORT_TEXT);
	mi.pszName = LPGEN("Export List To &Text File...");
	mi.pszService = service;
	gg->hMainMenu[6] = (HANDLE)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) &mi);
}
