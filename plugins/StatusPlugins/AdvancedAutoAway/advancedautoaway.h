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

#include <m_options.h>
#include <m_idle.h>
#include "version.h"

#define SETTING_IGNLOCK					"IgnoreLockKeys"
#define SETTING_IGNSYSKEYS				"IgnoreSysKeys"
#define SETTING_IGNALTCOMBO				"IgnoreAltCombo"
#define SETTING_SAMESETTINGS			"SameAutoAwaySettings"
#define SETTING_ALL						"ALLPROTOS"
#define SETTING_MSGCUSTOM				"Custom"
#define SETTING_STATUSMSG				"Msg"
#define SETTING_CONFIRMDELAY			"ConfirmTimeout"
#define MODULENAME						"AdvancedAutoAway"

#define SETTING_AWAYTIME_DEFAULT	5
#define SETTING_NATIME_DEFAULT		20
#define SETTING_AWAYCHECKTIMEINSECS		"CheckInterval"

#define STATUS_RESET				1
#define STATUS_AUTOAWAY				2
#define STATUS_AUTONA				3

#define SETTING_MONITORMOUSE		"MonitorMouse"
#define SETTING_MONITORKEYBOARD		"MonitorKeyboard"

#define FLAG_ONSAVER				1 // db: set lv1 status on screensaver ?
#define FLAG_ONMOUSE				2 // db: set after inactivity ?
#define FLAG_SETNA					4 // db: set NA after xx of away time ?
#define FLAG_CONFIRM				8 // db: show confirm dialog ?
#define FLAG_RESET					16 // db: restore status ?
#define FLAG_LV2ONINACTIVE			32 // db: set lv2 only on inactivity
#define FLAG_MONITORMIRANDA			64 // db: monitor miranda activity only
#define FLAG_ONLOCK					128 // db: on work station lock

struct TAAAProtoSetting : public PROTOCOLSETTINGEX 
{
	TAAAProtoSetting( PROTOACCOUNT* pa );
	~TAAAProtoSetting();

	__inline void* operator new( size_t size ) {	return calloc( 1, size ); }
	__inline void operator delete( void* p ) { free( p ); }

	int originalStatusMode;
	STATES 
		oldState, 
		curState;
	BOOL statusChanged; // AAA changed the status, don't update mStatus
	BOOL mStatus; // status changed manually or not ?
	int optionFlags, // db: see above
		 awayTime, // db: time to wait for inactivity
		 naTime, // db: time to wait after away is set
		 statusFlags; // db: set lv1 status if this is original status
	WORD lv1Status, // db
		  lv2Status; // db
	unsigned int sts1setTimer;
};

struct AAMSGSETTING
{
	short useCustom;
	int status;
	char* msg;
};

extern HINSTANCE hInst;

int CompareSettings( const TAAAProtoSetting* p1, const TAAAProtoSetting* p2 );

extern OBJLIST<TAAAProtoSetting> autoAwaySettings;

int  LoadAutoAwaySetting(TAAAProtoSetting& autoAwaySetting, char* protoName);
void LoadOptions(OBJLIST<TAAAProtoSetting>& settings, BOOL override);

#endif
