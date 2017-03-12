#include "stdafx.h"

IconItemT CSlackProto::Icons[] =
{
	{ LPGENW("Protocol icon"),			"main",				IDI_SLACK	},
};

void CSlackProto::InitIcons()
{
	Icon_RegisterT(g_hInstance, LPGENW("Protocols") L"/" _A2W(MODULE), Icons, _countof(Icons), MODULE);
}

HANDLE CSlackProto::GetIconHandle(int iconId)
{
	for (size_t i = 0; i < _countof(Icons); i++)
		if (Icons[i].defIconID == iconId)
			return Icons[i].hIcolib;

	return NULL;
}