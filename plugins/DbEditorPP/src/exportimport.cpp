#include "stdafx.h"


TCHAR *GetFilter()
{
	static TCHAR filter[MAX_PATH];
	mir_sntprintf(filter, _T("%s%c*.ini%c%s%c*.*%c"), TranslateT("INI Files"), 0, 0, TranslateT("All Files"), 0, 0);
	return filter;
}


int Openfile(TCHAR *outputFile, const char *module, int maxlen)
{
	OPENFILENAME ofn = { 0 };
	TCHAR filename[MAX_PATH];

	if (module) {
		int n = 0;
		mir_tstrncpy(filename, _A2T(module), _countof(filename));

		while (filename[n]) {
			switch (filename[n]) {
			case _T('*'):
			case _T(':'):
			case _T('/'):
			case _T('?'):
			case _T('|'):
			case _T('\\'):
				filename[n] = _T('_');
				break;
			}
			n++;
		}
	} 
	else
		filename[0] = 0;


	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = filename;
	ofn.lpstrFilter = GetFilter();
	ofn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrTitle = TranslateT("Export to file");
	ofn.nMaxFile = maxlen;
	ofn.lpstrDefExt = _T("ini");
	if (!GetSaveFileName(&ofn))
		return 0;

	mir_tstrncpy(outputFile, filename, maxlen);
	return 1;
}

void exportModule(MCONTACT hContact, const char *module, FILE *file)
{
	char tmp[32];
	ModuleSettingLL settinglist;
	ModSetLinkLinkItem *setting;

	if (IsModuleEmpty(hContact, module) || !EnumSettings(hContact, module, &settinglist))
		return;

	// print the module header..
	fprintf(file, "\n[%s]", module);
	setting = settinglist.first;
	while (setting) {
		DBVARIANT dbv;
		
		if (!db_get_s(hContact, module, setting->name, &dbv, 0)) {

			switch (dbv.type) {
			case DBVT_BYTE:
				fprintf(file, "\n%s=b%s", setting->name, _ultoa(dbv.bVal, tmp, 10));
				break;
			case DBVT_WORD:
				fprintf(file, "\n%s=w%s", setting->name, _ultoa(dbv.wVal, tmp, 10));
				break;
			case DBVT_DWORD:
				fprintf(file, "\n%s=d%s", setting->name, _ultoa(dbv.dVal, tmp, 10));
				break;
			case DBVT_BLOB:
			{
				ptrA data(StringFromBlob(dbv.pbVal, dbv.cpbVal));
				fprintf(file, "\n%s=n%s", setting->name, data);
				break;
			}
			case DBVT_WCHAR:
			case DBVT_ASCIIZ:
			case DBVT_UTF8:
			{   
				char *str = (dbv.type == DBVT_WCHAR) ? mir_utf8encodeW(dbv.pwszVal) : dbv.pszVal;

				if (strchr(str, '\r')) {
					CMStringA end = str;
					end.Replace("\\", "\\\\");
					end.Replace("\r", "\\r");
					end.Replace("\n", "\\n");
					fprintf(file, "\n%s=g%s", setting->name, end.c_str());
				} else {
					fprintf(file, "\n%s=%c", setting->name, (dbv.type == DBVT_ASCIIZ) ? 's' : 'u');
					fputs(str, file);
				}
				if (str != dbv.pszVal) 
					mir_free(str);
				break;
			}
			} // switch
			db_free(&dbv);
		}
		setting = (ModSetLinkLinkItem *)setting->next;
	}
	FreeModuleSettingLL(&settinglist);
}


char* NickFromHContact(MCONTACT hContact)
{
	static char nick[NAME_SIZE] = "";

	if (hContact) {
		char szProto[FLD_SIZE], name[NAME_SIZE];
		int loaded = 0;
		szProto[0] = 0;

		if (!db_get_static(hContact, "Protocol", "p", szProto, _countof(szProto)))
			loaded = Proto_GetAccount(szProto) ? 1 : 0;

		if (!szProto[0] || db_get_static(hContact, szProto, "Nick", name, _countof(name))) 
			mir_strncpy(name, "(UNKNOWN)", _countof(name));
		
		if (!loaded) {
			if (szProto[0])
				mir_snprintf(nick, _countof(nick), "%s (%s)", name, szProto);
			else 
				mir_strncpy(nick, name, _countof(nick));
		}
		else {
			char *uid = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
			if ((INT_PTR)uid != CALLSERVICE_NOTFOUND && uid) {
				char szUID[FLD_SIZE];
				GetValueA(hContact, szProto, uid, szUID, _countof(szUID));
				mir_snprintf(nick, _countof(nick), "%s *(%s)*<%s>*{%s}*", name, szProto, uid, szUID);
			}
			else 
				mir_snprintf(nick, _countof(nick), "%s (%s)", name, szProto);
		}
	}

	return nick;
}

// hContact == -1 export entire db. module == NULL export entire contact.
// hContact == -1, module == "" - all contacts
void exportDB(MCONTACT hContact, const char *module)
{
	ModSetLinkLinkItem *mod;

	// enum all the modules
	ModuleSettingLL modlist;
	if (!EnumModules(&modlist))
		return;

	TCHAR fileName[MAX_PATH];

	if (Openfile(fileName, (hContact == INVALID_CONTACT_ID) ? NULL : module, MAX_PATH)) {
		FILE *file = _tfopen(fileName, _T("wt"));
		if (!file) {
			msg(TranslateT("Couldn't open file for writing"));
			return;
		}

		SetCursor(LoadCursor(NULL, IDC_WAIT));

		// exporting entire db
		if (hContact == INVALID_CONTACT_ID) {
			hContact = NULL;

			if (module == NULL) {
				fprintf(file, "SETTINGS:\n");
				mod = modlist.first;
				while (mod) {
					if (IsModuleEmpty(hContact, mod->name)) {
						mod = (ModSetLinkLinkItem *)mod->next;
						continue;
					}
					exportModule(hContact, mod->name, file);
					mod = (ModSetLinkLinkItem *)mod->next;
					if (mod)
						fprintf(file, "\n");
				}
			}
			else {
				if (*module == 0)
					module = NULL; // reset module for all contacts export
			}

			hContact = db_find_first();
			if (hContact)
				fprintf(file, "\n\n");

			while (hContact) {
				
				if (ApplyProtoFilter(hContact)) {
					hContact = db_find_next(hContact);
					continue;
				}

				fprintf(file, "CONTACT: %s\n", NickFromHContact(hContact));

				if (module == NULL) // export all modules
				{
					mod = modlist.first;
					while (mod) {
						if (IsModuleEmpty(hContact, mod->name)) {
							mod = (ModSetLinkLinkItem *)mod->next;
							continue;
						}
						exportModule(hContact, mod->name, file);
						mod = (ModSetLinkLinkItem *)mod->next;
						if (mod)
							fprintf(file, "\n");
					}
				}
				else // export module
				{
					exportModule(hContact, module, file);
				}
				hContact = db_find_next(hContact);
			}
		}
		// exporting a contact
		else {
			if (!module) { // exporting every module
				if (hContact)
					fprintf(file, "CONTACT: %s\n", NickFromHContact(hContact));
				else
					fprintf(file, "SETTINGS:\n");

				mod = modlist.first;
				while (mod) {
					if (IsModuleEmpty(hContact, mod->name)) {
						mod = (ModSetLinkLinkItem *)mod->next;
						continue;
					}
					exportModule(hContact, mod->name, file);
					mod = (ModSetLinkLinkItem *)mod->next;
					if (mod)
						fprintf(file, "\n");
				}
			}
			else {
				if (hContact)
					fprintf(file, "FROM CONTACT: %s\n", NickFromHContact(hContact));
				else
					fprintf(file, "SETTINGS:\n");

				exportModule(hContact, module, file);
			}
		}
		fclose(file);

		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}

	FreeModuleSettingLL(&modlist);
}

MCONTACT CheckNewContact(const char *myProto, const char *uid, const char *myName)
{
	char szProto[FLD_SIZE], szName[NAME_SIZE];

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		if (!db_get_static(hContact, "Protocol", "p", szProto, _countof(szProto)))
			if (!mir_strcmp(szProto, myProto))
				if (GetValueA(hContact, szProto, uid, szName, _countof(szName)) && !mir_strcmp(szName, myName))
					return hContact;

	return INVALID_CONTACT_ID;
}


void importSettings(MCONTACT hContact, char *utf8)
{
	char module[FLD_SIZE] = "", setting[FLD_SIZE] = "", *end;
	int i = 0, value, type;
	char *importstring = utf8;
	char uid[FLD_SIZE], szUID[FLD_SIZE], szProto[FLD_SIZE];

	importstring = strtok(importstring, "\n");

	SetCursor(LoadCursor(NULL, IDC_WAIT));

	while (importstring != NULL) {
		i = 0;
		rtrim(importstring);
		if (importstring[i] == '\0') {
			importstring = strtok(NULL, "\n");
			continue;
		}

		if (!strncmp(&importstring[i], "SETTINGS:", mir_strlen("SETTINGS:"))) {
			importstring = strtok(NULL, "\n");
			continue;
		}

		if (!strncmp(&importstring[i], "CONTACT:", mir_strlen("CONTACT:"))) {
			hContact = INVALID_CONTACT_ID;

			i = i + (int)mir_strlen("CONTACT:");
			int len = (int)mir_strlen(&importstring[i]);

			if (len > 10) {
				uid[0] = 0; szUID[0] = 0, szProto[0] = 0;

				char *p1 = strrchr(&importstring[i], '>*{');
				char *p2 = strrchr(&importstring[i], '}*');

				if (p1 && p2 && p1 + 3 < p2 && p2 - p1 < _countof(szUID)) {
					strncpy(szUID, p1 + 1, p2 - p1 - 2);

					p1 = strrchr(&importstring[i], ')*<');
					p2 = strrchr(&importstring[i], '>*{');

					if (p1 && p2 && p1 + 3 < p2 && p2 - p1 < _countof(uid)) {
						strncpy(uid, p1 + 1, p2 - p1 - 3);

						p1 = strrchr(&importstring[i], ' *(');
						p2 = strrchr(&importstring[i], ')*<');

						if (p1 && p2 && p1 + 3 < p2 && p2 - p1 < _countof(szProto)) {
							strncpy(szProto, p1 + 1, p2 - p1 - 3);

							char *protouid = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
							if ((INT_PTR)protouid != CALLSERVICE_NOTFOUND) {
								if (!mir_strcmp(protouid, uid))
									hContact = CheckNewContact(szProto, uid, szUID);
							}
							else hContact = CheckNewContact(szProto, uid, szUID);
						}
					}
				}
			}

			if (hContact == INVALID_CONTACT_ID) {
				MCONTACT temp = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
				if (temp)
					hContact = temp;
			}
		}
		else if (importstring[i] == '[' && !strchr(&importstring[i + 1], '=')) { // get the module
			if (end = strpbrk(&importstring[i + 1], "]")) {
				*end = '\0';
				mir_strcpy(module, &importstring[i + 1]);
			}
		}
		else if (importstring[i] == '-' && importstring[i + 1] == '[' && !strchr(&importstring[i + 2], '=')) { // get the module
			if (end = strpbrk(&importstring[i + 2], "]")) {
				*end = '\0';
				mir_strcpy(module, &importstring[i + 2]);
				deleteModule(hContact, module, 0);
			}
		}
		else if (strstr(&importstring[i], "=") && module[0]) { // get the setting
			if (end = strpbrk(&importstring[i + 1], "=")) {
				*end = '\0';
				mir_strcpy(setting, &importstring[i]);

				// get the type
				type = *(end + 1);
				if (mir_strcmp(module, "CList") == 0 && mir_strcmp(setting, "Group") == 0) {
					ptrA GroupName(mir_utf8decodeA(end + 2));
					if (!GroupName)
						continue;

					HANDLE GroupHandle = (HANDLE)CallService(MS_CLIST_GROUPEXISTS, 0, LPARAM(GroupName));
					if (GroupHandle == 0) {
						GroupHandle = (HANDLE)CallService(MS_CLIST_GROUPCREATE, 0, (LPARAM)GroupName);
						if (GroupHandle) {
							CallService(MS_CLUI_GROUPADDED, (WPARAM)GroupHandle, 0);
							CallService(MS_CLIST_GROUPSETEXPANDED, (WPARAM)GroupHandle, 1);
						}
					}
				}

				switch (type) {
				case 'b':
				case 'B':
					if (sscanf((end + 2), "%u", &value) == 1)
						db_set_b(hContact, module, setting, (BYTE)value);
					break;
				case 'w':
				case 'W':
					if (sscanf((end + 2), "%u", &value) == 1)
						db_set_w(hContact, module, setting, (WORD)value);
					break;
				case 'd':
				case 'D':
					if (sscanf((end + 2), "%u", &value) == 1)
						db_set_dw(hContact, module, setting, (DWORD)value);
					break;
				case 's':
				case 'S':
					db_set_s(hContact, module, setting, (end + 2));
					break;
				case 'g':
				case 'G':
					for (char *pstr = end + 2; *pstr; pstr++) {
						if (*pstr == '\\') {
							switch (pstr[1]) {
								case 'n': *pstr = '\n'; break;
								case 't': *pstr = '\t'; break;
								case 'r': *pstr = '\r'; break;
								default:  *pstr = pstr[1]; break;
							}
							memmove(pstr + 1, pstr + 2, mir_strlen(pstr + 2) + 1);
						}
					}
				case 'u':
				case 'U':
					db_set_utf(hContact, module, setting, (end + 2));
					break;
				case 'l':
				case 'L':
				case '-':
					db_unset(hContact, module, setting);
					break;
				case 'n':
				case 'N':
					WriteBlobFromString(hContact, module, setting, (end + 2), (int)mir_strlen((end + 2)));
					break;
				}
			}
		}
		importstring = strtok(NULL, "\n");
	}
	SetCursor(LoadCursor(NULL, IDC_ARROW));
}


INT_PTR CALLBACK ImportDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		TranslateDialogDefault(hwnd);
		SendDlgItemMessage(hwnd, IDC_TEXT, EM_LIMITTEXT, (WPARAM)0x7FFFFFFF, 0);

		TCHAR name[NAME_SIZE], msg[MSG_SIZE];

		GetContactName((MCONTACT)lParam, NULL, name, _countof(name));

		mir_sntprintf(msg, TranslateT("Import to \"%s\""), name);
		SetWindowText(hwnd, msg);

		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDCANCEL:
			DestroyWindow(hwnd);
			break;

		case IDOK:
			MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT));
			if (length) {
				TCHAR *data = (TCHAR*)mir_alloc((length + 1)*sizeof(TCHAR));
				GetDlgItemText(hwnd, IDC_TEXT, data, length + 1);
				importSettings(hContact, T2Utf(data));
				mir_free(data);
				refreshTree(1);
			}
		}
		break;
	}
	return 0;
}


void ImportSettingsMenuItem(MCONTACT hContact)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_IMPORT), hwnd2mainWindow, ImportDlgProc, hContact);
}


int Openfile2Import(TCHAR *outputFiles, int maxlen)
{

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = GetFilter();
	ofn.lpstrFile = outputFiles;
	ofn.nMaxFile = maxlen;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.Flags = OFN_SHAREAWARE | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	ofn.lpstrTitle = TranslateT("Import from files");
	if (!GetOpenFileName(&ofn))
		return 0;

	return ofn.nFileOffset;
}

BOOL Exists(LPCTSTR strName)
{
	return GetFileAttributes(strName) != INVALID_FILE_ATTRIBUTES;
}

void ImportSettingsFromFileMenuItem(MCONTACT hContact, const char *FilePath)
{
	TCHAR szFileNames[MAX_PATH * 10];
	TCHAR szPath[MAX_PATH] = { 0 };
	TCHAR szFile[MAX_PATH];

	DWORD offset = 0;

	mir_tstrcpy(szFileNames, _T(""));	

	if (!FilePath)
		offset = Openfile2Import(szFileNames, _countof(szFileNames));
	else {
	    _A2T tmp(FilePath);
		if (GetFileAttributes(tmp) != INVALID_FILE_ATTRIBUTES)
			mir_tstrncpy(szFileNames, tmp, _countof(szFileNames));
	}

	int index = 0;
	if (mir_tstrcmp(szFileNames, _T(""))) {
		if ((DWORD)mir_tstrlen(szFileNames) < offset) {
			index += offset;
			mir_tstrncpy(szPath, szFileNames, offset);
			mir_tstrcat(szPath, _T("\\"));
		}

		while (szFileNames[index]) {
			mir_tstrcpy(szFile, szPath);
			mir_tstrcat(szFile, &szFileNames[index]);
			index += (int)mir_tstrlen(&szFileNames[index]) + 1;

			HANDLE hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (hFile != INVALID_HANDLE_VALUE) {
				if (GetFileSize(hFile, NULL) > 0) {
					HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
					if (hMap) {
						char *pFile = (char*)MapViewOfFile(hMap, FILE_MAP_COPY, 0, 0, 0);
						if (pFile) {
							importSettings(hContact, pFile);
							UnmapViewOfFile(pFile);
						}
						CloseHandle(hMap);
					}
				}
				CloseHandle(hFile);
			}
			else break;
		}
		if (mir_strcmp(FilePath, "") == 0)
			refreshTree(1);
	}
}
