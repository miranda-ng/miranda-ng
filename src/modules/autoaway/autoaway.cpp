/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
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
#include "commonheaders.h"

#define AA_MODULE "AutoAway"

void Proto_SetStatus(const char* szProto, unsigned status);

static int AutoAwayEvent(WPARAM, LPARAM lParam)
{
	int i;

	MIRANDA_IDLE_INFO mii;
	mii.cbSize = sizeof( mii );
	CallService( MS_IDLE_GETIDLEINFO, 0, (LPARAM)&mii );
	if ( mii.aaStatus == 0 )
		return 0;

	for ( i=0; i < accounts.getCount(); i++ ) {
		PROTOACCOUNT* pa = accounts[i];

		if (!Proto_IsAccountEnabled( pa ) || Proto_IsAccountLocked( pa )) continue;

		int statusbits = CallProtoService( pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0 );
		int currentstatus = CallProtoService( pa->szModuleName, PS_GETSTATUS, 0, 0 );
		int status = mii.aaStatus;
		if ( !(statusbits & Proto_Status2Flag(status)) ) {
			// the protocol doesnt support the given status
			if ( statusbits & Proto_Status2Flag( ID_STATUS_AWAY ))
				status = ID_STATUS_AWAY;
			// the proto doesnt support user mode or even away, bail.
			else
				continue;
		}
		if ( currentstatus >= ID_STATUS_ONLINE && currentstatus != ID_STATUS_INVISIBLE ) {			
			if ( (lParam&IDF_ISIDLE) && ( currentstatus == ID_STATUS_ONLINE || currentstatus == ID_STATUS_FREECHAT ))  {
				DBWriteContactSettingByte( NULL, AA_MODULE, pa->szModuleName, 1 );
				Proto_SetStatus( pa->szModuleName, status );
			}
			else if ( !(lParam & IDF_ISIDLE) && DBGetContactSettingByte( NULL, AA_MODULE, pa->szModuleName, 0 )) {
				// returning from idle and this proto was set away, set it back
				DBWriteContactSettingByte( NULL, AA_MODULE, pa->szModuleName, 0 );
				if ( !mii.aaLock )
					Proto_SetStatus( pa->szModuleName, ID_STATUS_ONLINE);
	}	}	}

	return 0;
}

int LoadAutoAwayModule(void)
{
	HookEvent(ME_IDLE_CHANGED, AutoAwayEvent);
	return 0;
}
