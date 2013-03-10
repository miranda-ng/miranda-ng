// DbHelpers.h
//
//////////////////////////////////////////////////////////////////////

bool ModuleSettingsExists(HANDLE hContact, const char* pszModuleName);
void DeleteModuleSettings(HANDLE hContact, const char* pszModuleName);
void RenameModule(HANDLE hContact, const char* pszOldName, const char* pszNewName);
