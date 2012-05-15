/*

SimpleAway plugin for Miranda-IM

Copyright © 2005 Harven, © 2006-2008 Dezeath

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

#ifndef M_SIMPLEAWAY_H__
#define M_SIMPLEAWAY_H__ 1

// Represents status that a protocol(s) is/are currently in
#define	ID_STATUS_CURRENT	40082

// Force a change of global status mode/message
// wParam = (int)new status, from statusmodes.h or ID_STATUS_CURRENT
// lParam = (char *)status message
#define MS_SA_SETSTATUSMODE "SimpleAway/SetStatusMode"
#define MS_AWAYSYS_SETSTATUSMODE MS_SA_SETSTATUSMODE // for compatibility with some plugins

// Brings up the status message dialog
// wParam = 0
// lParam = (char *)protocol name, NULL if for all protocols
#define MS_SA_SHOWSTATUSMSGDIALOG "SimpleAway/ShowStatusMessageDialog"

// Similar to the service above, for internal use only
#define MS_SA_TTCHANGESTATUSMSG "SimpleAway/TTChangeStatusMessage"

// Force a change of status mode/message. The status message dialog will appear,
// depending on the configuration of the user
// wParam = (int)new status, from statusmodes.h
// lParam = (char *)protocol name, NULL if for all protocols
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

// Returns the default status message for a status in specified protocol module
// or the current status message for the specified protocol if ID_STATUS_CURRENT is used
// wParam = (int)status, from statusmodes.h or ID_STATUS_CURRENT
// lParam = (char *)protocol name, NULL if for all protocols
// Returns status msg. Remember to free the return value
#ifndef MS_AWAYMSG_GETSTATUSMSG
#define MS_AWAYMSG_GETSTATUSMSG  "SRAway/GetStatusMessage"
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
