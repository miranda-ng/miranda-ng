struct ModSetLinkLinkItem
{
	char *name;
	ModSetLinkLinkItem *next;
};

struct ModuleSettingLL
{
	ModSetLinkLinkItem *first;
	ModSetLinkLinkItem *last;
};

int EnumModules(ModuleSettingLL *msll);
int EnumSettings(MCONTACT hContact, char *module, ModuleSettingLL *msll);

void FreeModuleSettingLL(ModuleSettingLL *msll);

bool IsModuleEmpty(MCONTACT hContact, char *szModule);
