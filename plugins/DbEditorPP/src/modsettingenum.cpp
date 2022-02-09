#include "stdafx.h"

void FreeModuleSettingLL(ModuleSettingLL *msll)
{
	if (msll == nullptr)
		return;

	ModSetLinkLinkItem *item = msll->first;

	while (item) {
		mir_free(item->name);
		ModSetLinkLinkItem *temp = item;
		item = (ModSetLinkLinkItem *)item->next;
		mir_free(temp);
	}

	msll->first = nullptr;
	msll->last = nullptr;
}

int enumModulesSettingsProc(const char *setting, void *pParam)
{
	ModuleSettingLL *msll = (ModuleSettingLL *)pParam;
	if (!msll->first) {
		msll->first = (ModSetLinkLinkItem *)mir_alloc(sizeof(ModSetLinkLinkItem));
		if (!msll->first)
			return 1;

		msll->first->name = mir_strdup(setting);
		msll->first->next = nullptr;
		msll->last = msll->first;
	}
	else {
		ModSetLinkLinkItem *item = (ModSetLinkLinkItem *)mir_alloc(sizeof(ModSetLinkLinkItem));
		if (!item)
			return 1;

		msll->last->next = item;
		msll->last = item;
		item->name = mir_strdup(setting);
		item->next = nullptr;
	}
	return 0;
}

int EnumModules(ModuleSettingLL *msll) // 1 = success, 0 = fail
{
	msll->first = nullptr;
	msll->last = nullptr;
	if (db_enum_modules(enumModulesSettingsProc, msll)) {
		g_pMainWindow->msg(TranslateT("Error loading module list"));
		return 0;
	}
	return 1;
}

int enumSettingsProc(const char *setting, void *lParam)
{
	return enumModulesSettingsProc(setting, lParam);
}

int EnumSettings(MCONTACT hContact, const char *module, ModuleSettingLL *msll)
{
	// enum all setting the contact has for the module
	msll->first = nullptr;
	msll->last = nullptr;
	if (db_enum_settings(hContact, enumSettingsProc, module, msll)) {
		g_pMainWindow->msg(TranslateT("Error loading setting list"));
		return 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int stringCompare(const char *p1, const char *p2)
{
	return mir_strcmp(p1, p2);
}

LIST<char> m_lResidentSettings(10, stringCompare);

int enumResidentProc(const char *setting, void*)
{
	m_lResidentSettings.insert(mir_strdup(setting));
	return 0;
}

int LoadResidentSettings()
{
	if (g_db)
		return !g_db->EnumResidentSettings(enumResidentProc, nullptr);
	return 0;
}

void FreeResidentSettings()
{
	for (auto &it : m_lResidentSettings)
		mir_free(it);
	m_lResidentSettings.destroy();
}

int IsResidentSetting(const char *module, const char *setting)
{
	if (!m_lResidentSettings.getCount()) return 0;
	if (!setting) return 1;

	char str[2 * FLD_SIZE];
	mir_strncpy(str, module, _countof(str) - 1);
	mir_strcat(str, "/");
	mir_strncat(str, setting, _countof(str));
	return m_lResidentSettings.getIndex(str) != -1;
}

int EnumResidentSettings(const char *module, ModuleSettingLL *msll)
{
	msll->first = nullptr;
	msll->last = nullptr;

	if (!module) return 0;
	if (!m_lResidentSettings.getCount()) return 0;

	int len = (int)mir_strlen(module);
	int cnt = 0;

	for (auto &it : m_lResidentSettings) {
		if (strncmp(module, it, len))
			continue;

		if (it[len] != '/' || it[len + 1] == 0)
			continue;

		enumModulesSettingsProc(&it[len + 1], msll);
		cnt++;
	}
	return cnt;
}

static int fixing = 0;

// previously saved in DB resident settings are unaccessible. 
// so let's find them and delete from DB
int fixResidentSettings()
{
	if (!m_lResidentSettings.getCount() || fixing)
		return 0;

	ModuleSettingLL ModuleList;
	if (!EnumModules(&ModuleList))
		return 0;

	fixing = 1;
	int cnt = 0;

	for (auto &hContact : Contacts()) {
		for (ModSetLinkLinkItem *module = ModuleList.first; module; module = module->next) {
			if (db_is_module_empty(hContact, module->name))
				continue;

			ModuleSettingLL SettingList;
			if (!EnumSettings(hContact, module->name, &SettingList))
				continue;

			for (ModSetLinkLinkItem *setting = SettingList.first; setting; setting = setting->next) {
				char str[2 * FLD_SIZE];
				mir_strncpy(str, module->name, _countof(str) - 1);
				mir_strcat(str, "/");
				mir_strncat(str, setting->name, _countof(str));

				int idx = m_lResidentSettings.getIndex(str);
				if (idx == -1)
					continue;

				Netlib_Logf(nullptr, "Removing resident setting %s/%s for contact %d", module->name, setting->name, hContact);
				g_db->SetSettingResident(0, str);
				db_unset(hContact, module->name, setting->name);
				g_db->SetSettingResident(1, str);
				cnt++;
			}

			FreeModuleSettingLL(&SettingList);
		}
	}

	FreeModuleSettingLL(&ModuleList);

	fixing = 0;

	return cnt;
}
