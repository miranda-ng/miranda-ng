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

#ifndef _FORTUNEMSG_
#define _FORTUNEMSG_

#define MIN_FORTUNEMSG	  10
#define MAX_FORTUNEMSG	1024
#define FORTUNE_BUFSIZE	(MAX_FORTUNEMSG + 1)

//returns the fortune message (from a random file)
//wParam=0
//lParam=(char *)buffer or 0
//returns (char *)status msg (buffer if specified), or 0 on failure
//If lParam != 0 then the fortune msg is to be stored there, make sure
//its length is at least equal to FORTUNE_BUFSIZE. If lParam == 0, then 
//the plugin will allocate the memory, but don't forget to free it (the
//return value) using MS_FORTUNEMSG_FREEMEMORY (but only if you specify lParam=0!!!)
#define MS_FORTUNEMSG_GETMESSAGE   "FortuneMsg/GetMessage"

//returns the fortune message for a protocol
//wParam=(char*)szProtoName
//lParam=(char *)buffer or 0
//returns (char *)status msg (buffer if specified), or 0 on failure
//If lParam != 0 then the fortune msg is to be stored there, make sure
//its length is at least equal to FORTUNE_BUFSIZE. If lParam == 0, then 
//the plugin will allocate the memory, but don't forget to free it (the
//return value) using MS_FORTUNEMSG_FREEMEMORY (but only if you specify lParam=0!!!)
#define MS_FORTUNEMSG_GETPROTOMSG  "FortuneMsg/GetProtoMessage"

//returns the fortune status message for a status
//wParam=(int)status
//lParam=(char *)buffer or 0
//returns (char *)status msg (buffer if specified), or 0 on failure
//If lParam != 0 then the fortune msg is to be stored there, make sure
//its length is at least equal to FORTUNE_BUFSIZE. If lParam == 0, then 
//the plugin will allocate the memory, but don't forget to free it (the
//return value) using MS_FORTUNEMSG_FREEMEMORY (but only if you specify lParam=0!!!)
#define MS_FORTUNEMSG_GETSTATUSMSG  "FortuneMsg/GetStatusMessage"

//frees the memory allocated by one of the other three services
//wParam=0
//lParam=(void *)pointer to the memory to be freed
//(the returned value from one of the other three services if called with lParam=0)
//return value: 0 on success, -1 on failure (argument was NULL)
#define MS_FORTUNEMSG_FREEMEMORY	"FortuneMsg/FreeMemory"


//this service was created for being used by Variables plugin
//wParam=0
//lParam=(ARGUMENTSINFO *) see m_variables.h for description of the structure
//returns (char *)status msg, or 0 on failure
#define MS_FORTUNEMSG_FROMVARIABLES	"FortuneMsg/FromVariables"


#endif
