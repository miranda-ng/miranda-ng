#include "gmail.h"

void BuildList(void)
{
	DBVARIANT dbv;

	acc_num = 0;
	for (MCONTACT hContact = db_find_first(pluginName); hContact; hContact = db_find_next(hContact, pluginName)) {
		if (!db_get_ts(hContact, pluginName, "name", &dbv)) {
			acc_num++;
			acc = (Account *)realloc(acc, acc_num * sizeof(Account));
			memset(&acc[acc_num-1], 0, sizeof(Account));
			acc[acc_num-1].hContact = hContact;
			lstrcpy(acc[acc_num-1].name, dbv.ptszVal);
			CallService(MS_IGNORE_IGNORE, hContact, IGNOREEVENT_USERONLINE);
			db_free(&dbv);
			
			if (!db_get_ts(hContact, pluginName, "Password", &dbv)) {
				lstrcpy(acc[acc_num-1].pass, dbv.ptszVal);
				db_free(&dbv);
			}
		}
	}

	for (int i = 0; i < acc_num; i++) {
		TCHAR *tail = _tcschr(acc[i].name, '@');
		if (tail && lstrcmp(tail + 1, _T("gmail.com")) != 0)
			lstrcpy(acc[i].hosted, tail + 1);
		acc[i].IsChecking = FALSE;
	}
}

BOOL GetBrowser(TCHAR *str)
{
	HKEY hKey = NULL;
	TCHAR *strKey;
	TCHAR strIE[] = _T("Applications\\iexplore.exe\\shell\\open\\command");
	TCHAR strDefault[] = _T("https\\shell\\open\\command");
	DBVARIANT dbv;

	if (opt.OpenUsePrg == 1)
		strKey = strIE;
	else if (opt.OpenUsePrg == 0)
		strKey = strDefault;
	else if (!db_get_ts(NULL, pluginName, "OpenUsePrgPath", &dbv)) {
		lstrcpy(str, dbv.ptszVal);
		db_free(&dbv);
		return FALSE;
	}

	// Open the registry
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, strKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		// Data size
		DWORD cbData = 0;
		// Get the default value
		if (RegQueryValueEx(hKey, NULL, NULL, NULL, NULL, &cbData) == ERROR_SUCCESS && cbData > 0) {
			if (RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)str, &cbData) == ERROR_SUCCESS) {
				if ((strKey = _tcsstr(str, _T("%1"))) != NULL)
					*(strKey--) = '\0';
				if ((strKey = _tcsstr(str, _T("-"))) != NULL)
					*(strKey--) = '\0';
				RegCloseKey(hKey);
				return TRUE;
			}
		}
	}
	return FALSE;
}

Account* GetAccountByContact(MCONTACT hContact)
{
	for (int i = 0; i < acc_num; i++)
		if (acc[i].hContact == hContact)
			return &acc[i];

	return NULL;
}