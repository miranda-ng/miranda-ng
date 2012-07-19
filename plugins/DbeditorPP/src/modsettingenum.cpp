#include "headers.h"

void FreeModuleSettingLL(ModuleSettingLL* msll)
{
    if (msll)
    {

		struct ModSetLinkLinkItem *item = msll->first;
		struct ModSetLinkLinkItem *temp;

		while (item)
		{
			mir_free(item->name);
			temp = item;
			item = (struct ModSetLinkLinkItem *)item->next;
			mir_free(temp);
		}

		msll->first = 0;
		msll->last = 0;
	}
}

int enumModulesSettingsProc( const char *szName , DWORD ofsModuleName , LPARAM lParam)
{
	ModuleSettingLL *msll = (ModuleSettingLL *)lParam;
	if (!msll->first)
	{
		msll->first = (struct ModSetLinkLinkItem *)mir_alloc(sizeof(struct ModSetLinkLinkItem));
		if (!msll->first) return 1;
		msll->first->name = mir_tstrdup(szName);
		msll->first->next = 0;
		msll->last = msll->first;
	}
	else
	{
		struct ModSetLinkLinkItem *item = (struct ModSetLinkLinkItem *)mir_alloc(sizeof(struct ModSetLinkLinkItem));
		if (!item) return 1;
		msll->last->next = (BYTE*)item;
		msll->last = (struct ModSetLinkLinkItem *)item;
		item->name = mir_tstrdup(szName);
		item->next = 0;
	}
	return 0;
}

int EnumModules(ModuleSettingLL *msll) // 1 = success, 0 = fail
{
	msll->first = 0;
	msll->last = 0;
	return !CallService(MS_DB_MODULES_ENUM, (WPARAM)msll,(WPARAM)enumModulesSettingsProc);
}


int enumSettingsProc(const char *szSetting,LPARAM lParam)
{
	return enumModulesSettingsProc(szSetting,0,lParam);
}


int EnumSettings(HANDLE hContact, char* module, ModuleSettingLL *msll)
{
	DBCONTACTENUMSETTINGS dbces;
	// enum all setting the contact has for the module
	dbces.pfnEnumProc = enumSettingsProc;
	dbces.szModule = module;
	dbces.lParam = (LPARAM)msll;
	msll->first = 0;
	msll->last = 0;
	return !CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)hContact,(LPARAM)&dbces);
}

int CheckIfModuleIsEmptyProc(const char *szSetting,LPARAM lParam)
{
	return 1;
}

int IsModuleEmpty(HANDLE hContact, char* szModule)
{
	DBCONTACTENUMSETTINGS dbces;
	int retVal;
	dbces.pfnEnumProc = CheckIfModuleIsEmptyProc;
	dbces.szModule = szModule;
	retVal = CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)hContact,(LPARAM)&dbces);
	if (retVal >= 0)
		return 0;
	else return 1;
}