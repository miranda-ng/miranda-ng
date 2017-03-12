#include "stdafx.h"

int CSlackProto::OnModulesLoaded(WPARAM, LPARAM)
{
	CSlackProto::InitIcons();
	//CSlackProto::InitMenus();

	return 0;
}