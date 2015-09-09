#include "stdafx.h"

extern HANDLE hUpdateService;

void InitServices()
{
	hUpdateService = CreateServiceFunction(MS_MESSAGESTATE_UPDATE, UpdateService);
}

void DestroyServices()
{
	if (hUpdateService)
		DestroyServiceFunction(hUpdateService);
}

INT_PTR UpdateService(WPARAM hContact, LPARAM lParam)
{
	MessageReadData *mrd = (MessageReadData*)lParam;
	if (mrd->dw_lastTime > db_get_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME, 0))
	{
		db_set_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME, mrd->dw_lastTime);
		db_set_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME_TYPE, mrd->iTimeType);
		IconsUpdate(hContact, mrd->dw_lastTime);
		ExtraIconsApply(hContact, 0);
	}
	return 0; 
}