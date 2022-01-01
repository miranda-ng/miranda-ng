/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-22 Miranda NG team

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

#include "stdafx.h"

static IconItem iconList[] =
{
	{ LPGEN("Omegle Icon"), "omegle", IDI_OMEGLE },
	{ LPGEN("Stranger is typing"), "typing_on", IDI_TYPING_ON },
	{ LPGEN("Stranger stopped typing"), "typing_off", IDI_TYPING_OFF },
};

void InitIcons(void)
{
	g_plugin.registerIcon("Protocols/Omegle", iconList, "Omegle");
}
