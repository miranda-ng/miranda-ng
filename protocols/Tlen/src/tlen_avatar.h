/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
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

#ifndef _TLEN_AVATAR_H_
#define _TLEN_AVATAR_H_

void TlenProcessPresenceAvatar(TlenProtocol *proto, XmlNode *node, TLEN_LIST_ITEM *item);
int TlenProcessAvatarNode(TlenProtocol *proto, XmlNode *avatarNode, TLEN_LIST_ITEM *item);
void TlenGetAvatarFileName(TlenProtocol *proto, TLEN_LIST_ITEM *item, TCHAR* psztDest, int cbLen);
void TlenGetAvatar(TlenProtocol *proto, MCONTACT hContact);
void TlenUploadAvatar(TlenProtocol *proto, unsigned char *data, int dataLen, int access);
void TlenRemoveAvatar(TlenProtocol *proto);

#endif // _TLEN_AVATAR_H_
