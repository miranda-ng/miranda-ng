/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2004-2007  Piotr Piastucki

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

#ifndef _TLEN_MUC_H_
#define _TLEN_MUC_H_

#include <windows.h>

#define USER_FLAGS_OWNER			0x01
#define USER_FLAGS_ADMIN			0x02
#define USER_FLAGS_REGISTERED		0x04
#define USER_FLAGS_GLOBALOWNER		0x08
#define USER_FLAGS_KICKED			0x80

extern int TlenMUCRecvInvitation(TlenProtocol *proto, const char *roomJid, const char *roomName, const char *from, const char *reason);

#endif
