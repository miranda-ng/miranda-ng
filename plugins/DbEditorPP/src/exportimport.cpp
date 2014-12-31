#include "headers.h"

int Mode;
HWND hwnd2importWindow;

static int Openfile(TCHAR *outputFile, const char *module)
{
	OPENFILENAME ofn = { 0 };
	char filename[MAX_PATH] = "";
	char filter[MAX_PATH];
	mir_snprintf(filter, SIZEOF(filter), "%s%c*.ini%c%s%c*.*%c", Translate("INI Files"), 0, 0, Translate("All Files"), 0, 0);
	char *title = Translate("Export to file");

	if (module) {
		int n = 0;
		mir_strncpy(filename, module, SIZEOF(filename));

		while (filename[n]) {
			switch (filename[n]) {
			case '*':
			case ':':
			case '/':
			case '?':
			case '|':
			case '\\':
				filename[n] = '_';
				break;
			}
			n++;
		}
	}

	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = filename;
	ofn.lpstrFilter = filter;
	ofn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrTitle = title;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "ini";
	if (!GetSaveFileName(&ofn))
		return 0;

	_tcsncpy_s(outputFile, MAX_PATH, filename, _TRUNCATE);
	return 1;
}

char* StrReplace(char* Search, char* Replace, char* Resource)
{
	int i = 0;
	int SearchLen = (int)_tcslen(Search);
	char* Work = mir_tstrdup(Replace);
	int ReplaceLen = (int)_tcslen(Work);

	char* Pointer = _tcsstr(Resource, Search);

	while (Pointer != NULL)
	{
		int PointerLen = (int)_tcslen(Pointer);
		int ResourceLen = (int)_tcslen(Resource);

		char* NewText = (char*)mir_calloc((ResourceLen - SearchLen + ReplaceLen + 1)*sizeof(char));

		_tcsncpy(NewText, Resource, ResourceLen - PointerLen);
		_tcscat(NewText, Work);
		_tcscat(NewText, Pointer + SearchLen);

		Resource = (char*)mir_realloc(Resource, (ResourceLen - SearchLen + ReplaceLen + 1)*sizeof(char));

		for (i = 0; i < (ResourceLen - SearchLen + ReplaceLen); i++)
			Resource[i] = NewText[i];
		Resource[i] = 0;
		mir_free(NewText);

		Pointer = _tcsstr(Resource + (ResourceLen - PointerLen + ReplaceLen), Search);
	}
	mir_free(Work);

	return Resource;
}

void exportModule(MCONTACT hContact, char *module, FILE *file)
{
	char tmp[32];
	ModuleSettingLL settinglist;
	ModSetLinkLinkItem *setting;

	EnumSettings(hContact, module, &settinglist);

	// print the module header..
	fprintf(file, "\n[%s]", module);
	setting = settinglist.first;
	while (setting) {
		DBVARIANT dbv;
		if (!GetSetting(hContact, module, setting->name, &dbv)) {
			switch (dbv.type) {
			case DBVT_BYTE:
				fprintf(file, "\n%s=b%s", setting->name, itoa(dbv.bVal, tmp, 10));
				db_free(&dbv);
				break;
			case DBVT_WORD:
				fprintf(file, "\n%s=w%s", setting->name, itoa(dbv.wVal, tmp, 10));
				db_free(&dbv);
				break;
			case DBVT_DWORD:
				fprintf(file, "\n%s=d%s", setting->name, itoa(dbv.dVal, tmp, 10));
				db_free(&dbv);
				break;
			case DBVT_ASCIIZ:
			case DBVT_UTF8:
				if (strchr(dbv.pszVal, '\r')) {
					char *end = StrReplace("\\", "\\\\", dbv.pszVal);
					end = StrReplace("\r", "\\r", end);
					end = StrReplace("\n", "\\n", end);
					fprintf(file, "\n%s=g%s", setting->name, end);
					break;
				}
				fprintf(file, "\n%s=%c", setting->name, (dbv.type == DBVT_UTF8) ? 'u' : 's');
				fputs(dbv.pszVal, file);
				db_free(&dbv);
				break;

			case DBVT_BLOB:
				char *data = NULL;
				if (!(data = (char*)mir_alloc(3 * (dbv.cpbVal + 1)*sizeof(char))))
					break;
				data[0] = '\0';
				for (int j = 0; j < dbv.cpbVal; j++) {
					char tmp[16];
					mir_snprintf(tmp, SIZEOF(tmp), "%02X ", (BYTE)dbv.pbVal[j]);
					strcat(data, tmp);
				}
				fprintf(file, "\n%s=n%s", setting->name, data);
				mir_free(data);
				db_free(&dbv);
				break;
			}
		}
		setting = (ModSetLinkLinkItem *)setting->next;
	}
	FreeModuleSettingLL(&settinglist);
}

char *NickFromHContact(MCONTACT hContact)
{
	static char nick[512] = "";

	if (hContact) {
		char szProto[256];
		int loaded = 0;

		if (GetValue(hContact, "Protocol", "p", szProto, SIZEOF(szProto)))
			loaded = IsProtocolLoaded(szProto);

		if (!szProto[0] || !loaded) {
			char name[256];

			if (szProto[0]) {
				if (GetValue(hContact, szProto, "Nick", name, SIZEOF(name)))
					mir_snprintf(nick, SIZEOF(nick), "%s (%s)", name, szProto);
				else
					mir_snprintf(nick, SIZEOF(nick), "(UNKNOWN) (%s)", szProto);
			}
			else
				mir_snprintf(nick, SIZEOF(nick), "(UNKNOWN)");
		}
		else {
			char *uid = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
			if ((INT_PTR)uid != CALLSERVICE_NOTFOUND && uid) {
				char szUID[256];
				GetValue(hContact, szProto, uid, szUID, SIZEOF(szUID));
				mir_snprintf(nick, SIZEOF(nick), "%s *(%s)*<%s>*{%s}*", (char*)GetContactName(hContact, szProto, 0), szProto, uid, szUID);
			}
			else mir_snprintf(nick, SIZEOF(nick), "%s (%s)", (char*)GetContactName(hContact, szProto, 0), szProto);
		}
	}

	return nick;
}
// hContact == -1 export entire db. module == NULL export entire contact.
// hContact == -1, module == "" - all contacts
void exportDB(MCONTACT hContact, char *module)
{
	ModSetLinkLinkItem *mod;

	// enum all the modules
	ModuleSettingLL modlist;
	if (!EnumModules(&modlist)) {
		msg(Translate("Error loading module list"), modFullname);
		return;
	}

	TCHAR fileName[MAX_PATH];
	if (Openfile(fileName, (hContact == INVALID_CONTACT_ID) ? NULL : module)) {
		FILE *file = _tfopen(fileName, _T("wt"));
		if (!file) {
			msg(Translate("Couldn't open file for writing"), modFullname);
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
				// filter
				if (Mode != MODE_ALL) {
					char szProto[256];
					int loaded = 0;

					if (GetValue(hContact, "Protocol", "p", szProto, SIZEOF(szProto)))
						loaded = IsProtocolLoaded(szProto);

					if ((loaded && Mode == MODE_UNLOADED) || (!loaded && Mode == MODE_LOADED)) {
						hContact = db_find_next(hContact);
						continue;
					}
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

MCONTACT CheckNewContact(char *myProto, char *uid, char *myName)
{
	char szProto[256], szName[256];

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		if (DBGetContactSettingStringStatic(hContact, "Protocol", "p", szProto, 256))
			if (!mir_strcmp(szProto, myProto))
				if (GetValue(hContact, szProto, uid, szName, SIZEOF(szName)) && !mir_strcmp(szName, myName))
					return hContact;

	return INVALID_CONTACT_ID;
}

void importSettings(MCONTACT hContact, char *importstring)
{
	char module[256] = "", setting[256] = "", *end;
	int i = 0, value, type;
	importstring = strtok(importstring, "\n");

	SetCursor(LoadCursor(NULL, IDC_WAIT));

	while (importstring != NULL) {
		i = 0;
		rtrim(importstring);
		if (importstring[i] == '\0') {
			importstring = strtok(NULL, "\n");
			continue;
		}
		else if (!strncmp(&importstring[i], "SETTINGS:", strlen("SETTINGS:"))) {
			importstring = strtok(NULL, "\n");
			continue;
		}
		else if (!strncmp(&importstring[i], "CONTACT:", strlen("CONTACT:"))) {
			int len;
			hContact = INVALID_CONTACT_ID;

			i = i + (int)strlen("CONTACT:");
			len = (int)strlen(&importstring[i]);

			if (len > 10) {
				char uid[256] = "", szUID[256] = "", szProto[512] = "";
				char *p1, *p2;

				p1 = strrchr(&importstring[i], '>*{');
				p2 = strrchr(&importstring[i], '}*');

				if (p1 && p2 && p1 + 3 < p2 && p2 - p1 < SIZEOF(szUID)) {
					strncpy(szUID, p1 + 1, p2 - p1 - 2);

					p1 = strrchr(&importstring[i], ')*<');
					p2 = strrchr(&importstring[i], '>*{');

					if (p1 && p2 && p1 + 3 < p2 && p2 - p1 < SIZEOF(uid)) {
						strncpy(uid, p1 + 1, p2 - p1 - 3);

						p1 = strrchr(&importstring[i], ' *(');
						p2 = strrchr(&importstring[i], ')*<');

						if (p1 && p2 && p1 + 3 < p2 && p2 - p1 < SIZEOF(szProto)) {
							char *protouid;
							strncpy(szProto, p1 + 1, p2 - p1 - 3);

							protouid = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
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
		else if (importstring[i] == '[' && !strchr(&importstring[i + 1], '='))// get the module
		{
			if (end = strpbrk(&importstring[i + 1], "]")) {
				*end = '\0';
				strcpy(module, &importstring[i + 1]);
			}
		}
		else if (importstring[i] == '-' && importstring[i + 1] == '[' &&
			!strchr(&importstring[i + 2], '='))// get the module
		{
			if (end = strpbrk(&importstring[i + 2], "]")) {
				*end = '\0';
				strcpy(module, &importstring[i + 2]);
				deleteModule(module, hContact, 1);
			}
		}
		else if (strstr(&importstring[i], "=") && module[0]) // get the setting
		{
			if (end = strpbrk(&importstring[i + 1], "=")) {
				*end = '\0';
				strcpy(setting, &importstring[i]);

				// get the type
				type = *(end + 1);
				if (mir_tstrcmp(module, "CList") == 0 && mir_tstrcmp(setting, "Group") == 0) {
					WCHAR* GroupName = mir_a2u(end + 2);
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
					mir_free(GroupName);
				}
				switch (type) {
				case 'b':
				case 'B':
					if (sscanf((end + 2), "%d", &value) == 1)
						db_set_b(hContact, module, setting, (BYTE)value);
					break;
				case 'w':
				case 'W':
					if (sscanf((end + 2), "%d", &value) == 1)
						db_set_w(hContact, module, setting, (WORD)value);
					break;
				case 'd':
				case 'D':
					if (sscanf((end + 2), "%d", &value) == 1)
						db_set_dw(hContact, module, setting, (DWORD)value);
					break;
				case 's':
				case 'S':
					db_set_s(hContact, module, setting, (end + 2));
					break;
				case 'g':
				case 'G':
				{
					char *pstr;
					for (pstr = end + 2; *pstr; pstr++) {
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
					WriteBlobFromString(hContact, module, setting, (end + 2), (int)strlen((end + 2)));
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
		hwnd2importWindow = hwnd;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		TranslateDialogDefault(hwnd);
		SendDlgItemMessage(hwnd, IDC_TEXT, EM_LIMITTEXT, (WPARAM)sizeof(TCHAR) * 0x7FFFFFFF, 0);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CRLF:
		{
			int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT));
			char *string = (char*)_alloca(length + 3);
			int Pos = 2;

			if (length) {
				int	Range = SendDlgItemMessage(hwnd, IDC_TEXT, EM_GETSEL, 0, 0);
				int Min = LOWORD(Range);
				int Max = HIWORD(Range);

				GetDlgItemText(hwnd, IDC_TEXT, string, length + 1);

				if (Min == -1)
					memcpy(string, crlf_string, sizeof(crlf_string));
				else if (Max == -1 || Max >= length)
					memcpy(&string[Min], crlf_string, sizeof(crlf_string));
				else if (Max - Min > 2) {
					memcpy(&string[Min], crlf_string, sizeof(crlf_string));
					memmove(&string[Min + 2], &string[Max], length - Max + 1);
				}
				else {
					memmove(&string[Min + 2], &string[Max], length - Max + 1);
					memcpy(&string[Min], crlf_string, sizeof(crlf_string));
				}

				if (Min) Pos += Min;
			}
			else memcpy(string, crlf_string, sizeof(crlf_string));

			SetDlgItemText(hwnd, IDC_TEXT, string);
			SendDlgItemMessage(hwnd, IDC_TEXT, EM_SETSEL, Pos, Pos);
			SetFocus(GetDlgItem(hwnd, IDC_TEXT));
		}
		break;

		case IDCANCEL:
			DestroyWindow(hwnd);
			hwnd2importWindow = 0;
			break;

		case IDOK:
			MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT));
			if (length) {
				char *string = (char*)_alloca(length + 1);
				if (!string) {
					msg(Translate("Couldn't allocate enough memory!"), modFullname);
					DestroyWindow(hwnd);
				}
				GetDlgItemText(hwnd, IDC_TEXT, string, length + 1);
				importSettings(hContact, string);
				refreshTree(1);
			}
		}
		break;
	}
	return 0;
}

void ImportSettingsMenuItem(MCONTACT hContact)
{
	if (hwnd2importWindow)
		DestroyWindow(hwnd2importWindow);

	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_IMPORT), 0, ImportDlgProc, hContact);
}

int Openfile2Import(char *outputFiles)
{
	OPENFILENAME ofn = { 0 };
	char filter[MAX_PATH];
	mir_snprintf(filter, SIZEOF(filter), "%s%c*.ini%c%s%c*.*%c", Translate("INI Files"), 0, 0, Translate("All Files"), 0, 0);
	char *title = Translate("Import from files");

	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.lpstrFilter = filter;
	ofn.hwndOwner = 0;
	ofn.lpstrFile = outputFiles;
	ofn.nMaxFile = MAX_PATH * 10;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	ofn.lpstrTitle = title;
	if (!GetOpenFileName(&ofn))
		return 0;

	return ofn.nFileOffset;
}

BOOL Exists(LPCTSTR strName)
{
	return GetFileAttributes(strName) != INVALID_FILE_ATTRIBUTES;
}

void ImportSettingsFromFileMenuItem(MCONTACT hContact, char* FilePath)
{
	char szFileNames[MAX_PATH * 10] = { 0 };
	char szPath[MAX_PATH] = "";
	char szFile[MAX_PATH];
	int index = 0;
	HANDLE hFile, hMap;
	PBYTE pFile = NULL;
	DWORD offset = 0;
	if (mir_tstrcmp(FilePath, "") == 0)
		offset = Openfile2Import(szFileNames);
	else {
		if (Exists(FilePath))
			mir_tstrcpy(szFileNames, FilePath);
		else
			mir_tstrcpy(szFileNames, "");
	}

	if (!mir_tstrcmp(szFileNames, "") == 0) {
		if ((DWORD)mir_strlen(szFileNames) < offset) {
			index += offset;
			strncpy(szPath, szFileNames, offset);
			strcat(szPath, "\\");
		}

		while (szFileNames[index]) {
			strcpy(szFile, szPath);
			strcat(szFile, &szFileNames[index]);
			index += (int)strlen(&szFileNames[index]) + 1;

			hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (hFile != INVALID_HANDLE_VALUE) {
				if (GetFileSize(hFile, NULL) > 0) {
					hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

					if (hMap) {
						pFile = (PBYTE)MapViewOfFile(hMap, FILE_MAP_COPY, 0, 0, 0);

						if (pFile) {
							importSettings(hContact, (char*)pFile);
							UnmapViewOfFile(pFile);
						}
						CloseHandle(hMap);
					}

				}
				CloseHandle(hFile);
			}
			else
				break;

		}
		if (mir_tstrcmp(FilePath, "") == 0)
			refreshTree(1);
	}
}
