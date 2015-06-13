/*
Custom profile folders plugin for Miranda IM

Copyright � 2005 Cristian Libotean

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

#ifndef M_FOLDERS_PROVIDED_SERVICES_H
#define M_FOLDERS_PROVIDED_SERVICES_H

#define MAX_FOLDERS_PATH 512

extern TCHAR szCurrentProfilePath[MAX_FOLDERS_PATH];
extern TCHAR szCurrentProfile[MAX_FOLDERS_PATH];
extern TCHAR szMirandaPath[MAX_FOLDERS_PATH];
extern TCHAR szUserDataPath[MAX_FOLDERS_PATH];

int InitServices();
void InitOptions();

#endif //M_FOLDERS_PROVIDED_SERVICES_H