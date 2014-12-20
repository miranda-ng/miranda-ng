/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-13 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "common.h"

extern OBJLIST<OmegleProto> g_Instances;

static IconItem icons[] =
{
	{ "omegle", LPGEN("Omegle Icon"), IDI_OMEGLE },
};

static HANDLE hIconLibItem[SIZEOF(icons)];

void InitIcons(void)
{
	Icon_Register(g_hInstance, "Protocols/Omegle", icons, SIZEOF(icons), "Omegle");
}

HANDLE GetIconHandle(const char* name)
{
	for(size_t i=0; i<SIZEOF(icons); i++)
	{
		if(strcmp(icons[i].szName, name) == 0)
			return hIconLibItem[i];
	}
	return 0;
}
