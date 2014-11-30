#include "gmail.h"

void BuildList(void)
{
	DBVARIANT dbv;

	acc_num = 0;
	for (MCONTACT hContact = db_find_first(pluginName); hContact; hContact = db_find_next(hContact, pluginName)) {
		if (!db_get_s(hContact, pluginName, "name", &dbv)) {
			acc_num++;
			acc = (Account *)realloc(acc, acc_num * sizeof(Account));
			memset(&acc[acc_num-1], 0, sizeof(Account));
			acc[acc_num-1].hContact = hContact;
			mir_strcpy(acc[acc_num-1].name, dbv.pszVal);
			CallService(MS_IGNORE_IGNORE, hContact, IGNOREEVENT_USERONLINE);
			db_free(&dbv);
			
			if (!db_get_s(hContact, pluginName, "Password", &dbv)) {
				mir_strcpy(acc[acc_num-1].pass, dbv.pszVal);
				db_free(&dbv);
			}
		}
	}

	for (int i = 0; i < acc_num; i++) {
		char *tail = strchr(acc[i].name, '@');
		if (tail && mir_strcmp(tail + 1, "gmail.com") != 0)
			mir_strcpy(acc[i].hosted, tail + 1);
		acc[i].IsChecking = FALSE;
	}
}

BOOL GetBrowser(char *str)
{
	HKEY hKey = NULL;
	char *strKey;
	char strIE[] = "Applications\\iexplore.exe\\shell\\open\\command";
	char strDefault[] = "https\\shell\\open\\command";
	DBVARIANT dbv;

	if (opt.OpenUsePrg == 1)
		strKey = strIE;
	else if (opt.OpenUsePrg == 0)
		strKey = strDefault;
	else if (!db_get_s(NULL, pluginName, "OpenUsePrgPath", &dbv)) {
		mir_strcpy(str, dbv.pszVal);
		db_free(&dbv);
		return FALSE;
	}

	// Open the registry
	if (RegOpenKeyExA(HKEY_CLASSES_ROOT, strKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		// Data size
		DWORD cbData = 0;
		// Get the default value
		if (RegQueryValueExA(hKey, NULL, NULL, NULL, NULL, &cbData) == ERROR_SUCCESS && cbData > 0) {
			if (RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)str, &cbData) == ERROR_SUCCESS) {
				if ((strKey = strstr(str, "%1")) != NULL)
					*(strKey--) = '\0';
				if ((strKey = strstr(str, "-")) != NULL)
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