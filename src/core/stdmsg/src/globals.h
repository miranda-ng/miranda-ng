/*

Copyright 2000-12 Miranda IM, 2012-15 Miranda NG project,
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

#ifndef SRMM_GLOBALS_H
#define SRMM_GLOBALS_H

#define SMF_SHOWINFO        0x00000001
#define SMF_SHOWBTNS        0x00000002
#define SMF_SENDBTN         0x00000004
#define SMF_SHOWTYPING      0x00000008
#define SMF_SHOWTYPINGWIN   0x00000010
#define SMF_SHOWTYPINGTRAY  0x00000020
#define SMF_SHOWTYPINGCLIST 0x00000040
#define SMF_SHOWICONS       0x00000080
#define SMF_SHOWTIME        0x00000100
#define SMF_AVATAR          0x00000200
#define SMF_SHOWDATE        0x00000400
#define SMF_HIDENAMES       0x00000800
#define SMF_SHOWSECS        0x00001000
#define SMF_SHOWREADCHAR    0x00002000
#define SMF_SENDONENTER     0x00004000
#define SMF_SENDONDBLENTER  0x00008000
#define SMF_AUTOCLOSE       0x00010000
#define SMF_AUTOMIN         0x00020000
#define SMF_TYPINGUNKNOWN   0x00040000
#define SMF_CTRLSUPPORT     0x00080000
#define SMF_SHOWFORMAT      0x00100000

#define SMF_ICON_TYPING     0

struct GlobalMessageData
{
	unsigned int flags;
	HANDLE hMessageWindowList;
	DWORD openFlags;
	DWORD msgTimeout;
	DWORD nFlashMax;
};

void InitGlobals();
void ReloadGlobals();

extern GlobalMessageData g_dat;

#endif
