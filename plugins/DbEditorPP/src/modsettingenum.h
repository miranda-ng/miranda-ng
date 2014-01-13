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
int EnumSettings(HANDLE hContact, char *module, ModuleSettingLL *msll);

void FreeModuleSettingLL(ModuleSettingLL *msll);

int IsModuleEmpty(HANDLE hContact, char *szModule);
