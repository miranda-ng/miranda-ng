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

#ifndef _SVC_TIMEZONE_H_
#define _SVC_TIMEZONE_H_

/**
 * This structure is used by GetTimeZoneInformationByIndex to retrieve
 * timezone information from windows' registry
 **/
typedef struct _REG_TZI_FORMAT
{
	LONG Bias;
	LONG StandardBias;
	LONG DaylightBias;
	SYSTEMTIME StandardDate;
	SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT, *PREG_TZI_FORMAT;

INT_PTR		GetContactTimeZoneInformation(WPARAM wParam, LPARAM lParam);
INT_PTR		GetContactLocalTime(WPARAM wParam, LPARAM lParam);

void		SvcTimezoneLoadModule();

#endif /* _SVC_TIMEZONE_H_ */