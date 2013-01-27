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
//  DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------
#include "icqoscar.h"

#include "m_extraicons.h"
#include "m_icolib.h"

HINSTANCE hInst;
int hLangpack;
CLIST_INTERFACE *pcli;

HANDLE   hExtraXStatus;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"IcqOscarJ Protocol",
	__VERSION_DWORD,
	"ICQ protocol support for Miranda NG.",
	"Joe Kucera, Bio, Martin \xd6" "berg, Richard Hughes, Jon Keating, etc",
	"jokusoftware@miranda-im.org",
	"(C) 2000-2010 M.\xd6" "berg, R.Hughes, J.Keating, Bio, Angeli-Ka, G.Hazan, J.Kucera",
	"http://miranda-ng.org/",
	UNICODE_AWARE,   //doesn't replace anything built-in
	{0x73a9615c, 0x7d4e, 0x4555, {0xba, 0xdb, 0xee, 0x5, 0xdc, 0x92, 0x8e, 0xff}} // {73A9615C-7D4E-4555-BADB-EE05DC928EFF}
};

extern "C" PLUGININFOEX __declspec(dllexport) *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

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

IconItem g_IconsList[4] =
{
	{ LPGEN("Request authorization"), "req_auth",      IDI_AUTH_ASK     },
	{ LPGEN("Grant authorization"),   "grant_auth",    IDI_AUTH_GRANT   },
	{ LPGEN("Revoke authorization"),  "revoke_auth",   IDI_AUTH_REVOKE  },
	{ LPGEN("Add to server list"),    "add_to_server", IDI_SERVLIST_ADD }
};

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP( &pluginInfo );

	srand(time(NULL));
	_tzset();

	pcli = (CLIST_INTERFACE*)CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)hInst);

	// Register the module
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName   = ICQ_PROTOCOL_NAME;
	pd.type     = PROTOTYPE_PROTOCOL;
	pd.fnInit   = icqProtoInit;
	pd.fnUninit = icqProtoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// Initialize charset conversion routines
	InitI18N();

	// Register static services
	CreateServiceFunction(ICQ_DB_GETEVENTTEXT_MISSEDMESSAGE, icq_getEventTextMissedMessage);

	// Define global icons
	Icon_Register(hInst, "Protocols", g_IconsList, SIZEOF(g_IconsList), ICQ_PROTOCOL_NAME);

	// Init extra statuses
	InitXStatusIcons();
	HookEvent(ME_SKIN2_ICONSCHANGED, OnReloadIcons);

	hExtraXStatus = ExtraIcon_Register("xstatus", "ICQ XStatus", "icq_xstatus13");

	g_MenuInit();
	return 0;
}


extern "C" int __declspec(dllexport) Unload(void)
{
	// destroying contact menu
	g_MenuUninit();

	g_Instances.destroy();
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
// OnPrebuildContactMenu event

void CListShowMenuItem(HANDLE hMenuItem, BYTE bShow)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	if (bShow)
		mi.flags = CMIM_FLAGS;
	else
		mi.flags = CMIM_FLAGS | CMIF_HIDDEN;

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&mi);
}

static void CListSetMenuItemIcon(HANDLE hMenuItem, HICON hIcon)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | CMIM_ICON;
	mi.hIcon = hIcon;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&mi);
}

/////////////////////////////////////////////////////////////////////////////////////////
// UpdateGlobalSettings event

void CIcqProto::UpdateGlobalSettings()
{
	char szServer[MAX_PATH] = "";
	getSettingStringStatic(NULL, "OscarServer", szServer, MAX_PATH);

	m_bSecureConnection = getSettingByte(NULL, "SecureConnection", DEFAULT_SECURE_CONNECTION);
	if (szServer[0]) {
		if (strstr(szServer, "aol.com"))
			setSettingString(NULL, "OscarServer", m_bSecureConnection ? DEFAULT_SERVER_HOST_SSL : DEFAULT_SERVER_HOST);

		if (m_bSecureConnection && !_strnicmp(szServer, "login.", 6)) {
			setSettingString(NULL, "OscarServer", DEFAULT_SERVER_HOST_SSL);
			setSettingWord(NULL, "OscarPort", DEFAULT_SERVER_PORT_SSL);
		}
	}

	if (m_hServerNetlibUser) {
		NETLIBUSERSETTINGS nlus = { sizeof(NETLIBUSERSETTINGS) };
		if ( !m_bSecureConnection && CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)m_hServerNetlibUser, (LPARAM)&nlus)) {
			if (nlus.useProxy && nlus.proxyType == PROXYTYPE_HTTP)
				m_bGatewayMode = 1;
			else
				m_bGatewayMode = 0;
		}
		else m_bGatewayMode = 0;
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
