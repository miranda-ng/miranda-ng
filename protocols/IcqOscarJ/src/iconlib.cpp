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
//  Support for IcoLib plug-in
//
// -----------------------------------------------------------------------------
#include "icqoscar.h"
#include "m_icolib.h"

IcqIconHandle IconLibDefine(const char *desc, const char *section, const char *module, const char *ident, const TCHAR *def_file, int def_idx)
{
	SKINICONDESC sid = { sizeof(sid) };
	sid.pwszSection = make_unicode_string(section);
	sid.pwszDescription = make_unicode_string(desc);
	sid.flags = SIDF_ALL_TCHAR;

	char szName[MAX_PATH + 128];
	null_snprintf(szName, sizeof(szName), "%s_%s", module ? module : ICQ_PROTOCOL_NAME, ident);
	sid.pszName = szName;
	sid.ptszDefaultFile = (TCHAR*)def_file;
	sid.iDefaultIndex = def_idx;

	IcqIconHandle hIcon = (IcqIconHandle)SAFE_MALLOC(sizeof(IcqIconHandle_s));
	hIcon->szName = null_strdup(sid.pszName);
	hIcon->hIcoLib = Skin_AddIcon(&sid);

	SAFE_FREE(&sid.pwszSection);
	SAFE_FREE(&sid.pwszDescription);

	return hIcon;
}


void IconLibRemove(IcqIconHandle *phIcon)
{
	if (phIcon && *phIcon)
	{
		IcqIconHandle hIcon = *phIcon;

		CallService(MS_SKIN2_REMOVEICON, 0, (LPARAM)hIcon->szName);
		SAFE_FREE(&hIcon->szName);
		SAFE_FREE((void**)phIcon);
	}
}


HANDLE IcqIconHandle_s::Handle()
{
	if (this)
		return hIcoLib;

	return NULL;
}


HICON IcqIconHandle_s::GetIcon(bool big)
{
	if (this)
		return Skin_GetIconByHandle(hIcoLib, big);

	return NULL;
}

void IcqIconHandle_s::ReleaseIcon(bool big)
{
	Skin_ReleaseIcon(szName, big);
}

