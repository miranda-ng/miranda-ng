// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018 Miranda NG team
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
// -----------------------------------------------------------------------------

#include "stdafx.h"

void CIcqProto::InitContactCache()
{
	mir_cslock l(m_csCache);
	for (auto &it : AccContacts())
		m_arCache.insert(new IcqCacheItem(getDword(it, "UIN"), it));
}

MCONTACT CIcqProto::FindContactByUIN(DWORD dwUin)
{
	mir_cslock l(m_csCache);
	auto *p = m_arCache.find((IcqCacheItem*)&dwUin);
	return (p) ? p->m_hContact : 0;
}

void CIcqProto::GetAvatarFileName(MCONTACT hContact, wchar_t* pszDest, size_t cbLen)
{
	int tPathLen = mir_snwprintf(pszDest, cbLen, L"%s\\%S", VARSW(L"%miranda_avatarcache%"), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(pszDest);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeW(pszDest);

	pszDest[tPathLen++] = '\\';

	CMStringW wszFileName(getMStringW(hContact, "IconId"));
	const wchar_t* szFileType = ProtoGetAvatarExtension(getByte(hContact, "AvatarType", PA_FORMAT_PNG));
	mir_snwprintf(pszDest + tPathLen, MAX_PATH - tPathLen, L"%s%s", wszFileName.c_str(), szFileType);
}

int StatusFromString(const CMStringW &wszStatus)
{
	if (wszStatus == "online")
		return ID_STATUS_ONLINE;
	if (wszStatus == "n/a")
		return ID_STATUS_NA;
	if (wszStatus == "away")
		return ID_STATUS_AWAY;
	if (wszStatus == "occupied")
		return ID_STATUS_OCCUPIED;
	if (wszStatus == "dnd")
		return ID_STATUS_DND;

	return ID_STATUS_OFFLINE;
}
