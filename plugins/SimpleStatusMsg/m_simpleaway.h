/*

Simple Status Message plugin for Miranda IM
Copyright (C) 2006-2010 Bartosz 'Dezeath' Bia³ek, (C) 2005 Harven

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef M_SIMPLEAWAY_H__
#define M_SIMPLEAWAY_H__ 1

//
// NOTE: These services are deprecated. Please do not use them anymore.
// Take a look at m_simplestatusmsg.h instead.
//

// Represents status that a protocol(s) is/are currently in
#define	ID_STATUS_CURRENT	40082

// Force a change of global status mode/message
// wParam = (int)new status (from statusmodes.h), 0 or ID_STATUS_CURRENT for current
// lParam = (TCHAR *)status message
#define MS_SA_SETSTATUSMODE "SimpleAway/SetStatusMode"
#define MS_AWAYSYS_SETSTATUSMODE MS_SA_SETSTATUSMODE // for compatibility with some plugins

// Brings up the status message dialog
// wParam = 0
// lParam = (LPARAM)(char *)protocol name, NULL if for all protocols
#define MS_SA_SHOWSTATUSMSGDIALOG "SimpleAway/ShowStatusMessageDialog"

// Similar to the service above, for internal use only
#define MS_SA_TTCHANGESTATUSMSG "SimpleAway/TTChangeStatusMessage"

// Force a change of status mode/message. The status message dialog will appear,
// depending on the configuration of the user
// wParam = (int)new status
// lParam = (LPARAM)(char *)protocol name, NULL if for all protocols
// Returns 1 when changed without showing the status message dialog
#define MS_SA_CHANGESTATUSMSG "SimpleAway/ChangeStatusMessage"

// For checking if SimpleAway is running
// wParam = lParam = 0
// Always returns 1
#define MS_SA_ISSARUNNING  "SimpleAway/IsSARunning"

// Copy the away/na/etc message of a contact
// wParam = (WPARAM)(HANDLE)hContact
// lParam = 0
// Returns 0 on success or nonzero on failure
// Returns immediately, without waiting for the message to retrieve
#define MS_SA_COPYAWAYMSG  "SimpleAway/CopyAwayMsg"

// Go to URL in away/na/etc message of a contact
// wParam = (WPARAM)(HANDLE)hContact
// lParam = 0
#define MS_SA_GOTOURLMSG  "SimpleAway/GoToURLMsg"

// Returns the default status message for a status in specified protocol module
// or the current status message for the specified protocol if 0 or ID_STATUS_CURRENT is used
// wParam = (int)status, 0 or ID_STATUS_CURRENT for current
// lParam = (LPARAM)(char *)protocol name, NULL if for all protocols
// Returns status msg. Remember to free the return value
#ifndef MS_AWAYMSG_GETSTATUSMSG
  #define MS_AWAYMSG_GETSTATUSMSG  "SRAway/GetStatusMessage"
#endif
#ifndef MS_AWAYMSG_GETSTATUSMSGW
  #define MS_AWAYMSG_GETSTATUSMSGW  "SRAway/GetStatusMessageW"
#endif

#ifndef MS_AWAYMSG_GETSTATUSMSGT
  #ifdef _UNICODE
    #define MS_AWAYMSG_GETSTATUSMSGT MS_AWAYMSG_GETSTATUSMSGW
  #else
    #define MS_AWAYMSG_GETSTATUSMSGT MS_AWAYMSG_GETSTATUSMSG
  #endif
#endif

// Force a change to specified global status mode/message
// (calls MS_SA_CHANGESTATUSMSG with proper parameters)
// wParam = lParam = 0
#define	MS_SA_SETOFFLINESTATUS		"SimpleAway/SetOfflineStatus"
#define	MS_SA_SETONLINESTATUS		"SimpleAway/SetOnlineStatus"
#define	MS_SA_SETAWAYSTATUS			"SimpleAway/SetAwayStatus"
#define	MS_SA_SETDNDSTATUS			"SimpleAway/SetDNDStatus"
#define	MS_SA_SETNASTATUS			"SimpleAway/SetNAStatus"
#define	MS_SA_SETOCCUPIEDSTATUS		"SimpleAway/SetOccupiedStatus"
#define	MS_SA_SETFREECHATSTATUS		"SimpleAway/SetFreeChatStatus"
#define	MS_SA_SETINVISIBLESTATUS	"SimpleAway/SetInvisibleStatus"
#define	MS_SA_SETONTHEPHONESTATUS	"SimpleAway/SetOnThePhoneStatus"
#define	MS_SA_SETOUTTOLUNCHSTATUS	"SimpleAway/SetOutToLunchStatus"

#endif // M_SIMPLEAWAY_H__
