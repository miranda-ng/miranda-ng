/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-14 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

CDb3Mmap::CDb3Mmap(const TCHAR* tszFileName) :
	CDb3Base(tszFileName)
{
	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);
	m_ChunkSize = sinf.dwAllocationGranularity;
}

CDb3Mmap::~CDb3Mmap()
{
	free(m_pNull);
}

int CDb3Mmap::Load(bool bSkipInit)
{
	int res = CDb3Base::Load(bSkipInit);
	if (!res && !bSkipInit)
		if (InitCrypt()) return 1;

	return res;
}

int CDb3Mmap::PrepareCheck()
{
	int ret = CheckDbHeaders();
	if (ret != ERROR_SUCCESS)
		return ret;

	InitCache();
	InitModuleNames();
	if ((ret = InitCrypt()) != ERROR_SUCCESS)
		return ret;

	return ERROR_SUCCESS;
}
