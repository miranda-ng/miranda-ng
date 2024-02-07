/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-24 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// implements the event notification module for tabSRMM. The code
// is largely based on the NewEventNotify plugin for Miranda NG. See
// notices below.
//
//  Name: NewEventNotify - Plugin for Miranda ICQ
// 	Description: Notifies you when you receive a message
// 	Author: icebreaker, <icebreaker@newmail.net>
// 	Date: 18.07.02 13:59 / Update: 16.09.02 17:45
// 	Copyright: (C) 2002 Starzinger Michael

#include "stdafx.h"

CMOption<int> NEN::ActionLeft(NEN_MODULE, "ActionLeft", MASK_OPEN | MASK_DISMISS);
CMOption<int> NEN::ActionRight(NEN_MODULE, "ActionRight", MASK_DISMISS); 

CMOption<bool> NEN::bMucPopups(NEN_MODULE, "MucPopups", true);
CMOption<bool> NEN::bWindowCheck(NEN_MODULE, "WindowCheck", false);
CMOption<bool> NEN::bNoSounds(NEN_MODULE, "NoSounds", false);
CMOption<bool> NEN::bNoAutoPopup(NEN_MODULE, "NoAutoPopup", false);

CMOption<int>   NEN::iDelayMsg(NEN_MODULE, "DelayMessage", 0);
CMOption<DWORD> NEN::colTextMsg(NEN_MODULE, "ColorTextMsg", 0);
CMOption<DWORD> NEN::colBackMsg(NEN_MODULE, "ColorBackMsg", RGB(255, 255, 128));


CMOption<bool>  NEN::bColDefaultErr(NEN_MODULE, "DefaultColorErr", true);
CMOption<int>   NEN::iDelayErr(NEN_MODULE, "DelayErr", 0);
CMOption<DWORD> NEN::colTextErr(NEN_MODULE, "ColorTextErr", 0);
CMOption<DWORD> NEN::colBackErr(NEN_MODULE, "ColorBackErr", RGB(255, 255, 128));
