// DbHelpers.h
//
//////////////////////////////////////////////////////////////////////

bool ModuleSettingsExists(MCONTACT hContact, const char* pszModuleName);
void DeleteModuleSettings(MCONTACT hContact, const char* pszModuleName);
void RenameModule(MCONTACT hContact, const char* pszOldName, const char* pszNewName);
