#include "headers.h"

HIMAGELIST himl;

void addIcons(TCHAR* szModuleFileName)
{
	SKINICONDESC sid={0};
	char name[32];
	sid.cbSize = sizeof(sid);
	sid.ptszSection = _T(modFullname);
	sid.ptszDefaultFile = szModuleFileName;
	sid.flags = SIDF_ALL_TCHAR;

	// closed known module
	sid.ptszDescription = LPGENT("Closed Known Module");
	mir_snprintf(name, SIZEOF(name), "DBE++_%d", ICO_KNOWN);
	sid.pszName = name;
	sid.iDefaultIndex = -ICO_KNOWN;
	Skin_AddIcon(&sid);

	// open known module
	sid.ptszDescription = LPGENT("Open Known Module");
	mir_snprintf(name, SIZEOF(name), "DBE++_%d", ICO_KNOWNOPEN);
	sid.pszName = name;
	sid.iDefaultIndex = -ICO_KNOWNOPEN;
	Skin_AddIcon(&sid);

	// closed unknown module
	sid.ptszDescription = LPGENT("Closed Unknown Module");
	mir_snprintf(name, SIZEOF(name), "DBE++_%d", ICO_UNKNOWN);
	sid.pszName = name;
	sid.iDefaultIndex = -ICO_UNKNOWN;
	Skin_AddIcon(&sid);

	// open unknown module
	sid.ptszDescription = LPGENT("Open Unknown Module");
	mir_snprintf(name, SIZEOF(name), "DBE++_%d", ICO_UNKNOWNOPEN);
	sid.pszName = name;
	sid.iDefaultIndex = -ICO_UNKNOWNOPEN;
	Skin_AddIcon(&sid);

	// settings contact
	sid.ptszDescription = LPGENT("Settings");
	mir_snprintf(name, SIZEOF(name), "DBE++_%d", ICO_SETTINGS);
	sid.pszName = name;
	sid.iDefaultIndex = -ICO_SETTINGS;
	Skin_AddIcon(&sid);

	// contact group
	sid.ptszDescription = LPGENT("Contacts Group");
	mir_snprintf(name, SIZEOF(name), "DBE++_%d", ICO_CONTACTS);
	sid.pszName = name;
	sid.iDefaultIndex = -ICO_CONTACTS;
	Skin_AddIcon(&sid);

	// unknwon contact
	sid.ptszDescription = LPGENT("Unknown Contact");
	mir_snprintf(name, SIZEOF(name), "DBE++_%d", ICO_OFFLINE);
	sid.pszName = name;
	sid.iDefaultIndex = -ICO_OFFLINE;
	Skin_AddIcon(&sid);

	// known contact
	sid.ptszDescription = LPGENT("Known Contact");
	mir_snprintf(name, SIZEOF(name), "DBE++_%d", ICO_ONLINE);
	sid.pszName = name;
	sid.iDefaultIndex = -ICO_ONLINE;
	Skin_AddIcon(&sid);
}

HICON LoadSkinnedDBEIcon(int icon)
{
	char name[32];
	mir_snprintf(name, SIZEOF(name), "DBE++_%d", icon);
	HICON hIcon = (HICON)CallService(MS_SKIN2_GETICON,0,(LPARAM)name);
	return (hIcon) ? hIcon : LoadIcon(hInst, MAKEINTRESOURCE(icon));
}


int AddIconToList(HIMAGELIST hil, HICON hIcon)
{
	if (!hIcon || !hil)
		return 0;

	ImageList_AddIcon(hil, hIcon);
	return 1;
}

static PROTOCOLDESCRIPTOR **protocols = NULL;
static int protoCount = 0;
static int shift = 0;

void AddProtoIconsToList(HIMAGELIST hil, int newshift)
{
	shift = newshift;

	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&protoCount,(LPARAM)&protocols);

	for (int i = 0; i < protoCount; i++) {
		if (protocols[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		HICON hIcon;
		if (hIcon=LoadSkinnedProtoIcon(protocols[i]->szName, ID_STATUS_ONLINE))
			AddIconToList(hil, hIcon);
		else
			AddIconToList(himl, LoadSkinnedDBEIcon(ICO_ONLINE));
	}
}

int GetProtoIcon(char *szProto)
{
	if ( !protoCount || !protocols || !szProto)
		return DEF_ICON;

	int n = 0;

	for (int i = 0; i < protoCount; i++) {
		if (protocols[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (!mir_strcmp(protocols[i]->szName, szProto))
			return n + shift;

		n++;
	}

	return DEF_ICON;
}

BOOL IsProtocolLoaded(char* pszProtocolName)
{
	if (protoCount)
		for(int i = 0; i < protoCount; i++) {
			if (protocols[i]->type != PROTOTYPE_PROTOCOL)
				continue;

			if (!mir_strcmp(protocols[i]->szName, pszProtocolName))
				return TRUE;
		}

	return FALSE;
}
