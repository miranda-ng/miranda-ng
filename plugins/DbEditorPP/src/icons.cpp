#include "stdafx.h"

int dbeIcons[] = {
	ICO_EMPTY,
	ICO_BINARY,
	ICO_BYTE,
	ICO_WORD,
	ICO_DWORD,
	ICO_STRING,
	ICO_UNICODE,
	ICO_HANDLE,
	ICO_SETTINGS,
	ICO_CLOSED,
	ICO_OPENED,
	ICO_CONTACTS,
	ICO_ONLINE,
	ICO_OFFLINE
};

static IconItem iconList[] = {
	{ LPGEN("Main icon"), "DBE++_0", ICO_DBE_BUTT },
	{ LPGEN("Closed module"), "DBE++_1", ICO_CLOSED },
	{ LPGEN("Open module"), "DBE++_2", ICO_OPENED },
	{ LPGEN("Settings"), "DBE++_5", ICO_SETTINGS },
	{ LPGEN("Contacts group"), "DBE++_6", ICO_CONTACTS },
	{ LPGEN("Unknown contact"), "DBE++_7", ICO_OFFLINE },
	{ LPGEN("Known contact"), "DBE++_8", ICO_ONLINE },
	{ LPGEN("Open user tree"), "DBE++_9", ICO_REGUSER },
	{ LPGEN("Empty setting"), "DBE++10", ICO_EMPTY },
	{ LPGEN("BLOB setting"), "DBE++_BINARY", ICO_BINARY },
	{ LPGEN("Byte setting"), "DBE++_BYTE", ICO_BYTE },
	{ LPGEN("Word setting"), "DBE++_WORD", ICO_WORD },
	{ LPGEN("Dword setting"), "DBE++_DWORD", ICO_DWORD },
	{ LPGEN("String setting"), "DBE++_STRING", ICO_STRING },
	{ LPGEN("Unicode setting"), "DBE++_UNICODE", ICO_UNICODE },
	{ LPGEN("Handle"), "DBE++_HANDLE", ICO_HANDLE }
};

void IcoLibRegister(void)
{
	g_plugin.registerIcon(LPGEN("Database") "/" modFullname, iconList);
}

HICON LoadSkinnedDBEIcon(int icon)
{
	for (auto &it : iconList)
		if (it.defIconID == icon)
			return IcoLib_GetIconByHandle(it.hIcolib);

	return LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(icon));
}

HIMAGELIST LoadIcons()
{
	HICON hIcon;
	HIMAGELIST hil = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, _countof(dbeIcons), 5);
	if (!hil)
		return nullptr;

	for (auto &it : dbeIcons)
		ImageList_AddIcon(hil, LoadSkinnedDBEIcon(it));

	for (auto &pa : Accounts()) {
		if (!Proto_GetAccount(pa->szModuleName))
			ImageList_AddIcon(hil, LoadSkinnedDBEIcon(ICO_OFFLINE));
		else if (hIcon = Skin_LoadProtoIcon(pa->szModuleName, ID_STATUS_ONLINE))
			ImageList_AddIcon(hil, hIcon);
		else
			ImageList_AddIcon(hil, LoadSkinnedDBEIcon(ICO_ONLINE));
	}

	return hil;
}

int GetProtoIconIndex(const char *szProto)
{
	if (szProto && szProto[0]) {
		auto &accs = Accounts();
		for (auto &pa : accs)
			if (!mir_strcmp(pa->szModuleName, szProto))
				return accs.indexOf(&pa) + _countof(dbeIcons);

		if (Proto_GetAccount(szProto))
			return _countof(dbeIcons) - 2; // ICO_ONLINE;
	}
	return _countof(dbeIcons) - 1; // ICO_OFFLINE;
}
