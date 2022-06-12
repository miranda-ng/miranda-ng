/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "../stdafx.h"

mir_cs::mir_cs()
{
	::InitializeCriticalSection(&m_cs);
}

mir_cs::~mir_cs()
{
	::DeleteCriticalSection(&m_cs);
}

void mir_cs::Lock()
{
	while (::TryEnterCriticalSection(&m_cs) == 0)
		SleepEx(50, TRUE);
}

void mir_cs::Unlock()
{
	::LeaveCriticalSection(&m_cs);
}
