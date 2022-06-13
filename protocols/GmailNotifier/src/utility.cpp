#include "stdafx.h"

void BuildList(void)
{
	g_accs.destroy();

	for (auto &hContact : Contacts(MODULENAME)) {
		ptrA szName(g_plugin.getStringA(hContact, "name"));
		if (szName != nullptr) {
			Account *p = new Account;
			p->hContact = hContact;
			mir_strcpy(p->name, szName);
			Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);

			ptrA szPassword(g_plugin.getStringA(hContact, "Password"));
			if (szPassword != nullptr)
				mir_strcpy(p->pass, szPassword);
			g_accs.insert(p);
		}
	}

	for (auto &acc : g_accs) {
		char *tail = strchr(acc->name, '@');
		if (tail && mir_strcmp(tail + 1, "gmail.com") != 0)
			mir_strcpy(acc->hosted, tail + 1);
		acc->IsChecking = false;
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
