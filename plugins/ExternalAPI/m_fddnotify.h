/*

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

#ifndef _FDDNOTIFY_
#define _FDDNOTIFY_

//Enables all notifications (for use by BossKey)
//wParam=0
//lParam=0
//returns 0
#define MS_FDDNOTIFY_ENABLE         "FddNotify/NotifyEnable"


//Disables all notifications (for use by BossKey)
//wParam=0
//lParam=0
//returns 0
#define MS_FDDNOTIFY_DISABLE        "FddNotify/NotifyDisable"


//Makes the flashing begin
//wParam=(unsigned int)eventCount
//lParam=0
//returns 0
#define MS_FDDNOTIFY_STARTBLINK     "FddNotify/StartBlinking"


//Receives the number of events that were opened (usuful for the 'until events opened' setting)
//wParam=(unsigned int)eventCount
//lParam=0
//returns 0
#define MS_FDDNOTIFY_EVENTSOPENED   "FddNotify/EventsWereOpened"


//Informs if the flashing is active
//wParam=0
//lParam=0
//returns 0 if the flashing is inactive or 1 if it is active 
#define MS_FDDNOTIFY_FLASHINGACTIVE "FddNotify/IsFlashingActive"


#endif