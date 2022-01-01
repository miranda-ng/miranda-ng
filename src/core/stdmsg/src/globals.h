/*

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

struct GlobalMessageData
{
	GlobalMessageData();

	CMOption<bool> bShowButtons;
	CMOption<bool> bSendButton;
	CMOption<bool> bShowTyping;
	CMOption<bool> bShowTypingWin;
	CMOption<bool> bShowTypingTray;
	CMOption<bool> bShowTypingClist;
	CMOption<bool> bShowIcons;
	CMOption<bool> bShowTime;
	CMOption<bool> bShowDate;
	CMOption<bool> bShowAvatar;
	CMOption<bool> bShowNames;
	CMOption<bool> bShowSecs;
	CMOption<bool> bShowReadChar;
	CMOption<bool> bSendOnEnter;
	CMOption<bool> bSendOnDblEnter;
	CMOption<bool> bSendOnCtrlEnter;
	CMOption<bool> bAutoClose;
	CMOption<bool> bAutoMin;
	CMOption<bool> bTypingNew;
	CMOption<bool> bTypingUnknown;
	CMOption<bool> bCtrlSupport;
	CMOption<bool> bShowFormat;
	CMOption<bool> bSavePerContact;
	CMOption<bool> bDoNotStealFocus;
	CMOption<bool> bCascade;
	CMOption<bool> bDeleteTempCont;
	CMOption<bool> bUseStatusWinIcon;

	CMOption<bool> bLimitAvatarHeight;
	CMOption<uint16_t> iAvatarHeight;

	CMOption<uint32_t> popupFlags;
	CMOption<uint32_t> msgTimeout;
	CMOption<uint32_t> nFlashMax;
	CMOption<uint8_t>  iGap;

	CMOption<uint8_t>  iLoadHistory;
	CMOption<uint16_t>  nLoadCount, nLoadTime;

	bool bSmileyInstalled = false;
};

void InitGlobals();

extern GlobalMessageData g_dat;

#endif
