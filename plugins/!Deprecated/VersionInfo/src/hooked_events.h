/*
Version information plugin for Miranda IM

Copyright © 2002-2006 Luca Santarelli, Cristian Libotean

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

#ifndef M_VERSIONINFO_HOOKED_EVENTS_H
#define M_VERSIONINFO_HOOKED_EVENTS_H

extern HANDLE hModulesLoaded;
extern HANDLE hOptionsInitialise;

int HookEvents();

int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
int OnOptionsInitialise(WPARAM wParam, LPARAM lParam);

#endif