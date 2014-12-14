#include "headers.h"

HIMAGELIST himl;

IconItem iconList[] = {
	{ LPGENT("Main icon"), "DBE++_0", ICO_DBE_BUTT },
	{ LPGENT("Closed known module"), "DBE++_1", ICO_KNOWN },
	{ LPGENT("Open known module"), "DBE++_2", ICO_KNOWNOPEN },
	{ LPGENT("Settings"), "DBE++_5", ICO_SETTINGS },
	{ LPGENT("Contacts group"), "DBE++_6", ICO_CONTACTS },
	{ LPGENT("Unknown contact"), "DBE++_7", ICO_OFFLINE },
	{ LPGENT("Known contact"), "DBE++_8", ICO_ONLINE },
	{ LPGENT("Open user tree"), "DBE++_9", ICO_REGUSER },

	{ LPGENT("BLOB setting"), "DBE++_BINARY", ICO_BINARY },
	{ LPGENT("Byte setting"), "DBE++_BYTE", ICO_BYTE },
	{ LPGENT("Word setting"), "DBE++_WORD", ICO_WORD },
	{ LPGENT("Dword setting"), "DBE++_DWORD", ICO_DWORD },
	{ LPGENT("String setting"), "DBE++_STRING", ICO_STRING },
	{ LPGENT("Unicode setting"), "DBE++_UNICODE", ICO_UNICODE },
	{ LPGENT("Handle"), "DBE++_HANDLE", ICO_HANDLE }
};

void addIcons(void)
{
	Icon_Register(hInst, modFullname, iconList, SIZEOF(iconList));
}

HICON LoadSkinnedDBEIcon(int icon)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == icon)
			return Skin_GetIconByHandle(iconList[i].hIcolib);

	return LoadIcon(hInst, MAKEINTRESOURCE(icon));
}

int AddIconToList(HIMAGELIST hil, HICON hIcon)
{
	if (!hIcon || !hil)
		return 0;

	ImageList_AddIcon(hil, hIcon);
	return 1;
}

static PROTOACCOUNT **protocols = NULL;
static int protoCount = 0;
static int shift = 0;

void AddProtoIconsToList(HIMAGELIST hil, int newshift)
{
	shift = newshift;

	ProtoEnumAccounts(&protoCount, &protocols);

	for (int i = 0; i < protoCount; i++) {
		HICON hIcon;
		if (hIcon = LoadSkinnedProtoIcon(protocols[i]->szModuleName, ID_STATUS_ONLINE))
			AddIconToList(hil, hIcon);
		else
			AddIconToList(himl, LoadSkinnedDBEIcon(ICO_ONLINE));
	}
}

int GetProtoIcon(char *szProto)
{
	if (!protoCount || !protocols || !szProto)
		return DEF_ICON;

	int n = 0;

	for (int i = 0; i < protoCount; i++) {
		if (!mir_strcmp(protocols[i]->szModuleName, szProto))
			return n + shift;

		n++;
	}

	return DEF_ICON;
}

BOOL IsProtocolLoaded(char *pszProtocolName)
{
	if (protoCount)
		for (int i = 0; i < protoCount; i++)
			if (!mir_strcmp(protocols[i]->szModuleName, pszProtocolName))
				return TRUE;

	return FALSE;
}
