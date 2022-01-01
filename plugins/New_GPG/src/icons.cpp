// Copyright © 2010-22 SecureIM developers (baloo and others), sss
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "stdafx.h"

static IconItem iconList[] =
{
	{ "Secured", "secured", IDI_SECURED },
	{ "Unsecured", "unsecured", IDI_UNSECURED }
};

void InitIconLib()
{
	g_plugin.registerIcon(MODULENAME, iconList);
}

HANDLE IconLibHookIconsChanged(MIRANDAHOOK hook)
{
	return HookEvent(ME_SKIN_ICONSCHANGED, hook);
}

void setSrmmIcon(MCONTACT h)
{
	MCONTACT hContact = db_mc_isMeta(h) ? metaGetMostOnline(h) : h;
	bool enabled = isContactSecured(hContact);
	MCONTACT hMC = db_mc_tryMeta(hContact);

	int flags = enabled ? 0 : MBF_HIDDEN;
	Srmm_SetIconFlags(hContact, MODULENAME, 1, flags);
	if (hMC != hContact)
		Srmm_SetIconFlags(hMC, MODULENAME, 1, flags);

	flags = enabled ? MBF_HIDDEN : 0;
	Srmm_SetIconFlags(hContact, MODULENAME, 2, flags);
	if (hMC != hContact)
		Srmm_SetIconFlags(hMC, MODULENAME, 2, flags);

	const char *szIconId = (enabled) ? "secured" : "unsecured";
	ExtraIcon_SetIconByName(g_plugin.hCLIcon, hContact, szIconId);
	if (hMC != hContact)
		ExtraIcon_SetIconByName(g_plugin.hCLIcon, hMC, szIconId);
}
