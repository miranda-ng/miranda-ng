/*

Copyright 2000-12 Miranda IM, 2012-16 Miranda NG project,
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

struct SrmmFlags
{
	bool bShowInfo : 1;
	bool bShowButtons : 1;
	bool bSendButton : 1;
	bool bShowTyping : 1;
	bool bShowTypingWin : 1;
	bool bShowTypingTray : 1;
	bool bShowTypingClist : 1;
	bool bShowIcons : 1;
	bool bShowTime : 1;
	bool bShowDate : 1;
	bool bShowAvatar : 1;
	bool bHideNames : 1;
	bool bShowSecs : 1;
	bool bShowReadChar : 1;
	bool bSendOnEnter : 1;
	bool bSendOnDblEnter : 1;
	bool bAutoClose : 1;
	bool bAutoMin : 1;
	bool bTypingUnknown : 1;
	bool bCtrlSupport : 1;
	bool bShowFormat : 1;
};

struct GlobalMessageData
{
	SrmmFlags flags;
	DWORD openFlags;
	DWORD msgTimeout;
	DWORD nFlashMax;
};

void InitGlobals();
void ReloadGlobals();

extern GlobalMessageData g_dat;

#endif
