/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "commonheaders.h"

/***********************************************************************************************************
 * services
 ***********************************************************************************************************/

/**
* This service function provides a TIME_ZONE_INFORMATION structure
* for the desired contact, in case the contact's timezone can be determined.
* parsed to new core tzi interface if present.
* 
* @param	wParam			- HANDLE of the contact, to retrieve timezone information from.
* @param	lParam			- pointer to a TIME_ZONE_INFORMATION to fill.
*
* @retval	0 - success
* @retval	1 - failure
**/

INT_PTR GetContactTimeZoneInformation(WPARAM wParam, LPARAM lParam)
{
	if (lParam == NULL)
		return (1);
	//use new core tz interface
	LPTIME_ZONE_INFORMATION pTimeZoneInformation = tmi.getTzi(tmi.createByContact(wParam, 0, 0));
	if (pTimeZoneInformation == NULL)
		return (1);
	memcpy((void *)lParam, pTimeZoneInformation, sizeof(TIME_ZONE_INFORMATION));
	return (0);
}

/**
* This function returns the contact's local time.
*
* @param	wParam			- HANDLE of the contact, to retrieve timezone information from.
* @param	lParam			- pointer to a systemtime structure
*	
* @return	TRUE or FALSE
**/

INT_PTR GetContactLocalTime(WPARAM wParam, LPARAM lParam)
{
	//use new core tz interface
	LPSYSTEMTIME pSystemTime = (LPSYSTEMTIME)lParam;
	return (INT_PTR)tmi.getTimeZoneTimeByContact(wParam, pSystemTime);
}

/***********************************************************************************************************
 * initialization
 ***********************************************************************************************************/

/**
* This function initially loads the module upon startup.
**/

void SvcTimezoneLoadModule()
{
	CreateServiceFunction(MS_USERINFO_TIMEZONEINFO, GetContactTimeZoneInformation);
	CreateServiceFunction(MS_USERINFO_LOCALTIME, GetContactLocalTime);
}
