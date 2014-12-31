/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (ñ) 2000-04 Miranda ICQ/IM project,
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

(c) 2009 by Merlin, ing.u.horn@googlemail.com

*/

#ifndef _M_MDYNDNS_H
#define _M_MDYNDNS_H

#define MDYNDNS_MODULE			"mDynDNS"	// db settings module path
#define MDYNDNS_KEY_DOMAIN		"Domain"	// db settings Domain Key
#define MDYNDNS_KEY_LASTIP		"LastIP"	// db settings IP Key

// SERVICE--------------------------------------------------------

#define MDYNDNS_IPMODE_AUTO		0			// get IP auto mode (best way)
#define MDYNDNS_IPMODE_IPCHECK	1			// get IP by mDynDNS checkip - safe - 1 check every 10min allowed
#define MDYNDNS_IPMODE_DNS		2			// get IP by DNS querry - unsafe (IP CAN be wrong in DNS cache)

// obtain the IP
// wParam = (IPMODE)			- above MDYNDNS_IPMODE flag
// lParam = (char*)				- pointer (min 16 bit buffer) that recive the IP (NULL on failure)
// returns: other than 0 on failure,

#define ME_MDYNDNS_GETIP		"mDynDNS/GetIP"

// EVENT ---------------------------------------------------------

// fired when the IP is changed
// wParam = 0
// lParam = char* new IP
// the event CAN pass a NULL pointer in lParam which means that the IP check fail

#define ME_MDYNDNS_IPCHANGED	"mDynDNS/IPchanged"

#endif
