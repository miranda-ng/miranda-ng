/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef M_AWAYMSG_H__
#define M_AWAYMSG_H__ 1

//show the away/na/etc message for a contact  v0.1.0.1+
//wParam = (MCONTACT)hContact
//lParam = 0
//returns 0 on success or nonzero on failure
//returns immediately, without waiting for the message to retrieve
#define MS_AWAYMSG_SHOWAWAYMSG   "SRAway/GetMessage"

//returns (TCHAR*)the default status message for a status
//wParam = (int)status
//lParam = (LPARAM)(char*)szProto
//returns status msg.  Remember to free the return value using mir_free/ptrW
#define MS_AWAYMSG_GETSTATUSMSGW  "SRAway/GetStatusMessageW"

#ifdef _UNICODE
#define MS_AWAYMSG_GETSTATUSMSGT MS_AWAYMSG_GETSTATUSMSGW
#else
#define MS_AWAYMSG_GETSTATUSMSGT not implemented
#endif

#endif // M_AWAYMSG_H__
