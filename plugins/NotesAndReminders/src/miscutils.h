#pragma once

void ReadSettingBlob(MCONTACT hContact, char *ModuleName,
					 char *SettingName, WORD *pSize, void **pbBlob);
void FreeSettingBlob(WORD pSize,void * pbBlob);

void WriteSettingIntArray(MCONTACT hContact,char *ModuleName,
					 char *SettingName,const int *Value, int Size);
bool ReadSettingIntArray(MCONTACT hContact,char *ModuleName,
				   char *SettingName,int *Value, int Size);

static void __inline SAFE_FREE(void** p)
{
	if (*p) {
		free(*p);
		*p = nullptr;
	}
}
