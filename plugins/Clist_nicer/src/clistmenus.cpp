/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include "stdafx.h"
#include <m_genmenu.h>
#include <m_ignore.h>
#include "cluiframes.h"

#pragma hdrstop

extern IconItem iconItem[];
void InitIconLibMenuIcons();

INT_PTR CloseAction(WPARAM, LPARAM)
{
	cfg::shutDown = 1;
	Miranda_Close();
	PostQuitMessage(0);
	return 0;
}

int InitCustomMenus(void)
{
	InitIconLibMenuIcons();

	CreateServiceFunction("CloseAction", CloseAction);
	return 0;
}
