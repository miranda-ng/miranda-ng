#include "stdafx.h"


void FreeModuleSettingLL(ModuleSettingLL *msll)
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


int enumModulesSettingsProc(const char *setting, DWORD, LPARAM lParam)
{
	ModuleSettingLL *msll = (ModuleSettingLL *)lParam;
	if (!msll->first) {
		msll->first = (ModSetLinkLinkItem *)mir_alloc(sizeof(ModSetLinkLinkItem));
		if (!msll->first)
			return 1;

		msll->first->name = mir_strdup(setting);
		msll->first->next = 0;
		msll->last = msll->first;
	}
	else {
		ModSetLinkLinkItem *item = (ModSetLinkLinkItem *)mir_alloc(sizeof(ModSetLinkLinkItem));
		if (!item)
			return 1;

		msll->last->next = item;
		msll->last = item;
		item->name = mir_strdup(setting);
		item->next = 0;
	}
	return 0;
}


int EnumModules(ModuleSettingLL *msll) // 1 = success, 0 = fail
{
	msll->first = 0;
	msll->last = 0;
	if (CallService(MS_DB_MODULES_ENUM, (WPARAM)msll, (WPARAM)enumModulesSettingsProc)) {
		msg(TranslateT("Error loading module list"));
		return 0;
	}
	return 1;
}


int enumSettingsProc(const char *setting, LPARAM lParam)
{
	return enumModulesSettingsProc(setting, 0, lParam);
}


int EnumSettings(MCONTACT hContact, const char *module, ModuleSettingLL *msll)
{
	DBCONTACTENUMSETTINGS dbces;
	// enum all setting the contact has for the module
	dbces.pfnEnumProc = enumSettingsProc;
	dbces.szModule = module;
	dbces.lParam = (LPARAM)msll;
	msll->first = 0;
	msll->last = 0;
	if (CallService(MS_DB_CONTACT_ENUMSETTINGS, hContact, (LPARAM)&dbces)) {
		msg(TranslateT("Error loading setting list"));
		return 0;
	}
	return 1;
}


int CheckIfModuleIsEmptyProc(const char *, LPARAM)
{
	return 1;
}


int IsModuleEmpty(MCONTACT hContact, const char *module)
{
	DBCONTACTENUMSETTINGS dbces;
	dbces.pfnEnumProc = CheckIfModuleIsEmptyProc;
	dbces.szModule = module;
	return 0 > CallService(MS_DB_CONTACT_ENUMSETTINGS, hContact, (LPARAM)&dbces);
}


static int stringCompare(const char *p1, const char *p2)
{
	return mir_strcmp(p1, p2);
}


LIST<char> m_lResidentSettings(10, stringCompare);
LIST<char> m_lResidentModules(5, stringCompare);


int enumResidentProc(const char *setting, DWORD, LPARAM)
{
	m_lResidentSettings.insert(mir_strdup(setting));

	char str[FLD_SIZE];
	const char *end = strstr(setting, "/");
	if (end && (end - setting) < SIZEOF(str)) {
		mir_strncpy(str, setting, end - setting + 1);
		if (m_lResidentModules.getIndex(str) == -1)
				m_lResidentModules.insert(mir_strdup(str));
	}
	return 0;
}


int LoadResidentSettings()
{	
	if (g_db) 
		return !g_db->EnumResidentSettings(enumResidentProc, 0);
	return 0;
}


void FreeResidentSettings()
{
	for (int i = 0; i < m_lResidentSettings.getCount(); i++) {
		mir_free(m_lResidentSettings[i]);
	}
	m_lResidentSettings.destroy();

	for (int i = 0; i < m_lResidentModules.getCount(); i++) {
		mir_free(m_lResidentModules[i]);
	}
	m_lResidentModules.destroy();
}


int IsResidentSetting(const char *module, const char *setting)
{
	if (!m_lResidentSettings.getCount()) return 0;

	if (m_lResidentModules.getIndex((char*)module) == -1) return 0; 
	if (!setting) return 1;

	char str[2*FLD_SIZE];
    mir_strncpy(str, module, SIZEOF(str)-1);
    mir_strcat(str, "/");
    mir_strncat(str, setting, SIZEOF(str));
	return m_lResidentSettings.getIndex(str) != -1;
}


int EnumResidentSettings(const char *module, ModuleSettingLL *msll)
{
	msll->first = 0;
	msll->last = 0;

	if (!module) return 0;
	if (!m_lResidentSettings.getCount()) return 0;
	if (m_lResidentModules.getIndex((char*)module) == -1) return 0; 

	int len = (int)mir_strlen(module);
	int cnt = 0;

	for (int i = 0; i < m_lResidentSettings.getCount(); i++) {
		if (strncmp(module, m_lResidentSettings[i], len))
			continue;

		if (m_lResidentSettings[i][len] != '/' || m_lResidentSettings[i][len+1] == 0) continue;

		enumModulesSettingsProc(&m_lResidentSettings[i][len+1], 0, (LPARAM)msll);
		cnt++;
	}
	return cnt;
}


int EnumResidentModules(ModuleSettingLL *msll)
{
	msll->first = 0;
	msll->last = 0;

	if (!m_lResidentModules.getCount()) return 0;

	int cnt = 0;

	for (int i = 0; i < m_lResidentModules.getCount(); i++) {
		enumModulesSettingsProc(m_lResidentModules[i], 0, (LPARAM)msll);		
		cnt++;		
	}

	return cnt;
}


static int fixing = 0;

// previously saved in DB resident settings are unaccessible. 
// so let's find them and delete from DB
int fixResidentSettings()
{
	if (!m_lResidentSettings.getCount() || fixing) return 0;

	ModuleSettingLL ModuleList, SettingList;
	ModSetLinkLinkItem *module, *setting;
	MCONTACT hContact = 0;
	int NULLContactDone = 0;
	char str[2*FLD_SIZE];
	int cnt = 0;

	if (!EnumModules(&ModuleList)) return 0;

	fixing = 1;

	while (hwnd2mainWindow) {

		if (!hContact) {
			if (NULLContactDone) 
				break;
			else {
				NULLContactDone = 1;
				hContact = db_find_first();
			}
		}
		else 
			hContact = db_find_next(hContact);

		for (module = ModuleList.first; module; module = module->next) {

			if (IsModuleEmpty(hContact, module->name) || m_lResidentModules.getIndex(module->name) == -1) 
				continue;
	
			if (!EnumSettings(hContact, module->name, &SettingList))
			 	continue;

			for (setting = SettingList.first; setting; setting = setting->next) {

			    mir_strncpy(str, module->name, SIZEOF(str)-1);
			    mir_strcat(str, "/");
			    mir_strncat(str, setting->name, SIZEOF(str));
				int idx = m_lResidentSettings.getIndex(str);

				if (idx == -1)
					continue;

				g_db->SetSettingResident(0, str);
				db_unset(hContact, module->name, setting->name);
				g_db->SetSettingResident(1, str);

				cnt++;

			} // for(setting)

			FreeModuleSettingLL(&SettingList);

		} // for(module)
	}

	FreeModuleSettingLL(&ModuleList);

	fixing = 0;

	return cnt;
}
