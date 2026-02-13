/*
    AdvancedAutoAway Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __ADVANCED_AUTOAWAY_HEADER
#define __ADVANCED_AUTOAWAY_HEADER

#define AAAMODULENAME        "AdvancedAutoAway"
#define SETTING_IGNLOCK      "IgnoreLockKeys"
#define SETTING_IGNSYSKEYS   "IgnoreSysKeys"
#define SETTING_IGNALTCOMBO  "IgnoreAltCombo"
#define SETTING_SAMESETTINGS "SameAutoAwaySettings"
#define SETTING_ALL          "ALLPROTOS"
#define SETTING_MSGCUSTOM    "Custom"
#define SETTING_STATUSMSG    "Msg"
#define SETTING_CONFIRMDELAY "ConfirmTimeout"
#define SETTING_ENTERIDLE    "EnterIdle"

#define SETTING_AWAYTIME_DEFAULT 5
#define SETTING_NATIME_DEFAULT  20
#define SETTING_AWAYCHECKTIMEINSECS "CheckInterval"

#define SETTING_MONITORMOUSE    "MonitorMouse"
#define SETTING_MONITORKEYBOARD "MonitorKeyboard"

void AdvancedAutoAwayLoad();
void AdvancedAutoAwayUnload();

int  LoadAutoAwaySetting(SMProto&, char *protoName);
void AAALoadOptions();

extern bool g_bAAASettingSame;

#endif
