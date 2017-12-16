#include "stdafx.h"

IconItemT CToxProto::Icons[] =
{
	{ LPGENW("Protocol icon"),			"main",				IDI_TOX			},
};

void CToxProto::InitIcons()
{
	Icon_RegisterT(g_hInstance, LPGENW("Protocols") L"/" _A2W(MODULE), Icons, _countof(Icons), MODULE);
}

HANDLE CToxProto::GetIconHandle(int iconId)
{
	for (size_t i = 0; i < _countof(Icons); i++)
		if (Icons[i].defIconID == iconId)
			return Icons[i].hIcolib;

	return nullptr;
}
