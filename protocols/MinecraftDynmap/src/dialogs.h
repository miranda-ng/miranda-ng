/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2015-16 Robert P�sel

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

#pragma once

void InitIcons(void);
HANDLE GetIconHandle(const char *name);

INT_PTR CALLBACK MinecraftDynmapAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK MinecraftDynmapOptionsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
