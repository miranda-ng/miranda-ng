// DbHelpers.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

typedef std::vector<const char*> SettingsList;

static int EnumSettingsProc1(const char *pszSetting, LPARAM lParam)
{
	return 0;
}

bool ModuleSettingsExists(HANDLE hContact, const char* pszModuleName)
{
	DBCONTACTENUMSETTINGS dbces = {0};
	dbces.szModule = pszModuleName;
	dbces.pfnEnumProc = EnumSettingsProc1;

	int nResult = ::CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)hContact, (LPARAM)&dbces);
	return (nResult != -1);
}

static int EnumSettingsProc2(const char *pszSetting, LPARAM lParam)
{
	SettingsList& settingsList = *((SettingsList*)lParam);
	settingsList.push_back(_strdup(pszSetting));
	return 0;
}

void DeleteModuleSettings(HANDLE hContact, const char* pszModuleName)
{
	SettingsList settingsList;
	DBCONTACTENUMSETTINGS dbces = {0};
	dbces.szModule = pszModuleName;
	dbces.lParam = (LPARAM)&settingsList;
	dbces.pfnEnumProc = EnumSettingsProc2;

	int nResult = ::CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)hContact, (LPARAM)&dbces);
	if (nResult != -1)
	{
		for(unsigned i=0; i<settingsList.size(); i++)
		{
			DBDeleteContactSetting(hContact, pszModuleName, settingsList[i]);
			free((char*)settingsList[i]);
		}
	}
}


static int GetSetting(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	DBCONTACTGETSETTING cgs;

	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=dbv;
	dbv->type = 0;

	int rr = CallService(MS_DB_CONTACT_GETSETTING,(WPARAM)hContact,(LPARAM)&cgs);

	if 	(dbv->type != DBVT_UTF8)
		return rr;
	else
		return 1;
}

void RenameModule(HANDLE hContact, const char* pszOldName, const char* pszNewName)
{
	SettingsList settingsList;
	DBCONTACTENUMSETTINGS dbces = {0};
	dbces.szModule = pszOldName;
	dbces.lParam = (LPARAM)&settingsList;
	dbces.pfnEnumProc = EnumSettingsProc2;

	int nResult = ::CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)hContact, (LPARAM)&dbces);
	if (nResult != -1)
	{
		DBVARIANT dbv;

		for(unsigned i=0; i<settingsList.size(); i++)
		{
			const char* pszSetting = settingsList[i];

			if (!GetSetting(hContact, pszOldName, pszSetting, &dbv))
			{
				switch (dbv.type)
				{
					case DBVT_BYTE:
						DBWriteContactSettingByte(hContact, pszNewName, pszSetting, dbv.bVal);
					break;

					case DBVT_WORD:
						DBWriteContactSettingWord(hContact, pszNewName, pszSetting, dbv.wVal);
					break;

					case DBVT_DWORD:
						DBWriteContactSettingDword(hContact, pszNewName, pszSetting, dbv.dVal);
					break;

					case DBVT_ASCIIZ:
						DBWriteContactSettingString(hContact, pszNewName, pszSetting, dbv.pszVal);
					break;

					case DBVT_UTF8:
						DBWriteContactSettingStringUtf(hContact, pszNewName, pszSetting, dbv.pszVal);
					break;

					default:
						assert(0);
						break;
				}
				DBDeleteContactSetting(hContact, pszOldName, pszSetting);
				DBFreeVariant(&dbv);
			}
			free((char*)settingsList[i]);
		}
	}
}