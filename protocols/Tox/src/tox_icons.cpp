#include "stdafx.h"

IconItemT CToxProto::Icons[] =
{
	{ LPGENW("Protocol icon"),			"main",				IDI_TOX  },
	{ LPGENW("Action icon"),			"main",				IDI_ME   },
	{ LPGENW("Correction icon"),		"edit",				IDI_EDIT },
};

void CToxProto::InitIcons()
{
	Icon_RegisterT(g_plugin.getInst(), LPGENW("Protocols") L"/" _A2W(MODULE), Icons, _countof(Icons), MODULE);
}

HANDLE CToxProto::GetIconHandle(int iconId)
{
	for (auto &it : Icons)
		if (it.defIconID == iconId)
			return it.hIcolib;
	return nullptr;
}

HICON CToxProto::GetIcon(int iconId)
{
	for (auto &it : Icons)
		if (it.defIconID == iconId)
			return IcoLib_GetIconByHandle(it.hIcolib);
	return nullptr;
}
