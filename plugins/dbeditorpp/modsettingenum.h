struct ModSetLinkLinkItem {
	char *name;
	BYTE *next; //struct ModSetLinkLinkItem
	int known;
};

typedef struct {
	struct ModSetLinkLinkItem *first;
	struct ModSetLinkLinkItem *last;
} ModuleSettingLL;

int EnumModules(ModuleSettingLL *msll);
int EnumSettings(HANDLE hContact, char* module, ModuleSettingLL *msll);

void FreeModuleSettingLL(ModuleSettingLL *msll);

int IsModuleEmpty(HANDLE hContact, char* szModule);