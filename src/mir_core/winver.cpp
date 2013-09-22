/*
Copyright (C) 2012-13 Miranda NG team (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "commonheaders.h"

static int dwWinVer;

void InitWinver()
{
	DWORD dwVer = LOWORD( GetVersion());
	dwWinVer = MAKEWORD(HIBYTE(dwVer), LOBYTE(dwVer));
}

MIR_CORE_DLL(BOOL) IsWinVerVistaPlus()
{
	return dwWinVer >= _WIN32_WINNT_VISTA;
}

MIR_CORE_DLL(BOOL) IsWinVer7Plus()
{
	return dwWinVer >= _WIN32_WINNT_WIN7;
}
