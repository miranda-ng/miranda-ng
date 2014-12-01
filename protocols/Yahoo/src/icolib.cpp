/*
 * $Id: util.c 3936 2006-10-02 06:58:19Z ghazan $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */
#include "yahoo.h"
#include <m_langpack.h>
#include <win2k.h>
#include "m_icolib.h"

#include "resource.h"

static IconItem iconList[] =
{
	{	LPGEN("Main"),         "yahoo",      IDI_YAHOO      },
	{	LPGEN("Mail"),         "mail",       IDI_INBOX      },
	{	LPGEN("Profile"),      "profile",    IDI_PROFILE    },
	{	LPGEN("Refresh"),      "refresh",    IDI_REFRESH    },
	{	LPGEN("Address Book"), "yab",        IDI_YAB        },
	{	LPGEN("Set Status"),   "set_status", IDI_SET_STATUS },
	{	LPGEN("Calendar"),     "calendar",   IDI_CALENDAR   }
};

void CYahooProto::IconsInit( void )
{
	Icon_Register(hInstance, "Protocols/YAHOO", iconList, SIZEOF(iconList), "YAHOO");
}

HICON CYahooProto::LoadIconEx( const char* name, bool big )
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "YAHOO_%s", name);
	return Skin_GetIcon(szSettingName, big);
}

HANDLE CYahooProto::GetIconHandle(int iconId)
{
	for (unsigned i=0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;

	return NULL;
}

void CYahooProto::ReleaseIconEx(const char* name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "YAHOO_%s", name);
	Skin_ReleaseIcon(szSettingName, big);
}
