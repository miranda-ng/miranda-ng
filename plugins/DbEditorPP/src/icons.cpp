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


IconItem iconList[] = {
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



HANDLE GetIcoLibHandle(int icon)
{
	for (int i = 0; i < _countof(iconList); i++)
		if (iconList[i].defIconID == icon)
			return iconList[i].hIcolib;
	return INVALID_HANDLE_VALUE;
}

void IcoLibRegister(void)
{
	Icon_Register(hInst, modFullname, iconList, _countof(iconList));
}

HICON LoadSkinnedDBEIcon(int icon)
{
	for (int i = 0; i < _countof(iconList); i++)
		if (iconList[i].defIconID == icon)
			return IcoLib_GetIconByHandle(iconList[i].hIcolib);

	return LoadIcon(hInst, MAKEINTRESOURCE(icon));
}

static PROTOACCOUNT **protocols = NULL;
static int protoCount = 0;

HIMAGELIST LoadIcons()
{
	HICON hIcon;
	HIMAGELIST hil = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, _countof(dbeIcons), 5);

	if (!hil) return NULL;

	for(int i = 0; i < _countof(dbeIcons); i++)
		ImageList_AddIcon(hil, LoadSkinnedDBEIcon(dbeIcons[i]));

	Proto_EnumAccounts(&protoCount, &protocols);

	for (int i = 0; i < protoCount; i++) {
		if (!Proto_IsProtocolLoaded(protocols[i]->szModuleName))
			ImageList_AddIcon(hil, LoadSkinnedDBEIcon(ICO_OFFLINE));
		else
		if (hIcon = Skin_LoadProtoIcon(protocols[i]->szModuleName, ID_STATUS_ONLINE))
			ImageList_AddIcon(hil, hIcon);
		else
			ImageList_AddIcon(hil, LoadSkinnedDBEIcon(ICO_ONLINE));
	}

	return hil;
}


int GetProtoIconIndex(const char *szProto)
{
	if (szProto && szProto[0]) {
		if (protoCount && protocols) {
			for (int i = 0; i < protoCount; i++) {
				if (!mir_strcmp(protocols[i]->szModuleName, szProto))
					return i + _countof(dbeIcons);
			}
			if (Proto_IsProtocolLoaded(szProto))
				return _countof(dbeIcons) - 2; // ICO_ONLINE;
		}
	}
	return _countof(dbeIcons) - 1; // ICO_OFFLINE;
}
