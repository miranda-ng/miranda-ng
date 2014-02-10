// DbHelpers.h
//
//////////////////////////////////////////////////////////////////////

bool ModuleSettingsExists(HCONTACT hContact, const char* pszModuleName);
void DeleteModuleSettings(HCONTACT hContact, const char* pszModuleName);
void RenameModule(HCONTACT hContact, const char* pszOldName, const char* pszNewName);
