#pragma once

void ReadSettingBlob(MCONTACT hContact, char *ModuleName,
					 char *SettingName, WORD *pSize, void **pbBlob);
void FreeSettingBlob(WORD pSize,void * pbBlob);

static void __inline SAFE_FREE(void** p)
{
	if (*p) {
		free(*p);
		*p = nullptr;
	}
}
