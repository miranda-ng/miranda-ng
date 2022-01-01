/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-05 Miranda ICQ/IM project,
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

#ifndef M_IDLE_H__
#define M_IDLE_H__ 1

// The idle module checks how long the user has been idle, it can be at the Miranda level
// or the OS level - the user has optional two settings, one for "short" idle and another
// for "long" idle. Thie module will generate long/short events based on these user preferences
// and other information too. If you are unsure which idle mode to report for, report it
// for short idle.

/////////////////////////////////////////////////////////////////////////////////////////
// Idle_Enter() - enters (if type >= 0) or leaves (if type == -1) the idle mode
// Each plugin that implements MIID_AUTOAWAY can set its own idle types

EXTERN_C MIR_APP_DLL(void) Idle_Enter(int type = -1);

/////////////////////////////////////////////////////////////////////////////////////////
// ME_IDLE_CHANGED - called when the idle mode is changed
// wParam: 0
// lParam: IDF_* (or'd field)
// 
// Affect: This event is fired when information about idle changes.
// Note: You will get multiple calls with IDF_ISIDLE set, the first is for short idle
// then long idle, then anytime during you might get IDF_ONFORCE if the screensaver or station
// become locked.

// It is up to you to keep state, i.e. once the idle that you care about is reported
// ignore other status notifications with IDF_ISIDLE set until you get one with IDF_ISIDLE isn't
// set.

#define IDF_ISIDLE		0x1 // idle has become active (if not set, inactive)
#define IDF_PRIVACY		0x8 // if set, the information provided shouldn't be given to third parties.

#define ME_IDLE_CHANGED "Miranda/Idle/Changed"

/////////////////////////////////////////////////////////////////////////////////////////
// Returns information about current idle settings, like short/long idle time in mins
// and if the user wants that info kept private, etc

struct MIRANDA_IDLE_INFO
{
	int idleTime;	       // idle in mins, if zero then disabled
	int privacy;		    // user doesnt want other people seeing anything more than they are idle
	int aaStatus;			 // status to go to when user is auto away
	int aaLock;				 // the status shouldn't be unset if its set
	int idleType;
	int idlesoundsoff;
};

EXTERN_C MIR_APP_DLL(void) Idle_GetInfo(MIRANDA_IDLE_INFO &pInfo);

#endif // M_IDLE_H__
