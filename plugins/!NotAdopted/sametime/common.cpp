#include "common.h"

int DBGetContactSettingUtf(HANDLE hContact, char *module, char *setting, DBVARIANT *dbv) {
	DBCONTACTGETSETTING cgs;
	cgs.szModule = module;
	cgs.szSetting = setting;
	cgs.pValue = dbv;

	dbv->type = DBVT_UTF8;

	return CallService(MS_DB_CONTACT_GETSETTING_STR, (WPARAM)hContact, (LPARAM)&cgs);
}