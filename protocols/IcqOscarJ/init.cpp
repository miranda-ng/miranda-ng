// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// -----------------------------------------------------------------------------
//
// File name      : $URL: http://miranda.googlecode.com/svn/trunk/miranda/protocols/IcqOscarJ/init.cpp $
// Revision       : $Revision: 13596 $
// Last change on : $Date: 2011-04-15 22:07:23 +0300 (ÐŸÑ‚, 15 Ð°Ð¿Ñ€ 2011) $
// Last change by : $Author: george.hazan $
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"
#include "m_extraicons.h"

HINSTANCE hInst;
PLUGINLINK* pluginLink;
MM_INTERFACE mmi;
UTF8_INTERFACE utfi;
MD5_INTERFACE md5i;
LIST_INTERFACE li;
int hLangpack;

DWORD MIRANDA_VERSION;

HANDLE hStaticServices[1];
IcqIconHandle hStaticIcons[4];
HANDLE hStaticHooks[1];;
HANDLE hExtraXStatus = NULL;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"IcqOscarJ Protocol",
	__VERSION_DWORD,
	"Support for ICQ network, enhanced.",
	"Joe Kucera, Bio, Martin Öberg, Richard Hughes, Jon Keating, etc",
	"jokusoftware@miranda-im.org",
	"(C) 2000-2010 M.Öberg, R.Hughes, J.Keating, Bio, Angeli-Ka, G.Hazan, J.Kucera",
	"http://addons.miranda-im.org/details.php?action=viewfile&id=1683",
	UNICODE_AWARE,
	0,   //doesn't replace anything built-in
	{0x73a9615c, 0x7d4e, 0x4555, {0xba, 0xdb, 0xee, 0x5, 0xdc, 0x92, 0x8e, 0xff}} // {73A9615C-7D4E-4555-BADB-EE05DC928EFF}
};

extern "C" PLUGININFOEX __declspec(dllexport) *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static PROTO_INTERFACE* icqProtoInit( const char* pszProtoName, const TCHAR* tszUserName )
{
	CIcqProto *ppro = new CIcqProto(pszProtoName, tszUserName);
	g_Instances.insert(ppro);
	return ppro;
}


static int icqProtoUninit( PROTO_INTERFACE* ppro )
{
	g_Instances.remove(( CIcqProto* )ppro);
	delete ( CIcqProto* )ppro;
	return 0;
}


static int OnModulesLoaded( WPARAM, LPARAM )
{
	hExtraXStatus = ExtraIcon_Register("xstatus", "ICQ XStatus");
	return 0;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getLI( &li );
	mir_getMMI( &mmi );
	mir_getUTFI( &utfi );
	mir_getMD5I( &md5i );
	mir_getLP( &pluginInfo );

	// Get Miranda version
	MIRANDA_VERSION = (DWORD)CallService(MS_SYSTEM_GETVERSION, 0, 0);

	{ // Are we running under unicode Miranda core ?
		char szVer[MAX_PATH];

		CallService(MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, (LPARAM)szVer);
		_strlwr(szVer); // make sure it is lowercase

		if (strstrnull(szVer, "alpha") != NULL)
		{ // Are we running under Alpha Core
			MIRANDA_VERSION |= 0x80000000;
		}
		else if (strstrnull(szVer, "preview") == NULL)
		{ // for Final Releases of Miranda 0.5+ clear build number
			MIRANDA_VERSION &= 0xFFFFFF00;
		}

		// Check if _UNICODE matches Miranda's _UNICODE
		if (strstrnull(szVer, "unicode") == NULL)
		{
			char szMsg[MAX_PATH], szCaption[100];

			MessageBoxUtf(NULL, ICQTranslateUtfStatic("You cannot use Unicode version of ICQ Protocol plug-in with Ansi version of Miranda IM.", szMsg, MAX_PATH),
				ICQTranslateUtfStatic("ICQ Plugin", szCaption, 100), MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
			return 1; // Failure
		}
	}

	srand(time(NULL));
	_tzset();

	// Register the module
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize   = sizeof(pd);
	pd.szName   = ICQ_PROTOCOL_NAME;
	pd.type     = PROTOTYPE_PROTOCOL;
	pd.fnInit   = icqProtoInit;
	pd.fnUninit = icqProtoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// Initialize charset conversion routines
	InitI18N();

	// Register static services
	hStaticServices[0] = CreateServiceFunction(ICQ_DB_GETEVENTTEXT_MISSEDMESSAGE, icq_getEventTextMissedMessage);

	{
		// Define global icons
		char szSectionName[MAX_PATH];
		null_snprintf(szSectionName, sizeof(szSectionName), "Protocols/%s", ICQ_PROTOCOL_NAME);

		TCHAR lib[MAX_PATH];
		GetModuleFileName(hInst, lib, MAX_PATH);
		hStaticIcons[ISI_AUTH_REQUEST] = IconLibDefine(LPGEN("Request authorization"), szSectionName, NULL, "req_auth", lib, -IDI_AUTH_ASK);
		hStaticIcons[ISI_AUTH_GRANT] = IconLibDefine(LPGEN("Grant authorization"), szSectionName, NULL, "grant_auth", lib, -IDI_AUTH_GRANT);
		hStaticIcons[ISI_AUTH_REVOKE] = IconLibDefine(LPGEN("Revoke authorization"), szSectionName, NULL, "revoke_auth", lib, -IDI_AUTH_REVOKE);
		hStaticIcons[ISI_ADD_TO_SERVLIST] = IconLibDefine(LPGEN("Add to server list"), szSectionName, NULL, "add_to_server", lib, -IDI_SERVLIST_ADD);
	}

	hStaticHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	g_MenuInit();
	return 0;
}


extern "C" int __declspec(dllexport) Unload(void)
{
	int i;

	// Release static icon handles
	for (i = 0; i < SIZEOF(hStaticIcons); i++)
		IconLibRemove(&hStaticIcons[i]);

	// Release static event hooks
	for (i = 0; i < SIZEOF(hStaticHooks); i++)
		if (hStaticHooks[i])
			UnhookEvent(hStaticHooks[i]);

	// destroying contact menu
	g_MenuUninit();

	// Destroy static service functions
	for (i = 0; i < SIZEOF(hStaticServices); i++)
		if (hStaticServices[i])
			DestroyServiceFunction(hStaticServices[i]);

	g_Instances.destroy();

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
// OnPrebuildContactMenu event

void CListShowMenuItem(HANDLE hMenuItem, BYTE bShow)
{
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	if (bShow)
		mi.flags = CMIM_FLAGS;
	else
		mi.flags = CMIM_FLAGS | CMIF_HIDDEN;

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&mi);
}

static void CListSetMenuItemIcon(HANDLE hMenuItem, HICON hIcon)
{
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIM_ICON;

	mi.hIcon = hIcon;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&mi);
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnReloadIcons event

int CIcqProto::OnReloadIcons(WPARAM wParam, LPARAM lParam)
{
	memset(bXStatusCListIconsValid, 0, sizeof(bXStatusCListIconsValid));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// UpdateGlobalSettings event

void CIcqProto::UpdateGlobalSettings()
{
	char szServer[MAX_PATH] = "";
	getSettingStringStatic(NULL, "OscarServer", szServer, MAX_PATH);

	m_bSecureConnection = getSettingByte(NULL, "SecureConnection", DEFAULT_SECURE_CONNECTION);
	if (szServer[0])
	{
		if (strstr(szServer, "aol.com"))
			setSettingString(NULL, "OscarServer", m_bSecureConnection ? DEFAULT_SERVER_HOST_SSL : DEFAULT_SERVER_HOST);

		if (m_bSecureConnection && !_strnicmp(szServer, "login.", 6))
		{
			setSettingString(NULL, "OscarServer", DEFAULT_SERVER_HOST_SSL);
			setSettingWord(NULL, "OscarPort", DEFAULT_SERVER_PORT_SSL);
		}
	}

	if (m_hServerNetlibUser)
	{
		NETLIBUSERSETTINGS nlus = {0};

		nlus.cbSize = sizeof(NETLIBUSERSETTINGS);
		if (!m_bSecureConnection && CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)m_hServerNetlibUser, (LPARAM)&nlus))
		{
			if (nlus.useProxy && nlus.proxyType == PROXYTYPE_HTTP)
				m_bGatewayMode = 1;
			else
				m_bGatewayMode = 0;
		}
		else
			m_bGatewayMode = 0;
	}

	m_bSecureLogin = getSettingByte(NULL, "SecureLogin", DEFAULT_SECURE_LOGIN);
	m_bAimEnabled = getSettingByte(NULL, "AimEnabled", DEFAULT_AIM_ENABLED);
	m_bUtfEnabled = getSettingByte(NULL, "UtfEnabled", DEFAULT_UTF_ENABLED);
	m_wAnsiCodepage = getSettingWord(NULL, "AnsiCodePage", DEFAULT_ANSI_CODEPAGE);
	m_bDCMsgEnabled = getSettingByte(NULL, "DirectMessaging", DEFAULT_DCMSG_ENABLED);
	m_bTempVisListEnabled = getSettingByte(NULL, "TempVisListEnabled", DEFAULT_TEMPVIS_ENABLED);
	m_bSsiEnabled = getSettingByte(NULL, "UseServerCList", DEFAULT_SS_ENABLED);
	m_bSsiSimpleGroups = FALSE; /// TODO: enable, after server-list revolution is over
	m_bAvatarsEnabled = getSettingByte(NULL, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED);
	m_bXStatusEnabled = getSettingByte(NULL, "XStatusEnabled", DEFAULT_XSTATUS_ENABLED);
	m_bMoodsEnabled = getSettingByte(NULL, "MoodsEnabled", DEFAULT_MOODS_ENABLED);
}
