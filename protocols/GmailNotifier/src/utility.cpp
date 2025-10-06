#include "stdafx.h"

void BuildList(void)
{
	g_accs.destroy();

	for (auto &hContact : Contacts(MODULENAME)) {
		Account *p = new Account(hContact);
		g_accs.insert(p);
	}
}

BOOL GetBrowser(char *str)
{
	HKEY hKey = nullptr;
	char *strKey;
	char strIE[] = "Applications\\iexplore.exe\\shell\\open\\command";
	char strDefault[] = "https\\shell\\open\\command";
	DBVARIANT dbv;

	if (g_plugin.OpenUsePrg == 1)
		strKey = strIE;
	else if (g_plugin.OpenUsePrg == 0)
		strKey = strDefault;
	else {
		if (!g_plugin.getString("OpenUsePrgPath", &dbv)) {
			mir_strcpy(str, dbv.pszVal);
			db_free(&dbv);
		}
		else *str = 0;
		return FALSE;
	}

	// Open the registry
	if (RegOpenKeyExA(HKEY_CLASSES_ROOT, strKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		// Data size
		DWORD cbData = 0;
		// Get the default value
		if (RegQueryValueExA(hKey, nullptr, nullptr, nullptr, nullptr, &cbData) == ERROR_SUCCESS && cbData > 0) {
			if (RegQueryValueExA(hKey, nullptr, nullptr, nullptr, (LPBYTE)str, &cbData) == ERROR_SUCCESS) {
				if ((strKey = strstr(str, "%1")) != nullptr)
					*(strKey--) = '\0';
				if ((strKey = strstr(str, "-")) != nullptr)
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
	for (auto &it : g_accs)
		if (it->hContact == hContact)
			return it;

	return nullptr;
}
