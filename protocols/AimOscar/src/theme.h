/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2009 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef THEME_H
#define THEME_H

void InitIcons(void);
void InitExtraIcons(void);

HICON  LoadIconEx(const char* name, bool big = false);
HANDLE GetIconHandle(const char* name);
void   ReleaseIconEx(const char* name, bool big = false);
void   WindowSetIcon(HWND hWnd, const char* name);
void   WindowFreeIcon(HWND hWnd);

void   add_AT_icons(CAimProto* ppro);
void   remove_AT_icons(CAimProto* ppro);
void   add_ES_icons(CAimProto* ppro);
void   remove_ES_icons(CAimProto* ppro);

void set_contact_icon(CAimProto* ppro, MCONTACT hContact);

#endif
