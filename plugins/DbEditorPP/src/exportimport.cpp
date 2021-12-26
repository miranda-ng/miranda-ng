#include "stdafx.h"

wchar_t *GetFilter()
{
	static wchar_t filter[MAX_PATH];
	mir_snwprintf(filter, L"%s%c*.ini%c%s%c*.*%c", TranslateT("INI Files"), 0, 0, TranslateT("All Files"), 0, 0);
	return filter;
}

int Openfile(wchar_t *outputFile, const char *module, int maxlen)
{
	wchar_t filename[MAX_PATH];

	if (module) {
		int n = 0;
		mir_wstrncpy(filename, _A2T(module), _countof(filename));

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
	else filename[0] = 0;

	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = filename;
	ofn.lpstrFilter = GetFilter();
	ofn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrTitle = TranslateT("Export to file");
	ofn.nMaxFile = maxlen;
	ofn.lpstrDefExt = L"ini";
	if (!GetSaveFileName(&ofn))
		return 0;

	mir_wstrncpy(outputFile, filename, maxlen);
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
				fprintf(file, "\n%s=n%s", setting->name, (char*)ptrA(StringFromBlob(dbv.pbVal, dbv.cpbVal)));
				break;
			case DBVT_WCHAR:
			case DBVT_ASCIIZ:
			case DBVT_UTF8:
				char *str = (dbv.type == DBVT_WCHAR) ? mir_utf8encodeW(dbv.pwszVal) : dbv.pszVal;

				if (strchr(str, '\r')) {
					CMStringA end = str;
					end.Replace("\\", "\\\\");
					end.Replace("\r", "\\r");
					end.Replace("\n", "\\n");
					fprintf(file, "\n%s=g%s", setting->name, end.c_str());
				}
				else {
					fprintf(file, "\n%s=%c", setting->name, (dbv.type == DBVT_ASCIIZ) ? 's' : 'u');
					fputs(str, file);
				}
				if (str != dbv.pszVal)
					mir_free(str);
				break;
			}
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
				mir_snprintf(nick, "%s (%s)", name, szProto);
			else
				mir_strncpy(nick, name, _countof(nick));
		}
		else {
			const char *uid = Proto_GetUniqueId(szProto);
			if (uid) {
				char szUID[FLD_SIZE];
				GetValueA(hContact, szProto, uid, szUID, _countof(szUID));
				mir_snprintf(nick, "%s *(%s)*<%s>*{%s}*", name, szProto, uid, szUID);
			}
			else mir_snprintf(nick, "%s (%s)", name, szProto);
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

	wchar_t fileName[MAX_PATH];

	if (Openfile(fileName, (hContact == INVALID_CONTACT_ID) ? nullptr : module, MAX_PATH)) {
		FILE *file = _wfopen(fileName, L"wt");
		if (!file) {
			g_pMainWindow->msg(TranslateT("Couldn't open file for writing"));
			return;
		}

		SetCursor(LoadCursor(nullptr, IDC_WAIT));

		// exporting entire db
		if (hContact == INVALID_CONTACT_ID) {
			if (module == nullptr) {
				fprintf(file, "SETTINGS:\n");
				mod = modlist.first;
				while (mod) {
					if (IsModuleEmpty(NULL, mod->name)) {
						mod = (ModSetLinkLinkItem *)mod->next;
						continue;
					}
					exportModule(NULL, mod->name, file);
					mod = (ModSetLinkLinkItem *)mod->next;
					if (mod)
						fprintf(file, "\n");
				}
			}
			else {
				if (*module == 0)
					module = nullptr; // reset module for all contacts export
			}

			fprintf(file, "\n\n");

			for (auto &cc : Contacts()) {
				if (ApplyProtoFilter(cc))
					continue;

				fprintf(file, "CONTACT: %s\n", NickFromHContact(cc));

				if (module == nullptr) // export all modules
				{
					mod = modlist.first;
					while (mod) {
						if (IsModuleEmpty(cc, mod->name)) {
							mod = (ModSetLinkLinkItem *)mod->next;
							continue;
						}
						exportModule(cc, mod->name, file);
						mod = (ModSetLinkLinkItem *)mod->next;
						if (mod)
							fprintf(file, "\n");
					}
				}
				else // export module
				{
					exportModule(cc, module, file);
				}
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

		SetCursor(LoadCursor(nullptr, IDC_ARROW));
	}

	FreeModuleSettingLL(&modlist);
}

MCONTACT CheckNewContact(const char *myProto, const char *uid, const char *myName)
{
	char szProto[FLD_SIZE], szName[NAME_SIZE];

	for (auto &hContact : Contacts())
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

	SetCursor(LoadCursor(nullptr, IDC_WAIT));

	while (importstring != nullptr) {
		i = 0;
		rtrim(importstring);
		if (importstring[i] == '\0') {
			importstring = strtok(nullptr, "\n");
			continue;
		}

		if (!strncmp(&importstring[i], "SETTINGS:", mir_strlen("SETTINGS:"))) {
			importstring = strtok(nullptr, "\n");
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
							hContact = CheckNewContact(szProto, uid, szUID);
						}
					}
				}
			}

			if (hContact == INVALID_CONTACT_ID) {
				MCONTACT temp = db_add_contact();
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
				deleteModule(0, hContact, module, 0);
			}
		}
		else if (strchr(&importstring[i], '=') && module[0]) { // get the setting
			if (end = strpbrk(&importstring[i + 1], "=")) {
				*end = '\0';
				mir_strcpy(setting, &importstring[i]);

				// get the type
				type = *(end + 1);
				if (mir_strcmp(module, "CList") == 0 && mir_strcmp(setting, "Group") == 0) {
					ptrW GroupName(mir_utf8decodeW(end + 2));
					if (!GroupName)
						continue;

					MGROUP GroupHandle = Clist_GroupCreate(0, GroupName);
					Clist_GroupSetExpanded(GroupHandle, true);
				}

				switch (type) {
				case 'b':
				case 'B':
					if (sscanf(end + 2, "%u", &value) == 1)
						db_set_b(hContact, module, setting, (uint8_t)value);
					break;
				case 'w':
				case 'W':
					if (sscanf(end + 2, "%u", &value) == 1)
						db_set_w(hContact, module, setting, (uint16_t)value);
					break;
				case 'd':
				case 'D':
					if (sscanf(end + 2, "%u", &value) == 1)
						db_set_dw(hContact, module, setting, (uint32_t)value);
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
					__fallthrough; // write it to database

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
		importstring = strtok(nullptr, "\n");
	}
	SetCursor(LoadCursor(nullptr, IDC_ARROW));
}

INT_PTR CALLBACK ImportDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		TranslateDialogDefault(hwnd);
		SendDlgItemMessage(hwnd, IDC_TEXT, EM_LIMITTEXT, (WPARAM)0x7FFFFFFF, 0);

		wchar_t name[NAME_SIZE], msg[MSG_SIZE];

		GetContactName((MCONTACT)lParam, nullptr, name, _countof(name));

		mir_snwprintf(msg, TranslateT("Import to \"%s\""), name);
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
				wchar_t *data = (wchar_t*)mir_alloc((length + 1) * sizeof(wchar_t));
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

void CMainDlg::ImportSettingsMenuItem(MCONTACT hContact)
{
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_IMPORT), m_hwnd, ImportDlgProc, hContact);
}

int Openfile2Import(wchar_t *outputFiles, int maxlen)
{
	OPENFILENAME ofn = {};
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
	wchar_t szFileNames[MAX_PATH * 10];
	wchar_t szPath[MAX_PATH] = {};
	wchar_t szFile[MAX_PATH];

	uint32_t offset = 0;

	mir_wstrcpy(szFileNames, L"");

	if (!FilePath)
		offset = Openfile2Import(szFileNames, _countof(szFileNames));
	else {
		_A2T tmp(FilePath);
		if (GetFileAttributes(tmp) != INVALID_FILE_ATTRIBUTES)
			mir_wstrncpy(szFileNames, tmp, _countof(szFileNames));
	}

	int index = 0;
	if (mir_wstrcmp(szFileNames, L"")) {
		if ((uint32_t)mir_wstrlen(szFileNames) < offset) {
			index += offset;
			mir_wstrncpy(szPath, szFileNames, offset);
			mir_wstrcat(szPath, L"\\");
		}

		while (szFileNames[index]) {
			mir_wstrcpy(szFile, szPath);
			mir_wstrcat(szFile, &szFileNames[index]);
			index += (int)mir_wstrlen(&szFileNames[index]) + 1;

			HANDLE hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
			if (hFile != INVALID_HANDLE_VALUE) {
				if (GetFileSize(hFile, nullptr) > 0) {
					HANDLE hMap = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
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
