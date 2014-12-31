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

#ifndef M_EMAIL_H__
#define M_EMAIL_H__ 1

//send an e-mail to the specified contact     v0.1.0.1+
//wParam = (MCONTACT)hContact
//lParam = 0
//returns 0 on success or nonzero on failure
//if an error occurs the service will display a message box with the error
//text, so this service should not be used if you do not want this behaviour.
#define MS_EMAIL_SENDEMAIL  "SREMail/SendCommand"

#endif // M_EMAIL_H__
