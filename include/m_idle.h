/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2005 Miranda ICQ/IM project, 
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

/* The idle module checks how long the user has been idle, it can be at the Miranda level
or the OS level - the user has optional two settings, one for "short" idle and another
for "long" idle. Thie module will generate long/short events based on these user preferences
and other information too. If you are unsure which idle mode to report for, report it 
for short idle.*/

#define IDF_ISIDLE		0x1 // idle has become active (if not set, inactive)
#define IDF_PRIVACY		0x8 // if set, the information provided shouldn't be given to third parties.

/*
	wParam: 0
	lParam: IDF_* (or'd field)
	Affect: This event is fired when information about idle changes.
	Note: You will get multiple calls with IDF_ISIDLE set, the first is for short idle
		then long idle, then anytime during you might get IDF_ONFORCE if the screensaver or station
		become locked.

		It is up to you to keep state, i.e. once the idle that you care about is reported
		ignore other status notifications with IDF_ISIDLE set until you get one with IDF_ISIDLE isn't
		set.
	Version: 0.3.4a+ (2004/09/16)
*/
#define ME_IDLE_CHANGED "Miranda/Idle/Changed"

#define MIRANDA_IDLE_INFO_SIZE_1 20

typedef struct {
	int cbSize;			    // sizeof()
	int idleTime;	       // idle in mins, if zero then disabled
	int privacy;		    // user doesnt want other people seeing anything more than they are idle
	int aaStatus;			 // status to go to when user is auto away
	int aaLock;				 // the status shouldn't be unset if its set
	int idleType;
}
	MIRANDA_IDLE_INFO;

/*
	wParam; 0
	lParam: &MIRANDA_IDLE_INFO
	Affect: Return information about current idle settings, like short/long idle time in mins 
		and if the user wants that info kept private, etc
	Returns: zero on success, non zero on failure
	Version: 0.3.4 (2004/09/16)
*/

#define MS_IDLE_GETIDLEINFO "Miranda/Idle/GetInfo"

#endif // M_IDLE_H__

