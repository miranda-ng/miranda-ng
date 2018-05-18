#include "stdafx.h"

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"),			"main",				IDI_TOX  },
	{ LPGEN("Action icon"),			"main",				IDI_ME   },
	{ LPGEN("Correction icon"),		"edit",				IDI_EDIT },
};

void CToxProto::InitIcons()
{
	g_plugin.registerIcon(LPGEN("Protocols") "/" MODULE, iconList, MODULE);
}

HANDLE CToxProto::GetIconHandle(int iconId)
{
	for (auto &it : iconList)
		if (it.defIconID == iconId)
			return it.hIcolib;
	return nullptr;
}

HICON CToxProto::GetIcon(int iconId)
{
	for (auto &it : iconList)
		if (it.defIconID == iconId)
			return IcoLib_GetIconByHandle(it.hIcolib);
	return nullptr;
}
