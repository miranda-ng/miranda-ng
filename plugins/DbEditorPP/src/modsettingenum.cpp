#include "headers.h"

void FreeModuleSettingLL(ModuleSettingLL* msll)
{
	if (msll == NULL)
		return;

	ModSetLinkLinkItem *item = msll->first;
	ModSetLinkLinkItem *temp;

	while (item) {
		mir_free(item->name);
		temp = item;
		item = (ModSetLinkLinkItem *)item->next;
		mir_free(temp);
	}

	msll->first = 0;
	msll->last = 0;
}

int enumModulesSettingsProc(const char *szName, DWORD, LPARAM lParam)
{
	ModuleSettingLL *msll = (ModuleSettingLL *)lParam;
	if (!msll->first) {
		msll->first = (ModSetLinkLinkItem *)mir_alloc(sizeof(ModSetLinkLinkItem));
		if (!msll->first)
			return 1;

		msll->first->name = mir_tstrdup(szName);
		msll->first->next = 0;
		msll->last = msll->first;
	}
	else {
		ModSetLinkLinkItem *item = (ModSetLinkLinkItem *)mir_alloc(sizeof(ModSetLinkLinkItem));
		if (!item)
			return 1;

		msll->last->next = item;
		msll->last = item;
		item->name = mir_tstrdup(szName);
		item->next = 0;
	}
	return 0;
}

int EnumModules(ModuleSettingLL *msll) // 1 = success, 0 = fail
{
	msll->first = 0;
	msll->last = 0;
	return !CallService(MS_DB_MODULES_ENUM, (WPARAM)msll, (WPARAM)enumModulesSettingsProc);
}

int enumSettingsProc(const char *szSetting, LPARAM lParam)
{
	return enumModulesSettingsProc(szSetting, 0, lParam);
}

int EnumSettings(MCONTACT hContact, char* module, ModuleSettingLL *msll)
{
	DBCONTACTENUMSETTINGS dbces;
	// enum all setting the contact has for the module
	dbces.pfnEnumProc = enumSettingsProc;
	dbces.szModule = module;
	dbces.lParam = (LPARAM)msll;
	msll->first = 0;
	msll->last = 0;
	return !CallService(MS_DB_CONTACT_ENUMSETTINGS, hContact, (LPARAM)&dbces);
}

int CheckIfModuleIsEmptyProc(const char *, LPARAM)
{
	return 1;
}

int IsModuleEmpty(MCONTACT hContact, char* szModule)
{
	DBCONTACTENUMSETTINGS dbces;
	dbces.pfnEnumProc = CheckIfModuleIsEmptyProc;
	dbces.szModule = szModule;
	int retVal = CallService(MS_DB_CONTACT_ENUMSETTINGS, hContact, (LPARAM)&dbces);
	if (retVal >= 0)
		return 0;
	return 1;
}
